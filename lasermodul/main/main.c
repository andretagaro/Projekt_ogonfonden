#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "vl53l5cx_api.h"
#include "platform.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "global_defines.h"
#include "driver/gpio.h"
#include "vl53l5cx_custom_functions.h"
#include "esp_now_functions.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "driver/adc.h"

void init_gpio(void);
void i2c_init_master(const uint8_t SDA_LINE, const uint8_t SCL_LINE, const uint32_t FREQ, const uint8_t PORT);
void config_adc(void);
void adjust_height(uint8_t* height_adjustment);
void adjust_for_height(uint8_t* height_adjustment, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left);
bool is_battery_under_in_mv(uint16_t limit);
void battery_handler(void* args);

uint8_t mac_adress_right[MAC_SIZE] = {0x8c, 0x4b, 0x14, 0x0e, 0xf4, 0x9c};
uint8_t mac_adress_left[MAC_SIZE] = {0x40, 0x91, 0x51, 0x2d, 0x0f, 0xa4};
uint8_t mac_adress_sender[MAC_SIZE] = {0xfc, 0xf5, 0xc4, 0x09, 0x61, 0x90};
uint8_t low_device_battery_counter = 0;

esp_timer_handle_t battery_timer_handle;

void app_main(void)
{   
    print_mac_adress_as_hex_string(); // Prints mac adress for hard coding as ESP-NOW peer.

    i2c_init_master(SDA_GPIO, SCL_GPIO, I2C_FREQ, 0);

    init_gpio();
    config_adc();

    /* set up timer for battery check */
    const esp_timer_create_args_t battery_timer = {
        .callback = battery_handler,
        .name = "battery handler"
    };
    esp_timer_create(&battery_timer, &battery_timer_handle);

    /* Sensor setup block */
    VL53L5CX_Configuration *sensor_right = malloc(sizeof(VL53L5CX_Configuration));
    config_sensor(sensor_right, 0, 1);
    VL53L5CX_Configuration *sensor_left = malloc(sizeof(VL53L5CX_Configuration));
    config_sensor(sensor_left, 0, 2);
   
    VL53L5CX_ResultsData *results_right = malloc(sizeof(VL53L5CX_ResultsData));
    VL53L5CX_ResultsData *results_left = malloc(sizeof(VL53L5CX_ResultsData));

    Reset_Sensor(&((*sensor_right).platform));
    Reset_Sensor(&((*sensor_left).platform));
    change_address_sensor(sensor_left);

    init_sensor_8x8(sensor_left, 15);
    init_sensor_8x8(sensor_right, 15);
   
    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_left);
    esp_now_add_peer_wrapper(mac_adress_right);

    char esp_now_send_buffer[MAX_ESP_NOW_SIZE];
    uint16_t grouped_results_sensor_right[6] = {0};
    uint16_t grouped_results_sensor_left[6] = {0};

    esp_timer_start_once(battery_timer_handle, 10000000); // Check battery every 10 seconds.
    //uint8_t height_adjustment = 0;
    //bool high_res_is_set = true; //Switch between 4x4 and 8x8.
    for(;;)
    {
        get_single_measurement_blocking(sensor_right, results_right);
        get_single_measurement_blocking(sensor_left, results_left);
        group_result_to_12segments_8x8(results_right, results_left, grouped_results_sensor_right, grouped_results_sensor_left);
        //adjust_for_height(&height_adjustment, grouped_results_sensor_right, grouped_results_sensor_left);
        esp_now_send_wrapper(grouped_results_sensor_right, grouped_results_sensor_left, esp_now_send_buffer, mac_adress_left, mac_adress_right);
        vTaskDelay(portTICK_PERIOD_MS); // This gives the system time to reset the WDT.
        /*if((gpio_get_level(DEC_BUTTON)) != (gpio_get_level(INC_BUTTON))) // One but not the other button is pressed.
        {
           adjust_height(&height_adjustment);
        }*/
    }
}

