#include <stdio.h>
#include "esp_now_functions.h"

uint8_t mac_adress_sender[MAC_SIZE] = {0x84, 0xf7,0x03,0x0b,0xd1, 0x2c};
uint8_t mac_adress_left[MAC_SIZE] = {0x84, 0xf7, 0x03, 0x0b, 0xdd, 0x5c}; // this is my mac.
uint8_t mac_adress_right[MAC_SIZE] = {0xfc, 0xf5, 0xc4, 0x09, 0x61, 0x90};

void app_main(void)
{
    print_mac_adress_as_hex_string();
    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_sender);
    esp_now_add_peer_wrapper(mac_adress_right);
    char esp_now_send_buffer[MAX_ESP_NOW_SIZE];
} 
