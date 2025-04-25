#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Configuration
struct Config
{
  float tempThreshold; // 温度阈值
  float soilMoistureThreshold; // 土壤湿度阈值
  bool manualMode; // 是否手动模式 手动 true 自动 false
  char ssid[32]; // WIFI名称
  char password[64]; // WIFI密码
};

extern Config config;

void initEEPROM();
void saveConfig();
 
#endif