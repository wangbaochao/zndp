// ntp.cpp
#include <Arduino.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "ntp.h"

// 创建UDP实例
WiFiUDP ntpUDP;
// 创建NTPClient对象，并指定时区偏移量
NTPClient timeClient(ntpUDP, "ntp.aliyun.com", 28800, 60000); // 使用阿里云的NTP服务器

static unsigned long lastTimeUpdate = 0; // 记录上次时间更新时间

// 初始化NTP
void initNTP() {
  // 初始化NTPClient
  timeClient.begin();
}

// 更新时间
void updateAndPrintTime() {
  unsigned long currentTime = millis();
  // 获取并打印当前时间
  if (currentTime - lastTimeUpdate >= 3600000) {  // 1小时 = 3600000毫秒
    timeClient.update();
    Serial.println("当前时间: " + timeClient.getFormattedTime());
    lastTimeUpdate = currentTime;
  }
}
 
