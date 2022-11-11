#include <stdio.h>
#include "esp_now_functions.h"

uint8_t mac_adress_sender[MAC_SIZE] = {0x7c,0xdf,0xa1,0xb4,0x43,0xcc};
uint8_t mac_adress_right[MAC_SIZE] = {0x84, 0xf7, 0x03, 0x0b, 0xdd, 0x5c};

void app_main(void)
{
    print_mac_adress_as_hex_string();

    for(uint8_t begin_loop = 3; begin_loop > 0; begin_loop--)
    {
        vTaskDelay(500/portTICK_PERIOD_MS);
        printf("Beggining in %d...\n", begin_loop);
    }

    gpio_pad_select_gpio(9);
    gpio_set_direction(9, GPIO_MODE_OUTPUT);
    gpio_set_level(9, 0);

    
    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_sender);
    char esp_now_send_buffer[MAX_ESP_NOW_SIZE];

    for(;;)
    {
        vTaskDelay(1);
    }
} 
