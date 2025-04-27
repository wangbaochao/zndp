#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// 定义GPIO引脚
// #define TEMP_PIN 4
// #define SOIL_MOISTURE_PIN 2
// #define SOIL_MOISTURE_PIN A0
// #define FAN_PIN 12
// #define PUMP_PIN 13
// #define LIGHT_PIN 5
// 实时数据（仅声明）
// extern float temperature;
// extern float soilMoisture;
// extern bool fanState;
// extern bool pumpState;
// extern bool lightState;

extern OneWire oneWire;
extern DallasTemperature sensors;

extern float temperature;
extern float soilMoisture;

float readTemperature();
float readSoilMoisture();

void updateDeviceStates();
#endif