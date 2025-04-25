#include <Arduino.h>
#include "wifi/wifi.h"
// #include "mqtt/mqtt.h"
#include "sensors/sensors.h"
#include "display/display.h"
#include "config/config.h"
#include "web/server.h"

// 初始化
void setup()
{
  Serial.begin(115200);
  initEEPROM();
  sensors.begin();
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  initOLED();
  createWiFiAP();
  initConnectWiFi();
  initWebServer();
  // mqttClient.setServer(mqtt_server, 1883);
  // mqttClient.setCallback(mqttCallback);
}

// 循环
void loop()
{
  float temperature = readTemperature();
  float soilMoisture = readSoilMoisture();
   
  if (WiFi.status() != WL_CONNECTED)
  {
    updateConnectWifi();
    Serial.println(String("连接 loop WiFi ") + WiFi.status() + ":");
  }
  else 
  {
    Serial.println(String("已连接 loop WiFi ") + WiFi.status() + ":");
    Serial.print("温度: ");
    Serial.println(String(temperature).c_str());
    Serial.print("湿度: ");
    Serial.println(String(soilMoisture).c_str());
    Serial.print("loop SSID: ");
    Serial.println(config.ssid);
    Serial.print("loop Password: ");
    Serial.println(config.password);
    Serial.print("loop manualMode: ");
    Serial.println(config.manualMode);
    Serial.print("loop tempThreshold: ");
    Serial.println(config.tempThreshold);
    Serial.print("loop soilMoistureThreshold: ");
    Serial.println(config.soilMoistureThreshold);


    // if (!mqttClient.connected()) {
    //   reconnectMQTT();
    // }
    // mqttClient.loop();

    // if (mqttClient.connected()) {
    //   mqttClient.publish("dht/temperature", String(temperature).c_str());
    //   mqttClient.publish("dht/soil_moisture", String(soilMoisture).c_str());
    // }
  }



  updateDeviceStates();
  updateOLED(temperature, soilMoisture);

  delay(5000);
}