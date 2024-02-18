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

/* Sets struct parameters for the sensor.
** @param sensor_n          pointer to the sensor to set parameters for.
** @param port  			The i2c port that should be used for the sensor.
** @param id   			    The internal id of the sensor, this is specified by user.
*/
void config_sensor(VL53L5CX_Configuration* sensor_n, const uint8_t port, const uint8_t id);

/* Changes the adress of specified sensor.
** @param sensor_to_change      ID of the sensor to change address of. 
*/
void change_address_sensor(VL53L5CX_Configuration* sensor_to_change);

/* Initializes the sensor in 4x4 mode and to specifed freqency.
** @param sensor_n         Pointer to the sensor the shall be initialized.
** @param hz  			   The freqency that the sensor should update sensor data at.
*/
void init_sensor_4x4(VL53L5CX_Configuration* sensor_n, uint8_t hz);

/* Initializes the sensor in 8x8 mode and to specifed freqency.
** @param sensor_n         Pointer to the sensor the shall be initialized.
** @param hz  			   The freqency that the sensor should update sensor data at.
*/
void init_sensor_8x8(VL53L5CX_Configuration* sensor_n, uint8_t hz);

/* Fetches a measurement in blocking mode.
** @param sensor_n         Pointer to the sensor the data should be fetched from.
** @param results_n  	   Pointer to the array where data is placed.
*/
void get_single_measurement_blocking(VL53L5CX_Configuration *sensor_n, VL53L5CX_ResultsData *results_n);

/* Prints results data to console in a grid layout.
** @param results_right         Results from the sensor that is pointing right.
** @param results_left          Results from the sensor that is pointing left. 
*/
void print_sensor_data_8x8(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left);

/* Groups results from two (4x4)-arrays into a two (6x2) arrays.
** @param results_right                         Results from the sensor that is pointing right.
** @param results_left                          Results from the sensor that is pointing left. 
** @param grouped_results_sensor_right          The (6x2) array of the right sensor.
** @param grouped_results_sensor_left           The (6x2) array of the left sensor.
*/
void group_result_to_12segments_4x4(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left);

/* Groups results from two (16x16)-arrays into a two (6x2) arrays.
** @param results_right                         Results from the sensor that is pointing right.
** @param results_left                          Results from the sensor that is pointing left. 
** @param grouped_results_sensor_right          The (6x2) array of the right sensor.
** @param grouped_results_sensor_left           The (6x2) array of the left sensor.
*/
void group_result_to_12segments_8x8(VL53L5CX_ResultsData* results_right, VL53L5CX_ResultsData* results_left, uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left);

#endif