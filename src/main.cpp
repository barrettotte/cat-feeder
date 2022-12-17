#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Servo.h>

#include "config.h"

Servo servo;
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

void setMotor(bool enabled, int speed) {
    digitalWrite(MOTOR_IN1_PIN, LOW);
    if (enabled) {
        digitalWrite(MOTOR_IN2_PIN, HIGH);
        analogWrite(MOTOR_EN_PIN, speed);
    } else {
        digitalWrite(MOTOR_IN2_PIN, LOW);
    }
}

void handleGetConfig(AsyncWebServerRequest* req) {
    // TODO:
    req->send_P(200, "application/json; charset=utf-8", "get config");
}

void handleSetConfig(AsyncWebServerRequest* req) {
    // TODO:
    req->send_P(200, "application/json; charset=utf-8", "set config");
}

void handleSetServo(AsyncWebServerRequest* req) {
    int pos = req->arg("pos").toInt();
    
    if (pos > 180) {
        pos = 180;
    } else if (pos < 0) {
        pos = 0;
    }
    Serial.printf("Servo: pos=%d\n", pos);

    servo.write(pos);
    delay(50);
    req->send(200);
}

void handleSetMotor(AsyncWebServerRequest* req) {
    int en = req->arg("en").toInt();
    int speed = req->arg("speed").toInt();
    
    if (speed > 255) {
        speed = 255;
    } else if (speed < 0) {
        speed = 0;
    }
    Serial.printf("Motor: en=%d, speed=%d\n", en, speed);

    setMotor(en, speed);
    delay(50);
    req->send(200);
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
    server.on("/servo", HTTP_POST, handleSetServo);
    server.on("/motor", HTTP_POST, handleSetMotor);
    server.on("/config", HTTP_GET, handleGetConfig);
    server.on("/config", HTTP_POST, handleSetConfig);

    server.begin();
    Serial.printf("Server listening on port %d...\n", HTTP_PORT);
}

void initServo() {
    if (servo.attach(SERVO_PIN) == 0) {
        Serial.println("Error occurred while attaching to servo.");
        while (1) {}
    }
    servo.write(0);
}

void initMotor() {
    pinMode(MOTOR_EN_PIN, OUTPUT);
    pinMode(MOTOR_IN1_PIN, OUTPUT);
    pinMode(MOTOR_IN2_PIN, OUTPUT);
    setMotor(false, 0);
}

void setup() {
    initSerial();
    initWifi();

    if (!LittleFS.begin()) {
        Serial.println("Error occurred while mounting LittleFS.");
        while (1) {}
    }
    initServer();
    initServo();
    initMotor();

    pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    // nop
}
