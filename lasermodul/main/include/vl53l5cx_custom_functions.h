#ifndef VL53L5CX_CUSTOM_FUNCTIONS
#define VL53L5CX_CUSTOM_FUNCTIONS

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

void init_sensor(VL53L5CX_Configuration* sensor_n);
void get_single_measurement_blocking(VL53L5CX_Configuration *sensor_n, VL53L5CX_ResultsData *results_n);
void print_sensor_data(VL53L5CX_ResultsData* results_1, VL53L5CX_ResultsData* results_2);
void change_address_sensor(VL53L5CX_Configuration* sensor_to_change);
void group_result_to_segments(VL53L5CX_ResultsData* results1, VL53L5CX_ResultsData* results2, uint16_t* group_results);
void print_segments(uint16_t* group_results);
#endif