/* Initializes the gpios.
*/
void init_gpio(void)
{
    gpio_pad_select_gpio(LP_RIGHT_PIN);
    gpio_pad_select_gpio(INT_RIGHT_PIN);
    gpio_pad_select_gpio(RST_RIGHT_PIN);
    gpio_pad_select_gpio(PWR_ENABLE_RIGHT_PIN);
    gpio_pad_select_gpio(LP_LEFT_PIN);
    gpio_pad_select_gpio(INT_LEFT_PIN);
    gpio_pad_select_gpio(RST_LEFT_PIN);
    gpio_pad_select_gpio(PWR_ENABLE_LEFT_PIN);
    gpio_pad_select_gpio(BATTERY_INDICATOR_LED);
    ESP_ERROR_CHECK(gpio_set_direction(BATTERY_INDICATOR_LED, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(LP_RIGHT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(RST_RIGHT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(PWR_ENABLE_RIGHT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(INT_RIGHT_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(LP_LEFT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(RST_LEFT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(PWR_ENABLE_LEFT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(INT_LEFT_PIN, GPIO_MODE_INPUT));

    gpio_pad_select_gpio(INC_BUTTON);
    gpio_pad_select_gpio(DEC_BUTTON);
    ESP_ERROR_CHECK(gpio_set_direction(INC_BUTTON, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(DEC_BUTTON, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_pulldown_en(INC_BUTTON));
    ESP_ERROR_CHECK(gpio_pulldown_en(DEC_BUTTON));
    ESP_ERROR_CHECK(gpio_pullup_dis(INC_BUTTON));
    ESP_ERROR_CHECK(gpio_pullup_dis(DEC_BUTTON));

    ESP_LOGI("INIT_GPIO", "gpios set");
}

/* Checks if battery voltage is under specified limit.
** @param SDA_LINE          The gpio to use as Serial Data Line (SDA).
** @param SCL_LINE			The gpio to use as Serial Clock Line (SCL).
** @param FREQ			    The frequency to configure i2c to.
** @param PORT		        What i2c port to use.
*/
void i2c_init_master(const uint8_t SDA_LINE, const uint8_t SCL_LINE, const uint32_t FREQ, const uint8_t PORT)
{
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_GPIO,        
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = SCL_GPIO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_FREQ
    };

    ESP_ERROR_CHECK(i2c_param_config(0, &config));
    ESP_ERROR_CHECK(i2c_driver_install(0, I2C_MODE_MASTER, 0, 0, 0));
    ESP_LOGI("I2C_INIT_MASTER", "i2c configuration ran");
}

/*  Configures adc for reading of battery voltage. 
*/
void config_adc(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
}

/* Adjust the sensor results by the height adjustment specified by height_adjustment. 
** @param height_adjustment                     Pointer to the height adjustment variable
** @param grouped_results_sensor_right          Pointer to the results array for the right sensor
** @param grouped_results_sensor_left           Pointer to the results array for the left sensor
*/
void adjust_for_height(uint8_t* height_adjustment, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left)
{
    for(uint8_t i = 0; i < 6; i++)
    {
        if(grouped_results_sensor_right[i] >= *height_adjustment)
        {
            grouped_results_sensor_right[i] -= *height_adjustment;
        }
        if(grouped_results_sensor_left[i] >= *height_adjustment)
        {
            grouped_results_sensor_left[i] -= *height_adjustment;
        }
    }
}

/* Checks what button is pressed, debounces and inc/dec the height adjustment variable.
** @param height_adjustment         Pointer to the height adjustment variable
*/
void adjust_height(uint8_t* height_adjustment)
{
    if(gpio_get_level(DEC_BUTTON) == 1)
    {
        do{
            vTaskDelay(30/portTICK_PERIOD_MS);
        }while(gpio_get_level(DEC_BUTTON) == 1);

        if(*height_adjustment >= 10)
        {
            *height_adjustment -= 10;
        }
    }
    else if(gpio_get_level(INC_BUTTON) == 1)
    {
        do{
            vTaskDelay(30/portTICK_PERIOD_MS);
        }while(gpio_get_level(INC_BUTTON) == 1);

        if(*height_adjustment < 50)
        {
            *height_adjustment += 10;
        }
    }

    ESP_LOGI("adjust_height", "Height adjustment is set to %d\n", *height_adjustment);
}

/* Checks if battery voltage is under specified limit.
** @param limit				Limit to check if battery voltage is under.
** @param return			true if under limit, else false.
*/
bool is_battery_under_in_mv(const uint16_t limit)
{
    uint16_t raw_half_battery_voltage;
    raw_half_battery_voltage = adc1_get_raw(ADC1_CHANNEL_7);
    const uint16_t battery_voltage = (2 * 8 * raw_half_battery_voltage) / 10;  // now battery voltage in mV
    printf("Battery in mV is: %d\n", battery_voltage);
    if(battery_voltage >= limit)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/*****************************************************************************************************************************************/
/******************************************TIMER HANDLERS BENETH THIS LINE****************************************************************/
/*****************************************************************************************************************************************/

/* Reads battery voltage, loops if battery is low and send message to sender
** to indicate low battery. 
*/
void battery_handler(void* args)
{
    bool battery_is_under_limit = is_battery_under_in_mv(3850);
    if(battery_is_under_limit == true || low_device_battery_counter > 0)
    {
        gpio_set_level(BATTERY_INDICATOR_LED, ON);
    }
    else
    {
        gpio_set_level(BATTERY_INDICATOR_LED, OFF);
    }
    esp_timer_start_once(battery_timer_handle, 10000000); // Check battery every 10 seconds.
}

