#ifndef _GLOBAL_DEFINES_H
#define _GLOBAL_DEFINES_H

#define CELL_0 13
#define CELL_1 27
#define CELL_2 33
#define CELL_3 15
#define CELL_4 32
#define CELL_5 13

#define OFF 0
#define ON 1
#define CELL_AMOUNT 6
#define MAX_VALUE_CARED_ABOUT 200
#define CLOSE_THRESHOLD 40

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


#endif