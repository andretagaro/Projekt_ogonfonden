#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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

uint8_t mac_adress_right[MAC_SIZE] = {0x8c, 0x4b, 0x14, 0x0e, 0xf4, 0x9c};
uint8_t mac_adress_left[MAC_SIZE] = {0x40, 0x91, 0x51, 0x2d, 0x0f, 0xa4};
uint8_t mac_adress_sender[MAC_SIZE] = {0xfc, 0xf5, 0xc4, 0x09, 0x61, 0x90};

void app_main(void)
{   
    print_mac_adress_as_hex_string();

    for(uint8_t begin_loop = 3; begin_loop > 0; begin_loop--)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
        printf("Beggining in %d...\n", begin_loop);
    }

    i2c_init_master(SDA_GPIO, SCL_GPIO, I2C_FREQ, 0);
    init_gpio();
    uint16_t half_battery_voltage = 0;
    config_adc();

    /* Sensor setup block */
    uint16_t grouped_results_sensor_right[6] = {0};
    uint16_t grouped_results_sensor_left[6] = {0};

    VL53L5CX_Configuration *sensor_right = malloc(sizeof(VL53L5CX_Configuration));
    sensor_right->platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
    sensor_right->platform.port = 0;
    sensor_right->platform.id = 1;

    VL53L5CX_Configuration *sensor_left = malloc(sizeof(VL53L5CX_Configuration));
    sensor_left->platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
    sensor_left->platform.port = 0;
    sensor_left->platform.id = 2;

    VL53L5CX_ResultsData *results_right = malloc(sizeof(VL53L5CX_ResultsData));
    VL53L5CX_ResultsData *results_left = malloc(sizeof(VL53L5CX_ResultsData));

    Reset_Sensor(&((*sensor_right).platform));
    Reset_Sensor(&((*sensor_left).platform));
    change_address_sensor(sensor_left);

    init_sensor(sensor_left);
    init_sensor(sensor_right);
    /* ---------------------------------- */

    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_left);
    esp_now_add_peer_wrapper(mac_adress_right);
    char esp_now_send_buffer[MAX_ESP_NOW_SIZE];


    for(;;)
    {
        get_single_measurement_blocking(sensor_right, results_right);
        get_single_measurement_blocking(sensor_left, results_left);
        group_result_to_segments_12_mode_even(results_right, results_left, grouped_results_sensor_right, grouped_results_sensor_left);
        esp_now_send_wrapper(grouped_results_sensor_right, grouped_results_sensor_left, esp_now_send_buffer, mac_adress_left, mac_adress_right);	
        vTaskDelay(portTICK_PERIOD_MS); // This gives the system time to reset the WDT.

        //half_battery_voltage = adc1_get_raw(ADC1_CHANNEL_7);
        //half_battery_voltage = (2 * 8 * half_battery_voltage) / 10;  
        //printf("Battery in mV is: %d\n", half_battery_voltage);

        /* debug section */
        //print_sensor_data(results_right, results_left);
        /* ------------- */
    }
}

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

    ESP_ERROR_CHECK(gpio_set_direction(LP_RIGHT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(RST_RIGHT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(PWR_ENABLE_RIGHT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(INT_RIGHT_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(LP_LEFT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(RST_LEFT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(PWR_ENABLE_LEFT_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(INT_LEFT_PIN, GPIO_MODE_INPUT));

    ESP_LOGI("INIT_GPIO", "gpios set");
}

void config_adc(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
}