#include <Arduino.h>

// 定义内存警戒阈值（单位：字节）
static constexpr size_t MEMORY_THRESHOLD = 500;
static unsigned long lastMemoryUpdateTime = 0; // 记录上次 checkMemory 更新时间

// 检测内存占用
void checkMemory()
{
  // 每一小时检查一次
  if (millis() - lastMemoryUpdateTime >= 3600000)
  {
    // Serial.println("每一小时检查一次内存占用");
    lastMemoryUpdateTime = millis();
    if (ESP.getFreeHeap() < MEMORY_THRESHOLD)
    {
      Serial.println("内存严重不足，重启系统...");
      ESP.restart();
    }
  }
}
