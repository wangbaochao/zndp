#include <EEPROM.h>
#include "config.h"

// 参数存储
Config config;

// 初始化EEPROM
void initEEPROM() {
  EEPROM.begin(sizeof(config));
  EEPROM.get(0, config);
}

// 保存配置到EEPROM
void saveConfig() {
  EEPROM.put(0, config);
  EEPROM.commit();
}

// WiFi配置结构体
WiFiConfig wifiConfig;

// 定义 EEPROM 存储位置
#define CONFIG_EEPROM_OFFSET 0
#define WIFI_CONFIG_EEPROM_OFFSET sizeof(Config)

// 初始化EEPROM中的WiFi配置
void initWiFiConfig() {
  EEPROM.begin(WIFI_CONFIG_EEPROM_OFFSET + sizeof(wifiConfig));
  EEPROM.get(WIFI_CONFIG_EEPROM_OFFSET, wifiConfig); // 存储在EEPROM的不同位置
}

// 保存WiFi配置到EEPROM
void saveWiFiConfig() {
  EEPROM.put(WIFI_CONFIG_EEPROM_OFFSET, wifiConfig);
  EEPROM.commit();
}



// #include <EEPROM.h>
// #include "config.h"

// // 参数存储
// struct Config {
//   float tempThreshold;
//   float soilMoistureThreshold;
//   bool manualMode;
// } config;

// // 初始化EEPROM
// void initEEPROM() {
//   EEPROM.begin(sizeof(config));
//   EEPROM.get(0, config);
// }

// // 保存配置到EEPROM
// void saveConfig() {
//   EEPROM.put(0, config);
//   EEPROM.commit();
// }

// // WiFi配置结构体
// struct WiFiConfig {
//   char ssid[32];
//   char password[64];
// } wifiConfig;

// // // 初始化WiFi配置
// // void initWiFiConfig() {
// //   EEPROM.begin(sizeof(wifiConfig));
// //   EEPROM.get(0, wifiConfig);
// // }

// // // 保存WiFi配置到EEPROM
// // void saveWiFiConfig() {
// //   EEPROM.put(0, wifiConfig);
// //   EEPROM.commit();
// // }

// // 定义 EEPROM 存储位置
// #define CONFIG_EEPROM_OFFSET 0
// #define WIFI_CONFIG_EEPROM_OFFSET sizeof(Config)

// // 初始化EEPROM中的WiFi配置
// void initWiFiConfig()
// {
//   EEPROM.begin(WIFI_CONFIG_EEPROM_OFFSET + sizeof(wifiConfig));
//   EEPROM.get(WIFI_CONFIG_EEPROM_OFFSET, wifiConfig); // 存储在EEPROM的不同位置
// }

// // 保存WiFi配置到EEPROM
// void saveWiFiConfig()
// {
//   EEPROM.put(WIFI_CONFIG_EEPROM_OFFSET, wifiConfig);
//   EEPROM.commit();
// }