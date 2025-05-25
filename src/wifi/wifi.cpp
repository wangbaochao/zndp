#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "wifi.h"
#include "config/config.h"
#include "display/display.h"

static int wifiRetryCount = 0;              // WiFi连接重试次数
static unsigned long lastWifiCheckTime = 0; // 记录上次传感器读取时间
int8_t rssi = 0; // 定义并初始化

// 定义最大重试次数
static const int MAX_WIFI_RETRY_COUNT = 100;



// 关闭 ap 有连接时不关闭 ap
void closeWiFiAP()
{
  if ((WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) && WiFi.softAPgetStationNum() == 0)
  {
    Serial.println("AP已开启，关闭AP...");
    WiFi.softAPdisconnect(true);
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("AP已开启，关闭AP...");
    WiFi.softAPdisconnect(true);
  }
}

// 重试连接逻辑
static bool retryConnection(const char *connectionType, void (*connectFunction)(), int &retryCount)
{
  if (retryCount < MAX_WIFI_RETRY_COUNT)
  {
    Serial.printf("%s未连接，尝试重新连接... (重试次数: %d)\n", connectionType, retryCount + 1);
    connectFunction();
    retryCount++;
    return true;
  }
  else
  {
    Serial.printf("%s连接失败，已达到最大重试次数，停止重试...\n", connectionType);
    // 判断 ap 是否开启
    if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
    {
      Serial.println("AP已开启，关闭AP...");
      // WiFi.softAPdisconnect(true);
      closeWiFiAP();
    }
    return false;
  }
}

// 检查WiFi连接状态
void checkWiFiStatus()
{
  if (millis() - lastWifiCheckTime >= 10000)
  {
    // Serial.println("每10秒检查WiFi连接状态");
    lastWifiCheckTime = millis();
    // 检查WiFi连接状态
    if (WiFi.status() != WL_CONNECTED)
    {
      if (!retryConnection("WiFi", updateConnectWifi, wifiRetryCount))
      {
        // WiFi.softAPdisconnect(true);
        closeWiFiAP();
      }
      
    }
    else
    {
      // Serial.println("WiFi disconnected");
      wifiRetryCount = 0;
      // 判断 ap 是否开启
      if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
      {
        Serial.println("AP已开启，关闭AP...");
        // WiFi.softAPdisconnect(true);
        closeWiFiAP();
      }
      // rssi = WiFi.RSSI();
      // Serial.print("当前WiFi信号强度: ");
      // Serial.print(rssi);
      // Serial.println(" dBm");
    }
  }
}




// 创建WiFi热点
void createWiFiAP()
{
  const char *ap_ssid = "GreenhouseConfig";
  const char *ap_password = "123456789";
  if (WiFi.softAP(ap_ssid, ap_password))
  {
    // // Serial.print("Access Point created: ");
    // Serial.println(ap_ssid);
    // // Serial.print("Password: ");
    // Serial.println(ap_password);
  }
  else
  {
    // Serial.println("Failed to create Access Point");
  }
}

// 初始化连接WiFi
void initConnectWiFi()
{
  if (strlen(config.ssid) > 0 && strlen(config.password) > 0)
  {
    WiFi.begin(config.ssid, config.password);
  }
}

// 更新WiFi连接状态
void updateConnectWifi()
{
  if (strlen(config.ssid) > 0 && strlen(config.password) > 0)
  {
    unsigned long startTime = millis();
    const unsigned long timeout = 60000;

    WiFi.begin(config.ssid, config.password);
    // Serial.println(String(" updateConnectWifi 连接WiFi ") + WiFi.status() + ":");
    // // Serial.print("SSID: ");
    // Serial.println(config.ssid);
    // // Serial.print("Password: ");
    // Serial.println(config.password);

    if (WiFi.status() == WL_CONNECTED)
    {
      // Serial.println("连接到 WiFi");
      // // Serial.print("IP address: ");
      // Serial.println(WiFi.localIP());
    }
    else
    {
      // Serial.println("Failed to connect to WiFi");
    }
  }
  else
  {
    // Serial.println("SSID 或 Password 未配置");
  }
}