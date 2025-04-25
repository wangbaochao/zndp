#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "wifi.h"
#include "config/config.h"
// #include "logging.h"
// #include "networking.h"
// #include "data_processing.h"
// #include "EEPROM.h"


// 创建WiFi热点
void createWiFiAP() {
  const char *ap_ssid = "GreenhouseConfig";
  const char *ap_password = "123456789";
  if (WiFi.softAP(ap_ssid, ap_password)) {
    Serial.print("Access Point created: ");
    Serial.println(ap_ssid);
    Serial.print("Password: ");
    Serial.println(ap_password);
  } else {
    Serial.println("Failed to create Access Point");
  }
}

// 初始化连接WiFi
void initConnectWiFi() {
  if (strlen(wifiConfig.ssid) > 0 && strlen(wifiConfig.password) > 0) {
    WiFi.begin(wifiConfig.ssid, wifiConfig.password);
  }
}

// 更新WiFi连接状态
void updateConnectWifi() {
  if (strlen(wifiConfig.ssid) > 0 && strlen(wifiConfig.password) > 0) {
    unsigned long startTime = millis();
    const unsigned long timeout = 60000;

    WiFi.begin(wifiConfig.ssid, wifiConfig.password);
    Serial.println(String(" updateConnectWifi 连接WiFi ") + WiFi.status() + ":");
    Serial.print("SSID: ");
    Serial.println(wifiConfig.ssid);
    Serial.print("Password: ");
    Serial.println(wifiConfig.password);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("连接到 WiFi");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      Serial.println("Failed to connect to WiFi");
    }
  } else {
    Serial.println("SSID 或 Password 未配置");
  }
}