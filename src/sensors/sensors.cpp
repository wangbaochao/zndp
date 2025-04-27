#include <OneWire.h>
#include <DallasTemperature.h>
#include "sensors.h"
#include "config/config.h"

float temperature;
float soilMoisture;

// DS18B20温度传感器 使用 config.tempPin 作为引脚 TEMP_PIN
OneWire oneWire(config.tempPin);
DallasTemperature sensors(&oneWire);
// 温度传感器读取
float readTemperature()
{
  sensors.requestTemperatures();
  temperature = sensors.getTempCByIndex(0);
  return temperature;
}
// 土壤湿度传感器读取
float readSoilMoisture()
{
  // 使用 config.soilMoisturePin 作为引脚 SOIL_MOISTURE_PIN
  int pin = config.soilMoisturePin;
  soilMoisture = analogRead(pin);
  soilMoisture = map(soilMoisture, 0, 4095, 100, 0);
  return soilMoisture;
}
// 更新设备状态
void updateDeviceStates()
{
  if (!config.manualMode)
  {
    if (readTemperature() <= config.tempThreshold)
    {
      digitalWrite(config.fanPin, LOW); // 风扇关闭
      config.fanState = false;
    }
    else
    {
      digitalWrite(config.fanPin, HIGH); // 风扇开启
      config.fanState = true;
    }

    if (readSoilMoisture() <= config.soilMoistureThreshold)
    {
      digitalWrite(config.pumpPin, HIGH); // 水泵开启
      config.pumpState = true;
    }
    else
    {
      digitalWrite(config.pumpPin, LOW); // 水泵关闭
      config.pumpState = false;
    }
  }
  // 灯
  if (config.lightState)
  {
    Serial.print(" 灯开启: ");
    Serial.println(config.lightPin);  
    digitalWrite(config.lightPin, HIGH); // 灯开启
    // config.lightState = true;
  }
  else
  {
    Serial.print(" 灯关闭: ");
    Serial.println(config.lightPin);  
    digitalWrite(config.lightPin, LOW); // 灯关闭
    // config.lightState = false;
  }
}