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

void deactivate_corresponding_motor(uint8_t i);
void activate_corresponding_motor(uint8_t i);
void all_cells_off(void);
void update_cells(void* args);
void update_which_cells_are_close(void);
void pulsate_cells(void *args);

#endif
