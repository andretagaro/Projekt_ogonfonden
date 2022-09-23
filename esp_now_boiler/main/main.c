#include <stdio.h>
#include "esp_now_functions.h"
#include "esp_timer.h"
#include "global_defines.h"
#include "driver/adc.h"

uint8_t mac_adress_right[MAC_SIZE] = {0x8c, 0x4b, 0x14, 0x0e, 0xf4, 0x9c};
uint8_t mac_adress_left[MAC_SIZE] = {0x40, 0x91, 0x51, 0x2d, 0x0f, 0xa4};
uint8_t mac_adress_sender[MAC_SIZE] = {0xfc, 0xf5, 0xc4, 0x09, 0x61, 0x90};

void init_gpio(void);
void update_cells(void* args);
void activate_corresponding_motor(uint8_t i);
void deactivate_corresponding_motor(uint8_t i);
void update_which_cells_are_close(void);
void pulsate_cells(void *args);
void all_cells_off(void);
void wait_for_reception(void);
void all_cells_off(void);
void no_data_received(void);
void config_adc(void);
bool is_battery_under_in_mv(uint16_t limit);
void check_battery(void);

cell cells[CELL_AMOUNT] = {0};
esp_timer_handle_t update_cells_timer_handle;
esp_timer_handle_t pulsate_cells_timer_handle;
uint16_t count_since_last_reception = 0;
bool timers_on = false;

void app_main(void)
{
    init_gpio();

    //Timer for update cells
    const esp_timer_create_args_t update_cells_timer = {
        .callback = update_cells,
        .name = "update cells"
    };
    esp_timer_create(&update_cells_timer, &update_cells_timer_handle);

    const esp_timer_create_args_t pulsate_cells_timer = {
    .callback = pulsate_cells,
    .name = "pulsate cells"
    };
    esp_timer_create(&pulsate_cells_timer, &pulsate_cells_timer_handle);

    print_mac_adress_as_hex_string();
    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_sender);
    esp_now_add_peer_wrapper(mac_adress_left);

	config_adc();

    uint8_t check_battery_level_counter = 255;
    for(;;)
    {
		count_since_last_reception++;
		if(count_since_last_reception >= RECEPTION_THRESHOLD)
		{
			wait_for_reception();
		}
		if(check_battery_level_counter == 255)
        {   
            check_battery_level_counter = 0;
			check_battery();
        }
        check_battery_level_counter++;
		update_which_cells_are_close();
		vTaskDelay(1);
    }
}

void check_battery(void)
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
}

void config_adc(void)
{
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_7, ADC_ATTEN_DB_11);
}

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

void wait_for_reception(void)
{	
	esp_timer_stop(update_cells_timer_handle);
	esp_timer_stop(pulsate_cells_timer_handle);
	timers_on = false;
	all_cells_off();
	while(count_since_last_reception >= RECEPTION_THRESHOLD)
	{
		ESP_LOGI("wait_for_reception", "no data arrived...");
		check_battery();
		no_data_received();
	}
}

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

void all_cells_off(void)
{
	gpio_set_level(CELL_0, OFF);
	gpio_set_level(CELL_1, OFF);
	gpio_set_level(CELL_2, OFF);
	gpio_set_level(CELL_3, OFF);
	gpio_set_level(CELL_4, OFF);
	gpio_set_level(CELL_5, OFF);
}

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

void update_cells(void* args)
{	
	for(uint8_t i = 0; i < CELL_AMOUNT; i++)
	{
		if(cells[i].cell_state == false)
		{
			if(cells[i].cell_off_counter > 0)
			{
				cells[i].cell_off_counter--;
			}
			if(cells[i].cell_off_counter == 0)
			{
				cells[i].cell_off_counter = cells[i].cell_value;

				if(cells[i].cell_close == false || (cells[i].cell_close && cells[i].cell_close_state))
				{
					activate_corresponding_motor(i);
					cells[i].cell_state = true;
				}
			}
		}
		else
		{
			if(cells[i].cell_on_counter > 0)
			{
				cells[i].cell_on_counter--;
			}
			if(cells[i].cell_on_counter == 0)
			{
				cells[i].cell_on_counter = MAX_VALUE_CARED_ABOUT - cells[i].cell_value;
				deactivate_corresponding_motor(i);
				cells[i].cell_state = false;
			}
		}
    }
    esp_timer_start_once(update_cells_timer_handle, 50); // us.
}

void pulsate_cells(void *args)
{ 
    esp_timer_stop(update_cells_timer_handle);
	for(uint8_t i = 0; i < CELL_AMOUNT; i++)
	{
		if(cells[i].cell_close == true)
		{
			cells[i].cell_close_counter--;
			if(cells[i].cell_close_counter == 0)
			{
				cells[i].cell_close_state = !(cells[i].cell_close_state);
				cells[i].cell_close_counter = cells[i].cell_close_value;
			}
		}
	}
    esp_timer_start_once(pulsate_cells_timer_handle, 100000); // us.
    esp_timer_start_once(update_cells_timer_handle, 50); // us.

	//Implementera funktion för att undersöka när senaste mottagningen kom.
}


void activate_corresponding_motor(uint8_t i)
{
	switch(i)
	{
		case 0:
			gpio_set_level(CELL_0, ON);
			break;
		case 1:
			gpio_set_level(CELL_1, ON);
			break;
		case 2:
			gpio_set_level(CELL_2, ON);
			break;
		case 3:
			gpio_set_level(CELL_3, ON);
			break;
		case 4:
			gpio_set_level(CELL_4, ON);
			break;
		case 5:
			gpio_set_level(CELL_5, ON);
			break;
		default:
			printf("Something is horribly wrong...");
	}
}

void deactivate_corresponding_motor(uint8_t i)
{
	switch(i)
	{
		case 0:
			gpio_set_level(CELL_0, OFF);
			break;
		case 1:
			gpio_set_level(CELL_1, OFF);
			break;
		case 2:
			gpio_set_level(CELL_2, OFF);
			break;
		case 3:
			gpio_set_level(CELL_3, OFF);
			break;
		case 4:
			gpio_set_level(CELL_4, OFF);
			break;
		case 5:
			gpio_set_level(CELL_5, OFF);
			break;
		default:
			printf("Something is horribly wrong...");
	}
}

void update_which_cells_are_close(void)
{
	uint16_t temp = (uint16_t)(CLOSE_THRESHOLD/2);

	for(uint8_t i = 0; i < CELL_AMOUNT; i++)
	{
		if((cells[i].cell_value < CLOSE_THRESHOLD) && (cells[i].cell_close == false))
		{
			cells[i].cell_close_state = true;
			cells[i].cell_close = true;
			
			if(cells[i].cell_value <= temp)
			{
				cells[i].cell_close_value = 1;
				cells[i].cell_close_counter = 1;
			}
			else
			{
				cells[i].cell_close_counter = 1;
				cells[i].cell_close_value = 1;
			}
		}
		else if((cells[i].cell_value < CLOSE_THRESHOLD) && (cells[i].cell_close == true))
		{
			if(cells[i].cell_value <= temp)
			{
				cells[i].cell_close_value = 1;
			}
			else
			{	
				cells[i].cell_close_value = 1;
			}
		}
		else if(cells[i].cell_value >= CLOSE_THRESHOLD)
		{
			cells[i].cell_close = false;
		}	
	}
}