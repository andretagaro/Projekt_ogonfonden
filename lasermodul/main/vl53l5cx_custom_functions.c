#include "vl53l5cx_custom_functions.h"

/* Sets struct parameters for the sensor.
** @param sensor_n          pointer to the sensor to set parameters for.
** @param port  			The i2c port that should be used for the sensor.
** @param id   			    The internal id of the sensor, this is specified by user.
*/
void config_sensor(VL53L5CX_Configuration* sensor_n, const uint8_t port, const uint8_t id)
{
    sensor_n->platform.address = VL53L5CX_DEFAULT_I2C_ADDRESS;
    sensor_n->platform.port = port;
    sensor_n->platform.id = id;
}

/* Changes the adress of specified sensor.
** @param sensor_to_change      ID of the sensor to change address of. 
*/
void change_address_sensor(VL53L5CX_Configuration* sensor_to_change)
{
    uint8_t status = 0;

    if(sensor_to_change->platform.id == 1)
    {
        ESP_ERROR_CHECK(gpio_set_level(LP_LEFT_PIN, OFF));
        status |=  vl53l5cx_set_i2c_address(sensor_to_change, SENSOR_CUSTOM_ADDRESS);
        ESP_ERROR_CHECK(gpio_set_level(LP_LEFT_PIN, ON));
    }
    else if(sensor_to_change->platform.id == 2)
    {
        ESP_ERROR_CHECK(gpio_set_level(LP_RIGHT_PIN, OFF));
        status |=  vl53l5cx_set_i2c_address(sensor_to_change, SENSOR_CUSTOM_ADDRESS);
        ESP_ERROR_CHECK(gpio_set_level(LP_RIGHT_PIN, ON));
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
        ESP_LOGI("ADDRESS CHANGE", "the address of sensor with id(%d) was changed", sensor_to_change->platform.id);
    }
}

/* Initializes the sensor in 4x4 mode and to specifed freqency.
** @param sensor_n         Pointer to the sensor the shall be initialized.
** @param hz  			   The freqency that the sensor should update sensor data at.
*/
void init_sensor_4x4(VL53L5CX_Configuration* sensor_n, uint8_t hz)
{
    uint8_t status = 0;

    //ESP_LOGI("INIT_SENSOR", "GPIO_SET");

    //Init the sensors
    status |= vl53l5cx_init(sensor_n);
    ESP_LOGI("INIT1", "OK");
    //Set resolution to 8x8
    status |= vl53l5cx_set_resolution(sensor_n, VL53L5CX_RESOLUTION_4X4);
    ESP_LOGI("INIT2", "OK");
    //Set rangnig speed
    if((hz >= 1) && (hz <= 60))
    {
        status |= vl53l5cx_set_ranging_frequency_hz(sensor_n, hz);
    }
    else
    {
        ESP_LOGE("init_sensor_8x8", "invalid freqency, input 1-60");
        return;
    }
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

/* Initializes the sensor in 8x8 mode and to specifed freqency.
** @param sensor_n         Pointer to the sensor the shall be initialized.
** @param hz  			   The freqency that the sensor should update sensor data at.
*/
void init_sensor_8x8(VL53L5CX_Configuration* sensor_n, uint8_t hz)
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
    if((hz >= 1) && (hz <= 60))
    {
        status |= vl53l5cx_set_ranging_frequency_hz(sensor_n, hz);
    }
    else
    {
        ESP_LOGE("init_sensor_8x8", "invalid freqency, input 1-60");
        return;
    }
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

/* Fetches a measurement in blocking mode.
** @param sensor_n         Pointer to the sensor the data should be fetched from.
** @param results_n  	   Pointer to the array where data is placed.
*/
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
    printf("Error when fetching data from sensor at adress %d", sensor_n->platform.address); 
  }
}


/* Prints results data to console in a grid layout.
** @param results_right         Results from the sensor that is pointing right.
** @param results_left          Results from the sensor that is pointing left. 
*/
void print_sensor_data_8x8(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left)
{
   for(uint8_t i = 0; i < 64; i = i + 8)
   {
      for(int8_t j = 7; j >= 0; j--)
      {
         if(results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)] < 30)
         {
            printf("[XXXXXXXX]");
         }
         else
         {
           printf("[%2d:%5d]", (i+j), results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)]);
         }
      }
        
      printf("        ");

      for(int8_t j = 7; j >= 0; j--)
      {
         if(results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)] < 30)
         {
            printf("[XXXXXXXX]");
         }
         else
         {
           printf("[%2d:%5d]", (i+j), results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*(i+j)]);
         }
      }

      printf("\n\n");
   }

  printf("\n\n\n\n\n\n\n");
}

