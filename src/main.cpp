#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "config.h"

// TODO: configure settings on web page
// TODO: save settings
// TODO: API

// TODO: motor driver connection
// TODO: toggle switch for motor
// TODO: servo control

AsyncWebServer server(HTTP_PORT);

void initSerial() {
    Serial.begin(9600);
    Serial.println();

    for(uint8_t t = 3; t > 0; t--){
        Serial.printf("WAIT %d...\n", t);
        Serial.flush();
        delay(500);
    }
    Serial.println("\n* * * START * * *");
}

void initWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_WIFI_SSID, _WIFI_PASS);
    Serial.printf("Connecting to WiFi [%s]", _WIFI_SSID);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.printf(".");
    }
    Serial.println("IP => " + WiFi.localIP().toString());
}

void initServer() {
    server.onNotFound([](AsyncWebServerRequest* req) {
        req->send_P(404, "text/plain; charset=utf-8", "Page not found.");
    });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(LittleFS, "/styles.css", "text/css");
    });
    server.begin();
    Serial.printf("Server listening on port %d...\n", HTTP_PORT);
}

void setup() {
    initSerial();
    initWifi();

    if (!LittleFS.begin()) {
        Serial.println("Error occurred while mounting LittleFS.");
        while (1) {}
    }
    initServer();

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    // nop
}
