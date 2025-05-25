#include <OneWire.h>
#include <DallasTemperature.h>
#include "sensors.h"
#include "config/config.h"
#include <DHT.h>
#include "ntp/ntp.h"
float temperature;
float soilMoisture;
float dht11Temperature;
float dht11SoilMoisture;

static unsigned long lastSensorReadTime = 0; // 记录上次传感器读取时间

// DS18B20温度传感器 使用 config.tempPin 作为引脚 TEMP_PIN  config.tempPin
OneWire oneWire(4);
DallasTemperature sensors(&oneWire);

// DHT11 传感器
DHT dht(config.dhtPin, DHT11);

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

// DHT11 传感器读取温度
float readDHT11Temperature()
{
  dht11Temperature = dht.readTemperature();
  return dht11Temperature;
}

// DHT11 传感器读取湿度
float readDHT11Humidity()
{
  dht11SoilMoisture = dht.readHumidity();
  return dht11SoilMoisture;
}

// 初始化传感器
void initSensors()
{
  sensors.begin();
  dht.begin();
  initPinMode();
}

void initPinMode()
{
  if (!validateAndFixGPIOPin(config.lightPin, "lightPin"))
  {
    config.lightPin = 5;
  }

  if (!validateAndFixGPIOPin(config.fanPin, "fanPin"))
  {
    config.fanPin = 7;
  }

  if (!validateAndFixGPIOPin(config.pumpPin, "pumpPin"))
  {
    config.pumpPin = 15;
  }

  pinMode(config.fanPin, OUTPUT);
  pinMode(config.pumpPin, OUTPUT);
  pinMode(config.lightPin, OUTPUT);

  // 默认关闭
  digitalWrite(config.fanPin, HIGH);
  digitalWrite(config.pumpPin, HIGH);
  digitalWrite(config.lightPin, HIGH);
}

// 读取传感器数据
void readSensors()
{
  // 每10秒读取一次传感器数据
  if (millis() - lastSensorReadTime >= 10000)
  {
    // Serial.println("每10秒读取一次传感器数据");
    lastSensorReadTime = millis();
    readTemperature();
    readSoilMoisture();
    readDHT11Temperature();
    readDHT11Humidity();
    updateDeviceStates();
  }
}

// 更新设备状态
void updateDeviceStates()
{
  if (!config.manualMode)
  {
    if (readTemperature() <= config.tempThreshold)
    {
      digitalWrite(config.fanPin, HIGH); // 风扇关闭
      config.fanState = false;
    }
    else
    {
      digitalWrite(config.fanPin, LOW); // 风扇开启
      config.fanState = true;
    }

    if (readSoilMoisture() <= config.soilMoistureThreshold)
    {
      digitalWrite(config.pumpPin, LOW); // 水泵开启
      config.pumpState = true;
    }
    else
    {
      digitalWrite(config.pumpPin, HIGH); // 水泵关闭
      config.pumpState = false;
    }
  }
  else
  {
    // 风扇
    if (config.fanState)
    {
      digitalWrite(config.fanPin, LOW); // 风扇开启
    }
    else
    {
      digitalWrite(config.fanPin, HIGH); // 风扇关闭
    }

    // 水泵
    if (config.pumpState)
    {
      digitalWrite(config.pumpPin, LOW); // 水泵开启
    }
    else
    {
      digitalWrite(config.pumpPin, HIGH); // 水泵关闭
    }
  }
  // 灯
  if (config.lightState)
  {
    digitalWrite(config.lightPin, LOW); // 灯开启
  }
  else
  {
    digitalWrite(config.lightPin, HIGH); // 灯关闭
  }
}