/* Groups results from two (4x4)-arrays into a two (6x2) arrays.
** @param results_right                         Results from the sensor that is pointing right.
** @param results_left                          Results from the sensor that is pointing left. 
** @param grouped_results_sensor_right          The (6x2) array of the right sensor.
** @param grouped_results_sensor_left           The (6x2) array of the left sensor.
*/
void group_result_to_12segments_4x4(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left)
{
  grouped_results_sensor_right[0] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*15] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*11];
  grouped_results_sensor_right[0] = grouped_results_sensor_right[0] / 20;

  grouped_results_sensor_right[1] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*14] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*10];
  grouped_results_sensor_right[1] = grouped_results_sensor_right[0] / 20;

  grouped_results_sensor_right[2] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*13] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*12] + 
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*9] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*8];
  grouped_results_sensor_right[2] = grouped_results_sensor_right[0] / 40;

  grouped_results_sensor_right[3] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*3] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*7];
  grouped_results_sensor_right[3] = grouped_results_sensor_right[0] / 20;

  grouped_results_sensor_right[4] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*2] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*6];
  grouped_results_sensor_right[4] = grouped_results_sensor_right[0] / 20;

  grouped_results_sensor_right[5] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*4] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*5] + 
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*1] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*0];
  grouped_results_sensor_right[5] = grouped_results_sensor_right[0] / 40;

  //Left side
  grouped_results_sensor_right[0] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*15] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*14] + 
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*11] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*10];
  grouped_results_sensor_right[0] = grouped_results_sensor_right[0] / 40;

  grouped_results_sensor_right[1] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*13] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*9];
  grouped_results_sensor_right[1] = grouped_results_sensor_right[0] / 20;

  grouped_results_sensor_right[2] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*8] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*12];
  grouped_results_sensor_right[2] = grouped_results_sensor_right[0] / 20;

  grouped_results_sensor_right[3] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*3] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*2] + 
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*7] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*6];
  grouped_results_sensor_right[3] = grouped_results_sensor_right[0] / 40;

  grouped_results_sensor_right[4] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*1] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*5];
  grouped_results_sensor_right[4] = grouped_results_sensor_right[0] / 20;

  grouped_results_sensor_right[5] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*0] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*4];
  grouped_results_sensor_right[5] = grouped_results_sensor_right[0] / 20;
}

/* Groups results from two (16x16)-arrays into a two (6x2) arrays.
** @param results_right                         Results from the sensor that is pointing right.
** @param results_left                          Results from the sensor that is pointing left. 
** @param grouped_results_sensor_right          The (6x2) array of the right sensor.
** @param grouped_results_sensor_left           The (6x2) array of the left sensor.
*/
void group_result_to_12segments_8x8(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left)
{ 
  //Right side
  grouped_results_sensor_right[0] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*37] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*38] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*39] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*45] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*46] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*47] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*53] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*54] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*55] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*61] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*62] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*63];
  grouped_results_sensor_right[0] = grouped_results_sensor_right[0] / 120;

  grouped_results_sensor_right[1] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*34] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*35] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*36] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*42] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*43] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*44] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*50] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*52] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*52] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*58] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*59] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*60];
  grouped_results_sensor_right[1] = grouped_results_sensor_right[1] / 120;

  grouped_results_sensor_right[2] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*32] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*33] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*40] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*41] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*48] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*49] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*56] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*57];
  grouped_results_sensor_right[2] = grouped_results_sensor_right[2] / 80;

  grouped_results_sensor_right[3] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*5] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*6] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*7] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*13] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*14] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*15] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*21] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*22] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*23] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*29] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*30] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*31];
  grouped_results_sensor_right[3] = grouped_results_sensor_right[3] / 120;

  grouped_results_sensor_right[4] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*2] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*3] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*4] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*10] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*11] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*12] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*18] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*19] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*20] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*26] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*27] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*28];
  grouped_results_sensor_right[4] = grouped_results_sensor_right[4] / 120;

  grouped_results_sensor_right[5] = results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*0] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*1] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*8] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*9] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*16] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*17] +
                                    results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*24] + results_right->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*25];
  grouped_results_sensor_right[5] = grouped_results_sensor_right[5] / 80;


  //Left side
  grouped_results_sensor_left[0] = results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*38] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*39] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*46] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*47] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*54] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*55] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*62] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*63];
  grouped_results_sensor_left[0] = grouped_results_sensor_left[0] / 80;
  
  grouped_results_sensor_left[1] = results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*35] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*36] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*37] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*43] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*44] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*45] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*51] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*52] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*53] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*59] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*60] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*61];
  grouped_results_sensor_left[1] = grouped_results_sensor_left[1] / 120;

  grouped_results_sensor_left[2] = results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*32] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*33] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*34] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*40] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*41] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*42] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*48] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*49] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*50] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*56] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*57] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*58];
  grouped_results_sensor_left[2] = grouped_results_sensor_left[2] / 120;

  grouped_results_sensor_left[3] = results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*6] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*7] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*14] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*15] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*22] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*23] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*30] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*31];
  grouped_results_sensor_left[3] = grouped_results_sensor_left[3] / 80;

  grouped_results_sensor_left[4] = results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*3] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*4] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*5] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*11] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*12] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*13] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*19] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*20] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*21] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*27] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*28] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*29];
  grouped_results_sensor_left[4] = grouped_results_sensor_left[4] / 120;


  grouped_results_sensor_left[5] = results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*0] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*1] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*2] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*8] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*9] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*10] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*16] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*17] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*18] +
                                   results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*24] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*25] + results_left->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*26];
  grouped_results_sensor_left[5] = grouped_results_sensor_left[5] / 120;
}