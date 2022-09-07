#include <stdio.h>
#include "esp_now_functions.h"
#include "esp_timer.h"

#define CELL_0 21
#define OFF 0
#define ON 1
#define CELL_AMOUNT 6
#define MAX_VALUE_CARED_ABOUT 400
#define CLOSE_THRESHOLD 200


uint8_t mac_adress_sender[MAC_SIZE] = {0x84, 0xf7,0x03,0x0b,0xd1, 0x2c};
uint8_t mac_adress_left[MAC_SIZE] = {0x84, 0xf7, 0x03, 0x0b, 0xdd, 0x5c}; 
uint8_t mac_adress_right[MAC_SIZE] = {0xfc, 0xf5, 0xc4, 0x09, 0x61, 0x90}; // this is my mac (ESP32)

void init_gpio(void);
void update_cells(void* args);
void activate_corresponding_motor(uint8_t i);
void deactivate_corresponding_motor(uint8_t i);
void update_which_cells_are_close(void);
void pulsate_cells(void *args);

struct cell
{
	bool cell_state;
	bool cell_close;
	bool cell_close_state;

	uint16_t cell_value;
	uint16_t cell_on_counter;
	uint16_t cell_off_counter;

	uint16_t cell_close_counter;
	uint16_t cell_close_value;
};
typedef struct cell cell;

cell cells[CELL_AMOUNT] = {0};
esp_timer_handle_t update_cells_timer_handle;
esp_timer_handle_t pulsate_cells_timer_handle;

void app_main(void)
{
    init_gpio();

    //Timer for update cells
    const esp_timer_create_args_t update_cells_timer = {
        .callback = update_cells,
        .name = "update cells"
    };
    esp_timer_create(&update_cells_timer, &update_cells_timer_handle);
    esp_timer_start_once(update_cells_timer_handle, 50); // us.

    const esp_timer_create_args_t pulsate_cells_timer = {
    .callback = pulsate_cells,
    .name = "pulsate cells"
    };
    esp_timer_create(&pulsate_cells_timer, &pulsate_cells_timer_handle);
    esp_timer_start_once(pulsate_cells_timer_handle, 100000); // us.


    print_mac_adress_as_hex_string();
    activate_esp_now();
    esp_now_add_peer_wrapper(mac_adress_sender);
    esp_now_add_peer_wrapper(mac_adress_left);

    cells[0].cell_value = 400;
    cells[0].cell_on_counter = MAX_VALUE_CARED_ABOUT - cells[0].cell_value;
    cells[0].cell_off_counter = cells[0].cell_value;
    for(;;)
    {
        cells[0].cell_value--;
        vTaskDelay(30/portTICK_PERIOD_MS);
        if(cells[0].cell_value == 0)
        {
            cells[0].cell_value = 400;
        }
        update_which_cells_are_close();
    }
}

void init_gpio()
{
    gpio_pad_select_gpio(CELL_0);
    ESP_ERROR_CHECK(gpio_set_direction(CELL_0, GPIO_MODE_OUTPUT));
    gpio_set_level(CELL_0, OFF);
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
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
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
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
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