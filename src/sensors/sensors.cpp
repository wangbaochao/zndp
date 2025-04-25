#include <OneWire.h>
#include <DallasTemperature.h>
#include "sensors.h"
#include "config/config.h"

// 变量定义并初始化
// 温度
float temperature = 0.0;
// 土壤湿度
float soilMoisture = 0.0;
// 风扇状态
bool fanState = false;
// 水泵状态
bool pumpState = false;
// 灯状态
bool lightState = false;
// DS18B20温度传感器
OneWire oneWire(TEMP_PIN);
DallasTemperature sensors(&oneWire);
// 温度传感器读取
float readTemperature() {
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  return temperature;
}
// 土壤湿度传感器读取
float readSoilMoisture() {
  soilMoisture = analogRead(SOIL_MOISTURE_PIN);
  soilMoisture = map(soilMoisture, 0, 4095, 100, 0);
  return soilMoisture;
}
// 更新设备状态
void updateDeviceStates() {
  if (!config.manualMode) {
    if (temperature <= config.tempThreshold) {
      digitalWrite(FAN_PIN, LOW);
      fanState = false;
    } else {
      digitalWrite(FAN_PIN, HIGH);
      fanState = true;
    }

    if (soilMoisture <= config.soilMoistureThreshold) {
      digitalWrite(PUMP_PIN, HIGH);
      pumpState = true;
    } else {
      digitalWrite(PUMP_PIN, LOW);
      pumpState = false;
    }
  }
}