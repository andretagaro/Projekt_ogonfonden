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

void config_sensor(VL53L5CX_Configuration* sensor_n, const uint8_t port, const uint8_t id);
void init_sensor_8x8(VL53L5CX_Configuration* sensor_n, uint8_t hz);
void init_sensor_4x4(VL53L5CX_Configuration* sensor_n, uint8_t hz);
void get_single_measurement_blocking(VL53L5CX_Configuration *sensor_n, VL53L5CX_ResultsData *results_n);
void print_sensor_data_8x8(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left);
void change_address_sensor(VL53L5CX_Configuration* sensor_to_change);
void group_result_to_12segments_8x8(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left);
void group_result_to_12segments_4x4(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left);
#endif