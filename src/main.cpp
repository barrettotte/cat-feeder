#include "Arduino.h"
#include "config.h"

// TODO: init serial
// TODO: init WiFi
// TODO: init file system
// TODO: activate built-in LED when startup finished

// TODO: serve web page
// TODO: configure settings on web page
// TODO: save settings
// TODO: API

// TODO: motor driver connection
// TODO: toggle switch for motor
// TODO: servo control

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
