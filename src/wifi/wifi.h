#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// struct WiFiConfig {
//   char ssid[32];
//   char password[64];
// };

// extern WiFiConfig wifiConfig;

void initWiFiConfig();
void saveWiFiConfig();
void createWiFiAP();
void initConnectWiFi();
void updateConnectWifi();

#endif