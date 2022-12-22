#include <Arduino.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <memory.h>
#include <Servo.h>

#include "config.h"

#define ERROR_HALT(s) Serial.println(s); while(1) {}

struct feedConfig {
    uint8_t duration;
    uint8_t speed;
};

struct feederState {
    bool trigger;
    uint8_t servoAngle;
    uint8_t motorDuration;
    uint8_t motorSpeed;
    bool motorEn;
};

struct interruptTimer {
    bool trigger;
    unsigned long currentMs;
    unsigned long prevMs;
};

/*** globals ***/

const int configJsonCapacity = JSON_OBJECT_SIZE(2); // 1 object with 2 members

Servo servo;
AsyncWebServer server(HTTP_PORT);
auto feeder = std::unique_ptr<feederState>(new feederState);
auto config = std::unique_ptr<feedConfig>(new feedConfig);
auto buttonTimer = std::unique_ptr<interruptTimer>(new interruptTimer);

/*** utils ***/

void setServoAngle(uint8_t angle) {
    servo.write(angle);
    feeder->servoAngle = angle;
    Serial.printf("Servo: angle=%d\n", angle);
}

void moveServo(bool open) {
    setServoAngle(open ? SERVO_OPEN : SERVO_CLOSE);
    delay(50);
}

void setMotor(bool enabled, uint8_t speed) {
    speed = constrain(speed, 0, MAX_SPEED);
    digitalWrite(MOTOR_IN1_PIN, LOW);

    if (enabled) {
        digitalWrite(MOTOR_IN2_PIN, HIGH);
        analogWrite(MOTOR_EN_PIN, speed);
    } else {
        digitalWrite(MOTOR_IN2_PIN, LOW);
    }
    feeder->motorEn = enabled;
    feeder->motorSpeed = speed;
    Serial.printf("Motor: en=%d,speed=%d\n", enabled, speed);
    delay(50);
}

uint8_t loadConfig() {
    int result = 0;
    char buffer[BUFFER_SIZE];

    File f = LittleFS.open(CONFIG_PATH, "r");
    f.readBytes(buffer, BUFFER_SIZE);

    StaticJsonDocument<configJsonCapacity> data;
    DeserializationError err = deserializeJson(data, buffer);

    if (err) {
        Serial.printf("JSON deserialize failed with code");
        Serial.println(err.f_str());
        result = -1;
    } else {
        config->duration = constrain(data["duration"], 0, MAX_DURATION);
        config->speed = constrain(data["speed"], 0, MAX_SPEED);
    }
    f.close();
    delay(50);
    return result;
}

void saveConfig() {
    File f = LittleFS.open(CONFIG_PATH, "w");
    f.printf("{\"duration\":%d,\"speed\":%d}", config->duration, config->speed);
    f.close();
    delay(50);
}

void resetState() {
    setMotor(false, DEFAULT_SPEED);
    moveServo(false);
    feeder->trigger = false;
}

void feed(uint8_t duration, uint8_t speed) {
    // blocks with delays...consider refactor to state machine?
    Serial.printf("Feeding with duration=%d,speed=%d\n", duration, speed);

    // open chute
    moveServo(true);
    delay(1000);

    // run motor for duration
    setMotor(true, speed);
    delay(duration * 1000); // secs to ms
    setMotor(false, 0);
    delay(500);

    // close chute
    moveServo(false);
    delay(1000);
}

/*** handlers ***/

void handleManualServo(AsyncWebServerRequest* req) {
    if (feeder->trigger) {
        req->send(503);
    } else {
        setServoAngle(req->arg("a").toInt());
        req->send(200);
    }
}

void handleManualMotor(AsyncWebServerRequest* req) {
    if (feeder->trigger) {
        req->send(503);
    } else {
        setMotor(req->arg("en").toInt(), req->arg("s").toInt());
        req->send(200);
    }
}

void handleGetConfig(AsyncWebServerRequest* req) {
    char buffer[BUFFER_SIZE];
    sprintf(buffer, "{\"duration\":%d,\"speed\":%d}", config->duration, config->speed);
    req->send_P(200, "application/json;charset=utf-8", buffer);
}

