#ifndef _ESP_NOW_FUNCTIONS_H
#define _ESP_NOW_FUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "esp_now.h"

#include "global_defines.h"
#include "cell_functions.h"

#define MAC_SIZE 6
#define MAX_ESP_NOW_SIZE 250

/* Add peer to the ESP-NOW network.
** @param mac_adress				Mac adress of the peer to add.
*/
void esp_now_add_peer_wrapper(uint8_t* mac_adress_sender);

/* Converts a mac adress in integer array form to a string.
** @param mac_as_string				Mac adress as a string.
** @param mac_adress_array			The mac adress in an integer array.
*/
void mac_adress_to_string(char* my_mac_as_string, uint8_t* mac_adress_array);

/* Prints the mac adress of the system as a hex string.
*/
void print_mac_adress_as_hex_string(void);

/* Activates ESP-NOW and needed peripherals.
*/
void activate_esp_now(void);

/* Converts the range data with a non linear function.
** @param distance_value		the value to delinearize.
** @return						the delinearized value.
*/
uint16_t delinearize(uint16_t distance_value);

/*****************************************************************************************************************************************/
/******************************************CALLBACKS BENETH THIS LINE*********************************************************************/
/*****************************************************************************************************************************************/

/* Callback when a message is received via ESP-NOW.
** @param mac_addr		mac adress of the peer who sent the message.
** @param data			the data that was received.
** @param data_len		amount of bytes received.
*/
void on_received_callback(const uint8_t *mac_addr, const uint8_t *data, int data_len);

/* Callback when a message is sent via ESP-NOW.
** @param sent_to_mac_addr		mac adress of the peer who the message was sent to.
** @param status				status of the sent message (i.e received or not received).
*/
void on_sent_callback(const uint8_t *sent_to_mac_addr, esp_now_send_status_t status);


#endif