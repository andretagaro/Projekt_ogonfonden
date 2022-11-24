# Projekt Ögonfonden, haptikbaserad navigering.

## Systemöversikt
Sensormodulen använder sig av lasersensorerna VL53l5CX för att kartlägga omgivningen, efter att datan behandlats skickas den till respektive haptikmodul som baserat på denna data genererar PWM-signaler som driver vibrationsmotorer. Dytun på dessa PWM-signaler ökar med minskade avstånd.

### lasermodul
Innehåller mjukvara för sensormodulen, bygger på FREERTOS och använder ESP32-wroom-modulen, men justeras för C3-modulen utan större problem.

### esp_now_boiler
Innehåller kod för haptikmodulen, kör FREERTOS i bakgrunden men använder inte några task-funktioner. Mjukvaran körs i nuläget på ESP32-wroom men kan jsuteras för C3-moudlen utan större problem.

### esp_now_boiler_2
Innehåller testkod för ESP-NOW protokollet.
