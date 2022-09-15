#include <stdio.h>
#include "esp_now_functions.h"

uint8_t mac_adress_right[MAC_SIZE] = {0x8c, 0x4b, 0x14, 0x0e, 0xf4, 0x9c};
uint8_t mac_adress_left[MAC_SIZE] = {0x40, 0x91, 0x51, 0x2d, 0x0f, 0xa4};
uint8_t mac_adress_sender[MAC_SIZE] = {0xfc, 0xf5, 0xc4, 0x09, 0x61, 0x90};

void app_main(void)
{
    print_mac_adress_as_hex_string();

    for(uint8_t begin_loop = 3; begin_loop > 0; begin_loop--)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
        printf("Beggining in %d...\n", begin_loop);
    }
    
    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_sender);
    char esp_now_send_buffer[MAX_ESP_NOW_SIZE];
} 
