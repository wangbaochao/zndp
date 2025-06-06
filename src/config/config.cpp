#include <EEPROM.h>
#include "config.h"

#define GPIO_MIN 0
#define GPIO_MAX 39

static unsigned long lastResetCheckTime = 0; // 记录上次重启读取时间
const unsigned long ONE_DAY_MS = 86400000UL;

// 参数存储
Config config = {
    0.0,   // 默认温度阈值
    0.0,   // 默认土壤湿度阈值
    false, // 默认自动模式
    false, // 风扇状态
    false, // 水泵状态
    false, // 灯状态
    "",    // 默认WIFI名称
    "",    // 默认WIFI密码
    4,     // 默认温度传感器引脚  
    6,     // 默认存储DHT11传感器的引脚
    2,     // 默认湿度传感器引脚
    7,    // 风扇引脚
    15,    // 水泵引脚
    5,     // 灯引脚
    255,   // 默认contrast

    "mqtt",        // 默认MQTT服务器地址
    "dht",         // 默认MQTT用户名
    "dht",         // 默认MQTT密码
    "dht/data",    // 默认MQTT主题
    "ESP32Client", // 默认MQTT客户端ID
    1883,          // 默认MQTT端口
};

bool updateState = false; // 更新状态

// 检查 GPIO 引脚是否受支持
bool isGPIOSupported(int pin)
{
  if (pin >= 34 && pin <= 39)
    return false; // GPIO34~39 仅支持输入
  return true;
}

// 验证并修复 GPIO 引脚值 int defaultPin,
bool validateAndFixGPIOPin(int &pin, const char *pinName)
{
  if (pin < GPIO_MIN || pin > GPIO_MAX || !isGPIOSupported(pin))
  {
    // pin = defaultPin;
    return false;
    // Serial.printf("修复无效的 %s 值，使用默认值: %d\n", pinName, defaultPin);
  }
  return true;
}

// 定义 EEPROM 存储位置
#define CONFIG_EEPROM_OFFSET sizeof(config)

// 初始化EEPROM
void initEEPROM()
{
  EEPROM.begin(CONFIG_EEPROM_OFFSET);
  // 检查EEPROM中是否已经存在有效的数据
  if (EEPROM.read(0) != 0xFF)
  {
    EEPROM.get(0, config); // 读取已存储的配置
                           // 验证并修复 GPIO 引脚值
    // validateAndFixGPIOPin(config.lightPin, "lightPin") ? 5 : config.lightPin = 5;
    // validateAndFixGPIOPin(config.fanPin,"fanPin") ? 12 : config.lightPin = 12;
    // validateAndFixGPIOPin(config.pumpPin,  "pumpPin") ? 13 : config.lightPin = 13;
  }
  else
  {
    // Serial.println("Sensors 没有数据");
  }
}

// 保存配置到EEPROM
void saveConfig()
{
  EEPROM.put(0, config);
  EEPROM.commit();
}


// 每天重启系统
void resetEveryDay()
{
  // 每隔 24 小时重启一次
  if (millis() - lastResetCheckTime >= ONE_DAY_MS)
  {
    lastResetCheckTime = millis();
    saveConfig();
    ESP.restart();
  }
}
