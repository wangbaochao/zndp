#include <EEPROM.h>
#include "config.h"

// 参数存储
Config config;
 
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
  }
  else
  {
    Serial.println("Sensors 没有数据");
  }
}


// 保存配置到EEPROM
void saveConfig()
{
  EEPROM.put(0, config);
  EEPROM.commit();
}
