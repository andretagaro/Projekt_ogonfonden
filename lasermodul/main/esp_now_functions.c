#include "esp_now_functions.h"

void esp_now_send_wrapper(uint16_t* message, char* esp_now_send_buffer, uint8_t* mac_adress_left, uint8_t* mac_adress_right)
{
    //Send to left
    sprintf(esp_now_send_buffer, "\n|%d|%d|%d|\n|%d|%d|%d|", message[9], message[8], 
    message[7], message[4], message[3], message[2]);
    esp_now_send(mac_adress_left, (uint8_t*) esp_now_send_buffer, strlen(esp_now_send_buffer));

    //Send to right
    sprintf(esp_now_send_buffer, "\n|%d|%d|%d|\n|%d|%d|%d|", message[7], message[6], 
    message[5], message[2], message[1], message[0]);
    esp_now_send(mac_adress_right, (uint8_t*) esp_now_send_buffer, strlen(esp_now_send_buffer));
}

void esp_now_add_peer_wrapper(uint8_t* mac_adress_sender)
{
    esp_now_peer_info_t peer_to_add;
    memset(&peer_to_add, 0, sizeof(esp_now_peer_info_t));
    memcpy(peer_to_add.peer_addr, mac_adress_sender, MAC_SIZE);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_to_add));
}

void mac_adress_to_string(char* my_mac_as_string, uint8_t* mac_adress_array)
{
    sprintf(my_mac_as_string, "%02x%02x%02x%02x%02x%02x", mac_adress_array[0], mac_adress_array[1], mac_adress_array[2], mac_adress_array[3], mac_adress_array[4], mac_adress_array[5]);
}

void print_mac_adress_as_hex_string(void)
{
    uint8_t my_mac_adress[MAC_SIZE];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(my_mac_adress));
    char my_mac_as_string[(MAC_SIZE * 2) + 1];
    mac_adress_to_string(my_mac_as_string, my_mac_adress);
    printf("My mac address is: %s\n", my_mac_as_string);
}

void activate_esp_now(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t w_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&w_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    //start esp_now
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_sent_callback));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_received_callback));
}

void on_received_callback(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char convert_mac_addr[(MAC_SIZE*2)+1];
    mac_adress_to_string(convert_mac_addr, (uint8_t*) mac_addr);
    printf("Received from mac adress: %s\n", convert_mac_addr);
    printf("The message is: %.*s\n\n", data_len, data);
}

void on_sent_callback(const uint8_t *sent_to_mac_addr, esp_now_send_status_t status)
{
    switch (status)
    {
    case ESP_NOW_SEND_SUCCESS: ;
        char convert_mac_addr[(MAC_SIZE*2)+1];
        mac_adress_to_string(convert_mac_addr, (uint8_t*) sent_to_mac_addr);
        printf("Message sent to %s.\n", convert_mac_addr);
        break;
    case ESP_NOW_SEND_FAIL:
        printf("Transmission failed! :(\n\n");
        break;
    default:
        break;
    }
}