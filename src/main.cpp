#include <Arduino.h>
#include "wifi/wifi.h"
#include "mqtt/mqtt.h"
#include "sensors/sensors.h"
#include "display/display.h"
#include "config/config.h"
#include "web/server.h"
#include "update/ota.h"
#include "ntp/ntp.h"
#include "heap/heap.h"



// 初始化
void setup()
{
  Serial.begin(115200);

  // 打印版本号到串口
  Serial.println("--------------------------------------------------");
  Serial.println(String("启动系统 - 固件版本: ") + SOFTWARE_VERSION);
  Serial.println("--------------------------------------------------");
  initEEPROM(); // 初始化EEPROM
  initSensors();// 初始化传感器
  initOLED();// 初始化OLED
  initConnectWiFi();// 初始化WiFi
  createWiFiAP(); // 创建WiFi热点
  initWebServer();// 初始化WebServer
  // initMQTT();
  initOTA();// 初始化OTA
  // initNTP();// 初始化NTP
}



// 循环
void loop()
{
  // 检测更新状态
  if (updateState)
  {
    updateState = false;
    handleOTA();
    // 触发 OTA 更新
    isOrNotNeedUpdate();
  }
  else
  {
    // 检测内存占用
    checkMemory();
    // 每10秒读取一次传感器数据
    readSensors();
    // 每2秒更新一次OLED显示
    updateOLED(temperature, soilMoisture, dht11Temperature, dht11SoilMoisture);
    // 检查WiFi连接状态
    checkWiFiStatus();
    // 更新时间
    //updateAndPrintTime();
    // 每24小时重置一次
    resetEveryDay();
  }
  // 延迟1秒，避免主循环过于频繁
  delay(1000);
}