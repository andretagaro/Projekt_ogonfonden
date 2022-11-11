#ifndef _GLOBAL_DEFINES_H
#define _GLOBAL_DEFINES_H

#define CELL_0 13
#define CELL_1 12
#define CELL_2 27
#define CELL_3 33
#define CELL_4 15
#define CELL_5 32

#define OFF 0
#define ON 1
#define CELL_AMOUNT 6
#define MAX_VALUE_CARED_ABOUT 225
#define CLOSE_THRESHOLD 40
#define RECEPTION_THRESHOLD 500000 // Time in us.
#define BATT_LIMIT 3200 // Threshold in mV.
#define MIN_PWM 2 //This is not a threshold integer not an percentage.
#define RECEPTION_TIMER_UPDATE_TIME 100000

struct cell
{ 
	bool cell_state; // Is the cell on or off?
	bool cell_close; // is the cell below the close threshold
	bool cell_close_state;

	uint16_t cell_value; // Range value
	uint16_t cell_on_counter; // Counter to determine when to turn the cell (i.e motor) off
	uint16_t cell_off_counter; // Counter to determine then to turn the cell (i.e motor) on

	uint16_t cell_close_counter;
	uint16_t cell_close_value;
};
typedef struct cell cell;

#endif