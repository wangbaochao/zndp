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

  // 打印版本号到串口
  Serial.println("--------------------------------------------------");
  Serial.println(String("启动系统 - 软件版本: ") + SOFTWARE_VERSION);
  Serial.println("--------------------------------------------------");

  initEEPROM();
  sensors.begin();
  pinMode(config.fanPin, OUTPUT);
  pinMode(config.pumpPin, OUTPUT);
  pinMode(config.lightPin, OUTPUT);
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
  static unsigned long lastSensorReadTime = 0; // 记录上次传感器读取时间
  static unsigned long lastOLEDUpdateTime = 0; // 记录上次OLED更新时间
  // 检测内存占用
  size_t freeHeap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
  if (freeHeap < 1000) { // 如果可用内存小于1000字节
    Serial.println("内存不足，尝试释放资源...");
    // 释放不再使用的资源
    // 例如：关闭WiFi连接、停止MQTT客户端等
    WiFi.disconnect(true);
    // mqttClient.disconnect();

    // 如果内存仍然不足，可以考虑重启系统
    if (heap_caps_get_free_size(MALLOC_CAP_8BIT) < 500) {
      Serial.println("内存严重不足，重启系统...");
      ESP.restart();
    }
  }
  // 每10秒读取一次传感器数据
  if (millis() - lastSensorReadTime >= 10000)
  {
    Serial.println("每10秒读取一次传感器数据");
    lastSensorReadTime = millis();
    float temperature = readTemperature();
    float soilMoisture = readSoilMoisture();

    // 更新设备状态
    updateDeviceStates();
  }

  // 每2秒更新一次OLED显示
  if (millis() - lastOLEDUpdateTime >= 2000)
  {
    Serial.println("每2秒更新一次OLED显示");
    lastOLEDUpdateTime = millis();
    updateOLED(temperature, soilMoisture);
  }

  // 检查WiFi连接状态
  if (WiFi.status() != WL_CONNECTED)
  {
    updateConnectWifi();
  } else
  {
       // if (!mqttClient.connected()) {
    //   reconnectMQTT();
    // }
    // mqttClient.loop();

    // if (mqttClient.connected()) {
    //   mqttClient.publish("dht/temperature", String(temperature).c_str());
    //   mqttClient.publish("dht/soil_moisture", String(soilMoisture).c_str());
    // }
    // wifi 已连接关闭 ap 热点
    WiFi.softAPdisconnect(true);// 关闭AP
  }

  // 延迟1秒，避免主循环过于频繁
  delay(1000);


  // 启用低功耗模式
  // esp_sleep_enable_timer_wakeup(1000 * 1000); // 1秒
  // esp_light_sleep_start();


}