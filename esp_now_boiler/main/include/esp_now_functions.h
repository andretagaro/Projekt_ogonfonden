#ifndef _ESP_NOW_FUNCTIONS_H
#define _ESP_NOW_FUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_now.h"

#define MAC_SIZE 6
#define MAX_ESP_NOW_SIZE 250

void mac_adress_to_string(char* my_mac_as_string, uint8_t* mac_adress_array);
void print_mac_adress_as_hex_string(void);
uint16_t delinearize(uint16_t distance_value);
void activate_esp_now(void);
void on_received_callback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void on_sent_callback(const uint8_t *sent_to_mac_addr, esp_now_send_status_t status);
void esp_now_add_peer_wrapper(uint8_t* mac_adress_sender);
void deactivate_corresponding_motor(uint8_t i);
void activate_corresponding_motor(uint8_t i);

#endif