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

void init_gpio(void);
void i2c_init_master(const uint8_t SDA_LINE, const uint8_t SCL_LINE, const uint32_t FREQ, const uint8_t PORT);

uint8_t mac_adress_sender[MAC_SIZE] = {0x84,0xf7,0x03,0x0b,0xd1,0x2c};
uint8_t mac_adress_left[MAC_SIZE]; // not defined yet
uint8_t mac_adress_right[MAC_SIZE]; // not defined yet

void app_main(void)
{   
    for(uint8_t begin_loop = 3; begin_loop > 0; begin_loop--)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
        printf("Beggining in %d...\n", begin_loop);
    }

    print_mac_adress_as_hex_string();
    i2c_init_master(SDA_GPIO, SCL_GPIO, I2C_FREQ, 0);
    init_gpio();

    /*Sensor setup block, place in function? */

    VL53L5CX_Configuration *sensor_1 = malloc(sizeof(VL53L5CX_Configuration));
    sensor_1->platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
    sensor_1->platform.port = 0;
    sensor_1->platform.id = 1;

    VL53L5CX_Configuration *sensor_2 = malloc(sizeof(VL53L5CX_Configuration));
    sensor_2->platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
    sensor_2->platform.port = 0;
    sensor_2->platform.id = 2;

    VL53L5CX_ResultsData *results_1 = malloc(sizeof(VL53L5CX_ResultsData));
    VL53L5CX_ResultsData *results_2 = malloc(sizeof(VL53L5CX_ResultsData));

    Reset_Sensor(&((*sensor_1).platform));
    Reset_Sensor(&((*sensor_2).platform));
    change_address_sensor(sensor_2);

    init_sensor(sensor_2);
    init_sensor(sensor_1);

    /* ---------------------------------- */

    uint16_t grouped_results[10] = {0};

    for(;;)
    {
       get_single_measurement_blocking(sensor_1, results_1);
       get_single_measurement_blocking(sensor_2, results_2);
       group_result_to_segments(results_1, results_2, grouped_results);
	   
       /* debug section */
       // print_segments(grouped_results);
       // print_sensor_data(results_1, results_2);
       /* ------------- */

        /* Send data via ESP NOW*/  
       vTaskDelay(portTICK_PERIOD_MS); // This gives the system time to reset to WDT.
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
    gpio_pad_select_gpio(LP_1_PIN);
    gpio_pad_select_gpio(INT_1_PIN);
    gpio_pad_select_gpio(RST_1_PIN);
    gpio_pad_select_gpio(PWR_ENABLE_1_PIN);
    gpio_pad_select_gpio(LP_2_PIN);
    gpio_pad_select_gpio(INT_2_PIN);
    gpio_pad_select_gpio(RST_2_PIN);
    gpio_pad_select_gpio(PWR_ENABLE_2_PIN);

    ESP_ERROR_CHECK(gpio_set_direction(LP_1_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(RST_1_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(PWR_ENABLE_1_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(INT_1_PIN, GPIO_MODE_INPUT));
    ESP_ERROR_CHECK(gpio_set_direction(LP_2_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(RST_2_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(PWR_ENABLE_2_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(INT_2_PIN, GPIO_MODE_INPUT));

    ESP_LOGI("INIT_GPIO", "gpios set");
}
