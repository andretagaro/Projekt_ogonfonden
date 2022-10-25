#include <stdio.h>
#include "esp_now_functions.h"
#include "esp_timer.h"
#include "global_defines.h"
#include "driver/adc.h"
#include "cell_functions.h"

uint8_t mac_adress_right[MAC_SIZE] = {0x8c, 0x4b, 0x14, 0x0e, 0xf4, 0x9c};
uint8_t mac_adress_left[MAC_SIZE] = {0x40, 0x91, 0x51, 0x2d, 0x0f, 0xa4};
uint8_t mac_adress_sender[MAC_SIZE] = {0xfc, 0xf5, 0xc4, 0x09, 0x61, 0x90};

void init_gpio(void);
void config_adc(void);

bool is_battery_under_in_mv(uint16_t limit);
void wait_for_reception(void);
void no_data_received(void);

/*Timer handlers*/
void battery_handler(void* args);
void reception_handler(void* args);


esp_timer_handle_t update_cells_timer_handle;
esp_timer_handle_t pulsate_cells_timer_handle;
esp_timer_handle_t battery_timer_handle;
esp_timer_handle_t reception_timer_handle;

extern cell cells[];
uint64_t us_since_last_reception = 0;
bool timers_on = false;

void app_main(void)
{
    init_gpio();

    /*Timer for update cells, this timer is responsible for setting the PWM freqency */
    const esp_timer_create_args_t update_cells_timer = {
        .callback = update_cells,
        .name = "update cells"
    };
    esp_timer_create(&update_cells_timer, &update_cells_timer_handle);

	/*Timer for pulsating cells, this timers sets the freqency of pulses when objects are close*/
    const esp_timer_create_args_t pulsate_cells_timer = {
    .callback = pulsate_cells,
    .name = "pulsate cells"
    };
    esp_timer_create(&pulsate_cells_timer, &pulsate_cells_timer_handle);

	/*This timer sets how often the battery voltage should be checked*/
    const esp_timer_create_args_t battery_timer = {
        .callback = battery_handler,
        .name = "battery handler"
    };
    esp_timer_create(&battery_timer, &battery_timer_handle);
	
	/*This timer sets how often we should check if reception has occured*/
	const esp_timer_create_args_t reception_timer = {
        .callback = reception_handler,
        .name = "reception_handler"
    };
    esp_timer_create(&reception_timer, &reception_timer_handle);

    print_mac_adress_as_hex_string(); // Print mac adress for hard coding as a ESP-NOW peer.

    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_sender);
	
	config_adc(); // Config adc for battery reading.

    //esp_timer_start_once(battery_timer_handle, 10000000);
	esp_timer_start_once(reception_timer_handle, RECEPTION_TIMER_UPDATE_TIME);
    for(;;)
    {
		if(us_since_last_reception >= RECEPTION_THRESHOLD)
		{
			esp_timer_stop(reception_timer_handle);
			wait_for_reception();
			esp_timer_start_once(reception_timer_handle, RECEPTION_TIMER_UPDATE_TIME);
		}
		update_which_cells_are_close();
		vTaskDelay(1); // This is needed to reset WDT in RTOS.
    }
}

/* Initializes the gpios.
*/
void init_gpio()
{
    gpio_pad_select_gpio(CELL_0);
	gpio_pad_select_gpio(CELL_1);
	gpio_pad_select_gpio(CELL_2);
	gpio_pad_select_gpio(CELL_3);
	gpio_pad_select_gpio(CELL_4);
	gpio_pad_select_gpio(CELL_5);

    ESP_ERROR_CHECK(gpio_set_direction(CELL_0, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(CELL_1, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(CELL_2, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(CELL_3, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(CELL_4, GPIO_MODE_OUTPUT));
	ESP_ERROR_CHECK(gpio_set_direction(CELL_5, GPIO_MODE_OUTPUT));

    gpio_set_level(CELL_0, OFF);
	gpio_set_level(CELL_1, OFF);
	gpio_set_level(CELL_2, OFF);
	gpio_set_level(CELL_3, OFF);
	gpio_set_level(CELL_4, OFF);
	gpio_set_level(CELL_5, OFF);
}

/*  Configures adc for reading of battery voltage. 
*/
void config_adc(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
}

/* Checks if battery voltage is under specified limit.
** @param limit				Limit to check if battery voltage is under.
** @param return			true if under limit, else false.
*/
bool is_battery_under_in_mv(uint16_t limit)
{
    uint16_t half_battery_voltage;
    half_battery_voltage = adc1_get_raw(ADC1_CHANNEL_7);
    half_battery_voltage = (2 * 8 * half_battery_voltage) / 10;  // now battery voltage in mV
    ESP_LOGI("BATTERY", "%d mV\n", half_battery_voltage);
    if(half_battery_voltage >= limit)
    {
        return false;
    }
    else
    {
        return true;
    }
}

/* Waits for reception if no reception has occured within the threshold.
*/
void wait_for_reception(void)
{	
	esp_timer_stop(update_cells_timer_handle);
	esp_timer_stop(pulsate_cells_timer_handle);
	timers_on = false;
	all_cells_off();
	while(us_since_last_reception >= RECEPTION_THRESHOLD)
	{
		ESP_LOGI("wait_for_reception", "no data arrived...");
		no_data_received();
	}
}

/* Pulsated cells in a specific order to indicate that no reception has occured.
*/
void no_data_received(void)
{
	gpio_set_level(CELL_0, ON);
	vTaskDelay(100/portTICK_PERIOD_MS);
	gpio_set_level(CELL_0, OFF);
	vTaskDelay(400/portTICK_PERIOD_MS);
	gpio_set_level(CELL_5, ON);
	vTaskDelay(100/portTICK_PERIOD_MS);
	gpio_set_level(CELL_5, OFF);
	vTaskDelay(400/portTICK_PERIOD_MS);
}


/*****************************************************************************************************************************************/
/******************************************TIMER HANDLERS BENETH THIS LINE****************************************************************/
/*****************************************************************************************************************************************/

/* Reads battery voltage, loops if battery is low and send message to sender
** to indicate low battery. 
*/
void battery_handler(void* args)
{
	bool is_battery_low = is_battery_under_in_mv(BATT_LIMIT);
	if(is_battery_low == true)
	{
		ESP_LOGW("BATTERY", "LOW");

		char esp_now_send_buffer[MAX_ESP_NOW_SIZE];
		sprintf(esp_now_send_buffer, "L");
		esp_now_send(mac_adress_sender, (uint8_t*) esp_now_send_buffer, strlen(esp_now_send_buffer));

		esp_timer_stop(update_cells_timer_handle);
		esp_timer_stop(pulsate_cells_timer_handle);
		all_cells_off();
		while(is_battery_under_in_mv(BATT_LIMIT))
		{	
			vTaskDelay(10000/portTICK_PERIOD_MS);
		}
		timers_on = false;
		sprintf(esp_now_send_buffer, "H");
		esp_now_send(mac_adress_sender, (uint8_t*) esp_now_send_buffer, strlen(esp_now_send_buffer));
	}
	esp_timer_start_once(battery_timer_handle, 10000000); // Check battery every 10 seconds.
}


/* Keeps track of amount of micro seconds since last reception.
*/
void reception_handler(void* args)
{
	us_since_last_reception += RECEPTION_TIMER_UPDATE_TIME;
	esp_timer_start_once(reception_timer_handle, RECEPTION_TIMER_UPDATE_TIME);
}


