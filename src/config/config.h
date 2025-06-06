#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// 定义软件版本号
#define SOFTWARE_VERSION "v1.0.9"

// 定义软件html版本号
#define SOFTWARE_HTML_VERSION "v1.0.9"

// Configuration
struct Config
{
  float tempThreshold;         // 温度阈值
  float soilMoistureThreshold; // 土壤湿度阈值
  bool manualMode;             // 是否手动模式  手动 1 true 自动 0 false
  bool fanState;               // 风扇状态
  bool pumpState;              // 水泵状态
  bool lightState;             // 灯状态
  char ssid[32];               // WIFI名称
  char password[64];           // WIFI密码
  int tempPin;                 // 用于存储温度传感器的引脚
  int dhtPin;                  // 用于存储DHT11传感器的引脚
  int soilMoisturePin;         // 用于存储土壤湿度传感器的引脚
  int fanPin;                  // 风扇引脚
  int pumpPin;                 // 水泵引脚
  int lightPin;                // 灯引脚
  int contrast;                // LCD 对比度

   // 添加 MQTT 配置
   char mqttServer[64];         // MQTT服务器地址
   char mqttUser[32];           // MQTT用户名
   char mqttPassword[64];       // MQTT密码
   char mqttTopic[64];          // MQTT主题
   char mqttClientId[32];       // MQTT客户端ID  
   int mqttPort; // MQTT端口
};

extern bool updateState; // 更新状态

extern Config config;

void initEEPROM();
void saveConfig();
void initSensors();
void resetEveryDay();

bool isGPIOSupported(int pin);
bool validateAndFixGPIOPin(int &pin, const char *pinName);

#endif