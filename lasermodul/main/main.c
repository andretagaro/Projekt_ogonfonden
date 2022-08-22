#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vl53l5cx_api.h"
#include "platform.h"
#include "driver/i2c.h"
#include "esp_log.h"


#define SDA_GPIO 2 
#define SCL_GPIO 3
#define I2C_FREQ 1000000

void i2c_init_master(const uint8_t SDA_LINE, const uint8_t SCL_LINE, const uint32_t FREQ, const uint8_t PORT);

void app_main(void)
{   
    for(uint8_t begin_loop = 10; begin_loop > 0; begin_loop--)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
        printf("Beggining in %d seconds\n", begin_loop);
    }

    i2c_init_master(SDA_GPIO, SCL_GPIO, I2C_FREQ, 0);
}


void i2c_init_master(const uint8_t SDA_LINE, const uint8_t SCL_LINE, const uint32_t FREQ, const uint8_t PORT)
{
    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = SDA_LINE,        
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = SCL_LINE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = FREQ,
    };

    ESP_ERROR_CHECK(i2c_param_config(PORT, &config));
    ESP_ERROR_CHECK(i2c_driver_install(PORT, I2C_MODE_MASTER, 0, 0, 0));

    ESP_LOGI("I2C_INIT_MASTER", "i2c configuration ran");
}