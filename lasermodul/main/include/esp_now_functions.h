#ifndef _ESP_NOW_FUNCTIONS_
#define _ESP_NOW_FUNCTIONS_

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
void activate_esp_now(void);
void on_received_callback(const uint8_t *mac_addr, const uint8_t *data, int data_len);
void on_sent_callback(const uint8_t *sent_to_mac_addr, esp_now_send_status_t status);
void esp_now_add_peer_wrapper(uint8_t* mac_adress_sender);
void esp_now_send_wrapper(uint16_t* message, char* esp_now_send_buffer, uint8_t* mac_adress);

#endif