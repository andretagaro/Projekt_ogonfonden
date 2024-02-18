#include "esp_now_functions.h"
//extern uint8_t low_device_battery_counter;

/* Add peer to the ESP-NOW network.
** @param grouped_results_sensor_right				Results from right sensor.
** @param grouped_results_sensor_left				Results from left sensor..
** @param esp_now_send_buffer       				The buffer to send via ESP-NOW.
** @param mac_adress_left		            		Mac adress to the left module.
** @param mac_adress_right		                	Mac adress to the right module.
*/
void esp_now_send_wrapper(uint16_t* grouped_results_sensor_right, uint16_t* grouped_results_sensor_left, char* esp_now_send_buffer, uint8_t* mac_adress_left, uint8_t* mac_adress_right)
{
    sprintf(esp_now_send_buffer, "D%03d%03d%03d%03d%03d%03d", grouped_results_sensor_right[0], grouped_results_sensor_right[1], grouped_results_sensor_right[2],
    grouped_results_sensor_right[3], grouped_results_sensor_right[4], grouped_results_sensor_right[5]);
    //ESP_LOGE("Sent", "%s", esp_now_send_buffer);
    esp_now_send(mac_adress_right, (uint8_t*) esp_now_send_buffer, strlen(esp_now_send_buffer));

    sprintf(esp_now_send_buffer, "D%03d%03d%03d%03d%03d%03d", grouped_results_sensor_left[0], grouped_results_sensor_left[1], grouped_results_sensor_left[2],
    grouped_results_sensor_left[3], grouped_results_sensor_left[4], grouped_results_sensor_left[5]);
    esp_now_send(mac_adress_left, (uint8_t*) esp_now_send_buffer, strlen(esp_now_send_buffer));
}

/* Add peer to the ESP-NOW network.
** @param mac_adress				Mac adress of the peer to add.
*/
void esp_now_add_peer_wrapper(uint8_t* mac_adress_sender)
{
    esp_now_peer_info_t peer_to_add;
    memset(&peer_to_add, 0, sizeof(esp_now_peer_info_t));
    memcpy(peer_to_add.peer_addr, mac_adress_sender, MAC_SIZE);
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_to_add));
}

/* Converts a mac adress in integer array form to a string.
** @param mac_as_string				Mac adress as a string.
** @param mac_adress_array			The mac adress in an integer array.
*/
void mac_adress_to_string(char* my_mac_as_string, uint8_t* mac_adress_array)
{
    sprintf(my_mac_as_string, "%02x%02x%02x%02x%02x%02x", mac_adress_array[0], mac_adress_array[1], mac_adress_array[2], mac_adress_array[3], mac_adress_array[4], mac_adress_array[5]);
}

/* Prints the mac adress of the system as a hex string.
*/
void print_mac_adress_as_hex_string(void)
{
    uint8_t my_mac_adress[MAC_SIZE];
    ESP_ERROR_CHECK(esp_efuse_mac_get_default(my_mac_adress));
    char my_mac_as_string[(MAC_SIZE * 2) + 1];
    mac_adress_to_string(my_mac_as_string, my_mac_adress);
    printf("My mac address is: %s\n", my_mac_as_string);
}

/* Activates ESP-NOW and needed peripherals.
*/
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

/*****************************************************************************************************************************************/
/******************************************CALLBACKS BENETH THIS LINE*********************************************************************/
/*****************************************************************************************************************************************/

/* Callback when a message is received via ESP-NOW.
** @param mac_addr		mac adress of the peer who sent the message.
** @param data			the data that was received.
** @param data_len		amount of bytes received.
*/
void on_received_callback(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    char convert_mac_addr[(MAC_SIZE*2)+1];
    mac_adress_to_string(convert_mac_addr, (uint8_t*) mac_addr);
    printf("Received from mac adress: %s\n", convert_mac_addr);
    printf("The message is: %.*s\n\n", data_len, data);

    /*if(data[0] == 'L')
    {
        low_device_battery_counter++;
    }
    if(data[0] == 'H')
    {
        low_device_battery_counter--;
    }*/
}

/* Callback when a message is sent via ESP-NOW.
** @param sent_to_mac_addr		mac adress of the peer who the message was sent to.
** @param status				status of the sent message (i.e received or not received).
*/
void on_sent_callback(const uint8_t *sent_to_mac_addr, esp_now_send_status_t status)
{
    switch (status)
    {
    case ESP_NOW_SEND_SUCCESS: ;
        //char convert_mac_addr[(MAC_SIZE*2)+1];
        //mac_adress_to_string(convert_mac_addr, (uint8_t*) sent_to_mac_addr);
        //printf("Message sent to %s.\n", convert_mac_addr);
        break;
    case ESP_NOW_SEND_FAIL: ;
        char convert_mac_addr[(MAC_SIZE*2)+1];
        mac_adress_to_string(convert_mac_addr, (uint8_t*) sent_to_mac_addr);
        printf("Transmission to %s failed!\n\n", convert_mac_addr);
        break;
    default:
        break;
    }
}