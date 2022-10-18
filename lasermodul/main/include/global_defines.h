#ifndef GLOBAL_DEFINES_H_
#define GLOBAL_DEFINES_H_

/* LOGIC DEFINES */
#define ON 1
#define OFF 0
#define FOREVER 1
/* --------------- */


/* I2C DEFINES */
#define SDA_GPIO 23
#define SCL_GPIO 22
#define I2C_FREQ 1000000
/* --------------- */

/* battery pin */
#define BATTERY_INDICATOR_LED 4

/*VL53L5CX DEFINES */
#define LP_RIGHT_PIN 27
#define INT_RIGHT_PIN 26
#define RST_RIGHT_PIN 15
#define PWR_ENABLE_RIGHT_PIN 13

#define LP_LEFT_PIN 33
#define INT_LEFT_PIN 25
#define RST_LEFT_PIN 32
#define PWR_ENABLE_LEFT_PIN 12

#define SENSOR_CUSTOM_ADDRESS 0x22
/* --------------- */

#endif