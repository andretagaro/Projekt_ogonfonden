#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vl53l5cx_api.h"
#include "platform.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "global_defines.h"
#include "driver/gpio.h"

void init_sensor(VL53L5CX_Configuration* sensor_n);
void init_gpio(void);
void i2c_init_master(const uint8_t SDA_LINE, const uint8_t SCL_LINE, const uint32_t FREQ, const uint8_t PORT);
void get_single_measurement_blocking(VL53L5CX_Configuration *sensor_n, VL53L5CX_ResultsData *results_n);
void print_sensor_data(VL53L5CX_ResultsData* results_1, VL53L5CX_ResultsData* results_2);
void change_address_sensor(VL53L5CX_Configuration* sensor_to_change);

void app_main(void)
{   
    for(uint8_t begin_loop = 3; begin_loop > 0; begin_loop--)
    {
        vTaskDelay(1000/portTICK_PERIOD_MS);
        printf("Beggining in %d seconds\n", begin_loop);
    }
    
    i2c_init_master(SDA_GPIO, SCL_GPIO, I2C_FREQ, 0);
    int test;
    i2c_get_timeout(0, &test);
    init_gpio();

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

    for(;;)
    {
       get_single_measurement_blocking(sensor_1, results_1);
       get_single_measurement_blocking(sensor_2, results_2);
	   print_sensor_data(results_1, results_2);
       vTaskDelay(portTICK_PERIOD_MS);
    }
}

void change_address_sensor(VL53L5CX_Configuration* sensor_to_change)
{
    printf("%d\n", sensor_to_change->platform.id);

    uint8_t status = 0;

    if(sensor_to_change->platform.id == 1)
    {
        ESP_ERROR_CHECK(gpio_set_level(LP_2_PIN, OFF));
        status |=  vl53l5cx_set_i2c_address(sensor_to_change, SENSOR_2_ADDRESS);
        ESP_ERROR_CHECK(gpio_set_level(LP_2_PIN, ON));
    }
    else if(sensor_to_change->platform.id == 2)
    {
        ESP_ERROR_CHECK(gpio_set_level(LP_1_PIN, OFF));
        status |=  vl53l5cx_set_i2c_address(sensor_to_change, SENSOR_2_ADDRESS);
        ESP_ERROR_CHECK(gpio_set_level(LP_1_PIN, ON));
    }
    else
    {
        ESP_LOGW("ADDRESS CHANGE", "unknown sensor id");
    }

    if(status != 0)
    {
        ESP_LOGE("ADDRESS CHANGE", "could not change address");
    }
    else
    {
        ESP_LOGI("ADDRESS CHANGE", "the address of %d was changed", sensor_to_change->platform.id);
    }
}

void init_sensor(VL53L5CX_Configuration* sensor_n)
{
    uint8_t status = 0;

    //ESP_LOGI("INIT_SENSOR", "GPIO_SET");

    //Init the sensors
    status |= vl53l5cx_init(sensor_n);
    ESP_LOGI("INIT1", "OK");
    //Set resolution to 8x8
    status |= vl53l5cx_set_resolution(sensor_n, VL53L5CX_RESOLUTION_8X8);
    ESP_LOGI("INIT2", "OK");
    //Set rangnig speed
    status |= vl53l5cx_set_ranging_frequency_hz(sensor_n, 15);
    ESP_LOGI("INIT3", "OK");
    status |= vl53l5cx_set_ranging_mode(sensor_n, VL53L5CX_RANGING_MODE_CONTINUOUS);
    ESP_LOGI("INIT4", "OK");
    status |= vl53l5cx_start_ranging(sensor_n);
    ESP_LOGI("INIT5", "OK");

    if(status != 0)
    {
        printf("Error when initializing sensors.. :(\n"); 
    }
    else
    {
        printf("Sensor VL53L5CX ULD ready ! (Version : %s)\n\n", VL53L5CX_API_REVISION);
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

void get_single_measurement_blocking(VL53L5CX_Configuration *sensor_n, VL53L5CX_ResultsData *results_n)
{
  bool fail = false;
  uint8_t isReady;

  //Wait for data to be ready
  do
  {
    fail |= vl53l5cx_check_data_ready(sensor_n, &isReady);
    WaitMs(&(sensor_n->platform), 5);
  } while(!(isReady));

  fail |= vl53l5cx_get_ranging_data(sensor_n, results_n);
 
  if(fail == true)
  {
    printf("Error when fetching data from sensor at adress ‰d", sensor_n->platform.address); 
  }
}

void print_sensor_data(VL53L5CX_ResultsData* results_1, VL53L5CX_ResultsData* results_2)
{
   for(uint8_t i = 0; i < 64; i = i + 8)
   {
      for(int8_t j = 7; j >= 0; j--)
      {
         if(results_1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)] < 30)
         {
            printf("[XXXXXXXX]");
         }
         else
         {
           printf("[%2d:%5d]", (i+j), results_1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)]);
         }
      }
        
      printf("        ");

      for(int8_t j = 7; j >= 0; j--)
      {
         if(results_2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)] < 30)
         {
            printf("[XXXXXXXX]");
         }
         else
         {
           printf("[%2d:%5d]", (i+j), results_2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)]);
         }
      }

      printf("\n\n");
   }

  printf("\n\n\n\n\n\n\n");
}