#include "cell_functions.h"

extern esp_timer_handle_t update_cells_timer_handle;
extern esp_timer_handle_t pulsate_cells_timer_handle;

cell cells[CELL_AMOUNT] = {0};

/* Actives correspoding motor, placement of motors as following:
** 456 upper row 
** 123 lower row.
** @param i				The motor to turn on.
*/
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
			ESP_LOGE("activate_corresponding_motor", "Something is horribly wrong...");
	}
}

/* Deactives correspoding motor, placement of motors as following:
** 456 upper row 
** 123 lower row.
** @param i				The motor to turn off.
*/
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
			ESP_LOGE("deactivate_corresponding_motor", "Something is horribly wrong...");
	}
}

/* Checks if any range data is lower than the threshold,
** and updates the cells close state accordingly.
*/
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

/* Switches all cells, i.e motors off.
*/
void all_cells_off(void)
{
	gpio_set_level(CELL_0, OFF);
	gpio_set_level(CELL_1, OFF);
	gpio_set_level(CELL_2, OFF);
	gpio_set_level(CELL_3, OFF);
	gpio_set_level(CELL_4, OFF);
	gpio_set_level(CELL_5, OFF);
}

/*****************************************************************************************************************************************/
/******************************************TIMER HANDLERS BENETH THIS LINE****************************************************************/
/*****************************************************************************************************************************************/

/* Updates the cells according to the range data from the laser module (sender peer),
** this function is responsible for switching the cells on and off to create a PWM signal.
** the integer cell_off_counter sets the OFF time, and cell_on_counter sets the ON time.
*/
void update_cells(void* args)
{	
	for(uint8_t i = 0; i < CELL_AMOUNT; i++)
	{
		if(cells[i].cell_state == OFF)
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
					if(cells[i].cell_value < MAX_VALUE_CARED_ABOUT)
					{
						activate_corresponding_motor(i);
						cells[i].cell_state = true;
					}
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

/* If an object is close and a cell should be pulsating this function sets if
** the cells should be pulse on of pulse off. This function only sets the state,
** the function 'update_cells' is doing the actual pulsing.
*/
void pulsate_cells(void *args)
{ 
    //esp_timer_stop(update_cells_timer_handle);
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
    esp_timer_start_once(pulsate_cells_timer_handle, 100000);
    //esp_timer_start_once(update_cells_timer_handle, 50);
}
