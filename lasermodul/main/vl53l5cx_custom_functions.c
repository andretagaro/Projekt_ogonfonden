#include "vl53l5cx_custom_functions.h"

void change_address_sensor(VL53L5CX_Configuration* sensor_to_change)
{
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

void group_result_to_segments(VL53L5CX_ResultsData* results1, VL53L5CX_ResultsData* results2, uint16_t* group_results)
{
	//Bottom row
	group_results[9] = results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*0] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*1] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*2] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*8] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*9] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*10] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*16] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*17] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*18] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*24] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*25] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*26];

	group_results[8] = results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*3] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*4] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*5] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*11] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*12] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*13] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*19] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*20] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*21] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*27] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*28] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*29];

	group_results[7] = results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*6] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*7] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*0] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*1] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*14] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*15] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*8] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*9] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*22] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*23] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*16] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*17] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*30] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*31] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*24] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*25];

	group_results[6] = results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*2] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*3] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*4] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*10] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*11] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*12] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*18] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*19] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*20] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*26] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*27] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*28];

	group_results[5] = results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*5] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*6] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*7] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*13] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*14] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*15] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*21] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*22] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*23] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*29] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*30] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*31];

									

	//Top row
	group_results[4] = results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*32] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*33] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*34] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*40] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*41] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*42] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*48] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*49] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*50] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*56] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*57] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*58];

	group_results[3] = results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*35] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*36] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*37] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*43] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*44] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*45] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*51] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*52] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*53] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*59] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*60] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*61];

	group_results[2] = results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*38] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*39] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*32] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*33] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*46] + results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*47] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*40] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*41] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*54] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*55] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*48] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*49] +
										 results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*62] +  results1->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*63] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*56] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*57];

	group_results[1] = results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*34] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*35] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*36] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*42] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*43] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*44] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*50] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*51] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*52] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*58] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*59] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*60];

	group_results[0] = results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*37] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*38] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*39] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*45] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*46] + results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*47] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*53] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*54] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*55] +
										 results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*61] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*62] +  results2->distance_mm[VL53L5CX_NB_TARGET_PER_ZONE*63];


	//Calculate avarage.. and convert to cm.
	for(int i = 0; i < 10; i++)
	{
		if(group_results[i] >= 160)
		{
			if(i != 2 || i != 7)
			{
				group_results[i] = (uint16_t)(group_results[i] / 120);
			}
			else
			{
				group_results[i] = (uint16_t)(group_results[i] / 160);
			}
		}
		else
		{
			group_results[i] = 0;
		}
	}
}

void print_segments(uint16_t* group_results)
{
	for(int i = 9; i >= 0; i--)
	{
		printf("%d:[%03d]", i, group_results[i]);

		if(i == 5)
		{
			printf("\n");
		}
	}

	printf("\n\n");
}