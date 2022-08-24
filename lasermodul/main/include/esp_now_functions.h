#ifndef _ESP_NOW_FUNCTIONS_
#define _ESP_NOW_FUNCTIONS_

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
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_now.h"

void mac_adress_to_string(char* my_mac_as_string, uint8_t* mac_adress_array);

void print_mac_adress_as_hex_string(void);


#endif