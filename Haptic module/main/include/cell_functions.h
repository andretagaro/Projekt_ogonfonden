#ifndef _CELL_FUNCTIONS_H
#define _CELL_FUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_now.h"
#include "esp_now_functions.h"
#include "esp_timer.h"
#include "global_defines.h"
#include "driver/adc.h"

/* Actives correspoding motor, placement of motors as following:
** 456 upper row 
** 123 lower row.
** @param i				The motor to turn on.
*/
void activate_corresponding_motor(uint8_t i);

/* Deactives correspoding motor, placement of motors as following:
** 456 upper row 
** 123 lower row.
** @param i				The motor to turn off.
*/
void deactivate_corresponding_motor(uint8_t i);

/* Checks if any range data is lower than the threshold,
** and updates the cells close state accordingly.
*/
void update_which_cells_are_close(void);

/* Switches all cells, i.e motors off.
*/
void all_cells_off(void);


/*****************************************************************************************************************************************/
/******************************************TIMER HANDLERS BENETH THIS LINE****************************************************************/
/*****************************************************************************************************************************************/

/* Updates the cells according to the range data from the laser module (sender peer),
** this function is responsible for switching the cells on and off to create a PWM signal.
** the integer cell_off_counter sets the OFF time, and cell_on_counter sets the ON time.
*/
void update_cells(void* args);

/* If an object is close and a cell should be pulsating this function sets if
** the cells should be pulse on of pulse off. This function only sets the state,
** the function 'update_cells' is doing the actual pulsing.
*/
void pulsate_cells(void *args);

#endif