void handleSetConfig(AsyncWebServerRequest* req, JsonVariant& json) {
    StaticJsonDocument<configJsonCapacity> data;
    data = json.as<JsonObject>();
    config->duration = constrain(data["duration"], 0, MAX_DURATION);
    config->speed = constrain(data["speed"], 0, MAX_SPEED);

    Serial.printf("Set config: duration=%d,speed=%d\n", config->duration, config->speed);
    req->send(200);
}

void handleReset(AsyncWebServerRequest* req) {
    resetState();
    config->duration = DEFAULT_DURATION;
    config->speed = DEFAULT_SPEED;
    saveConfig();

    Serial.println("Reset config and state");
    req->send(200);
}

void handleFeed(AsyncWebServerRequest* req) {
    uint8_t duration = config->duration;
    uint8_t speed = config->speed;

    // if passed, override config temporarily
    if (req->hasParam("d")) {
        duration = constrain(req->arg("d").toInt(), 0, MAX_DURATION);
    }
    if (req->hasParam("s")) {
        speed = constrain(req->arg("s").toInt(), 0, MAX_SPEED);
    }
    feeder->motorDuration = duration;
    feeder->motorSpeed = speed;
    feeder->trigger = true;

    req->send_P(200, "text/plain;charset=utf-8", "Fed the cat!");
}

/*** initialization ***/

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

void initConfig() {
    config->duration = DEFAULT_DURATION;
    config->speed = DEFAULT_SPEED;
    if (loadConfig()) {
        ERROR_HALT("Error occurred while loading initial config.");
    }
}

void initServo() {
    if (servo.attach(SERVO_PIN) == 0) {
        ERROR_HALT("Error occurred while attaching to servo.");
    }
    moveServo(false);
}

void initMotor() {
    pinMode(MOTOR_EN_PIN, OUTPUT);
    pinMode(MOTOR_IN1_PIN, OUTPUT);
    pinMode(MOTOR_IN2_PIN, OUTPUT);
    setMotor(false, DEFAULT_SPEED);
}

void initFs() {
    if (!LittleFS.begin()) {
        ERROR_HALT("Error occurred while mounting LittleFS.");
    }
    delay(50);
}

void initServer() {
    server.onNotFound([](AsyncWebServerRequest* req) {
        req->send_P(404, "text/plain;charset=utf-8", "Page not found.");
    });
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(LittleFS, "/index.html", "text/html");
    });
    server.on("/styles.css", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(LittleFS, "/styles.css", "text/css");
    });
    server.on("/cat-feeder.js", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(LittleFS, "/cat-feeder.js", "application/javascript");
    });
    server.on("/config.json", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(LittleFS, "/config.json", "application/json");
    });

    server.on("/servo", HTTP_POST, handleManualServo);
    server.on("/motor", HTTP_POST, handleManualMotor);
    server.on("/config", HTTP_GET, handleGetConfig);
    server.addHandler(new AsyncCallbackJsonWebHandler("/config", handleSetConfig));
    server.on("/reset", HTTP_POST, handleReset);
    server.on("/feed", HTTP_POST, handleFeed);

    server.begin();
    Serial.printf("Server listening on port %d...\n", HTTP_PORT);
}

/*** interrupts ***/

void IRAM_ATTR handleButtonInterrupt() {
    buttonTimer->currentMs = millis();

    if (buttonTimer->currentMs - buttonTimer->prevMs > DEBOUNCE_MS) {
        buttonTimer->trigger = true;
        buttonTimer->prevMs = buttonTimer->prevMs;
        Serial.println("Button pressed.");
    }
}

/*** main ***/

void setup() {
    initSerial();
    initWifi();
    initFs();

    initConfig();
    feeder->trigger = false;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonInterrupt, FALLING);

    initServo();
    initMotor();
    initServer();

    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.println("Setup Done.");
}

void loop() {
    if (buttonTimer->trigger && !feeder->trigger) {
        feeder->motorDuration = config->duration;
        feeder->motorSpeed = config->speed;
        feeder->trigger = true;
        buttonTimer->trigger = false;
    }
    if (feeder->trigger && !feeder->motorEn) {
        feed(feeder->motorDuration, feeder->motorSpeed);
        feeder->trigger = false;
    }
}
