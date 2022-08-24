#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "vl53l5cx_api.h"
#include "platform.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "global_defines.h"
#include "driver/gpio.h"

void init_sensor(VL53L5CX_Configuration* Dev1);
void init_gpio(void);
void i2c_init_master(const uint8_t SDA_LINE, const uint8_t SCL_LINE, const uint32_t FREQ, const uint8_t PORT);
void get_single_measurement_blocking(VL53L5CX_Configuration *Dev, VL53L5CX_ResultsData 	*Results);
void print_sensor_data(VL53L5CX_ResultsData* Results_1, VL53L5CX_ResultsData* Results_2);

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

    VL53L5CX_Configuration *dev1 = malloc(sizeof *dev1);
    VL53L5CX_ResultsData *results1 = malloc(sizeof(VL53L5CX_ResultsData));
    VL53L5CX_ResultsData *results2 = malloc(sizeof(VL53L5CX_ResultsData));

    init_sensor(dev1);

    for(;;)
    {
       get_single_measurement_blocking(dev1, results1);
       //printf("done\n");
       get_single_measurement_blocking(dev1, results2);
       //printf("done second\n");
	   print_sensor_data(results1, results2);
       vTaskDelay(portTICK_PERIOD_MS);
    }
}

void init_sensor(VL53L5CX_Configuration* Dev1)
{
    uint8_t status = 0;

    Dev1->platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
    Dev1->platform.port = 0;

    Reset_Sensor(Dev1);

    ESP_LOGI("INIT_SENSOR", "GPIO_SET");

    //Init the sensors
    status |= vl53l5cx_init(Dev1);
    //Set resolution to 8x8
    status |= vl53l5cx_set_resolution(Dev1, VL53L5CX_RESOLUTION_8X8);
    //Set rangnig speed
    status |= vl53l5cx_set_ranging_frequency_hz(Dev1, 15);
    status |= vl53l5cx_set_ranging_mode(Dev1, VL53L5CX_RANGING_MODE_CONTINUOUS);
    status |= vl53l5cx_start_ranging(Dev1);

    if(status != 0)
    {
        printf("Error when initializing sensors.. :(\n"); 
    }
    else
    {
        printf("VL53L5CX ULD ready ! (Version : %s)\n\n", VL53L5CX_API_REVISION);
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

    ESP_ERROR_CHECK(gpio_set_direction(LP_1_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(RST_1_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(PWR_ENABLE_1_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_direction(INT_1_PIN, GPIO_MODE_INPUT));

    ESP_LOGI("INIT_GPIO", "gpios set");
}

void get_single_measurement_blocking(VL53L5CX_Configuration *Dev, VL53L5CX_ResultsData 	*Results)
{
  bool fail = false;
  uint8_t isReady;

  //Wait for data to be ready
  do
  {
    fail |= vl53l5cx_check_data_ready(Dev, &isReady);
    WaitMs(&(Dev->platform), 5);
  } while(!(isReady));

  fail |= vl53l5cx_get_ranging_data(Dev, Results);
 
  if(fail == true)
  {
    printf("Error when fetching data from sensor at adress ‰d", Dev->platform.address); 
  }
}

void print_sensor_data(VL53L5CX_ResultsData* Results_1, VL53L5CX_ResultsData* Results_2)
{
   for(uint8_t i = 0; i < 64; i = i + 8)
   {
      for(int8_t j = 7; j >= 0; j--)
      {
         if(Results_1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)] < 7)
         {
            printf("[XXXXXXXX]");
         }
         else
         {
           printf("[%2d:%5d]", (i+j), Results_1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)]);
         }
      }
        
      printf("        ");

      for(int8_t j = 7; j >= 0; j--)
      {
         if(Results_2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)] < 7)
         {
            printf("[XXXXXXXX]");
         }
         else
         {
           printf("[%2d:%5d]", (i+j), Results_2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)]);
         }
      }

      printf("\n\n");
   }

  printf("\n\n\n\n\n\n\n");
}