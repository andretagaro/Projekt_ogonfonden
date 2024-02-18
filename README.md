# Projekt Ögonfonden, haptic based navigation.

## System overview

The sensor module uses the laser sensors VL53l5CX to map the enviroment, after the module has processed the data it is sent via ESP-NOW to the haptic modules. Based on the data from the sensor module the haptic modules generates a representations matrix on a grid of 2x6 vibration motors.

### Laser module

Contains code for the sensor module, the sensor module is built with the laser sensors VL53l5CX and a Adafruit HUZZAH32 ESP-32 developement card. 

### Haptic module

Contains code for the haptic module, the haptic moudle is built with a custom PCB (see the folder Motor driver card for schematic, assembly and gerber files) and a Adafruit HUZZAH32 ESP-32 developement card.