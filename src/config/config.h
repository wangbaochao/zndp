#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Configuration
struct Config {
  float tempThreshold;
  float soilMoistureThreshold;
  bool manualMode;
};

extern Config config;

void initEEPROM();
void saveConfig();

// WiFi Configuration
struct WiFiConfig {
  char ssid[32];
  char password[64];
};

extern WiFiConfig wifiConfig;

void initWiFiConfig();
void saveWiFiConfig();

#endif