#include <ESPAsyncWebServer.h>
#include "wifi/wifi.h"
#include "config/config.h"
#include "sensors/sensors.h"
#include "web/server.h"

 // Web服务器 定义为全局变量
AsyncWebServer server(80);

// Web服务器处理
void handleRoot(AsyncWebServerRequest *request)
{
  // 静态 HTML 模板
  const char *htmlTemplate =
      "<!DOCTYPE html>"
      "<html><head><title>Greenhouse Control</title></head><body>"
      "<h1>Greenhouse Environment Control</h1>"
      "<p>Temperature: %s °C</p>"
      "<p>Soil Moisture: %s %%</p>"
      "<p>Fan: %s</p>"
      "<p>Pump: %s</p>"
      "<p>Light: %s</p>"
      "<p>Mode: %s</p>"
      "<p>WiFi Status: %s</p>"
      "<form action='/set' method='POST'>"
      "Temperature Threshold: <input type='number' name='tempThreshold' value='%s'><br>"
      "Soil Moisture Threshold: <input type='number' name='soilMoistureThreshold' value='%s'><br>"
      "<input type='submit' value='Save'>"
      "</form>"
      "<form action='/toggleMode' method='POST'>"
      "<input type='submit' value='Toggle Mode'>"
      "</form>"
      "<a href='/wifiConfig'>Configure WiFi</a>"
      "</body></html>";

  // 动态内容缓冲区
  char buffer[1024]; // 根据实际需求调整缓冲区大小
  snprintf(buffer, sizeof(buffer),
           htmlTemplate,
           String(temperature).c_str(),
           String(soilMoisture).c_str(),
           fanState ? "ON" : "OFF",
           pumpState ? "ON" : "OFF",
           lightState ? "ON" : "OFF",
           config.manualMode ? "Manual" : "Auto",
           WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected",
           String(config.tempThreshold).c_str(),
           String(config.soilMoistureThreshold).c_str());

  // 发送响应
  request->send(200, "text/html; charset=utf-8", buffer);
}

void handleSet(AsyncWebServerRequest *request)
{
  if (request->hasParam("tempThreshold") && request->hasParam("soilMoistureThreshold"))
  {
    config.tempThreshold = request->getParam("tempThreshold")->value().toFloat();
    config.soilMoistureThreshold = request->getParam("soilMoistureThreshold")->value().toFloat();
    saveConfig();
    request->send(200, "text/plain; charset=utf-8", "Settings saved");
  }
  else
  {
    request->send(400, "text/plain; charset=utf-8", "Invalid parameters");
  }
}

void handleToggleMode(AsyncWebServerRequest *request)
{
  config.manualMode = !config.manualMode;
  saveConfig();
  request->send(200, "text/plain; charset=utf-8", "Mode toggled");
}

//  处理WiFi配置页面
void handleWiFiConfig(AsyncWebServerRequest *request)
{
  if (request->method() == HTTP_POST)
  {

    bool hasSsid = request->hasParam("ssid", true);
    bool hasPassword = request->hasParam("password", true);

    // 打印所有参数用于调试
    int paramsCount = request->params();
    for (int i = 0; i < paramsCount; i++)
    {
      const AsyncWebParameter *p = request->getParam(i);
      if (p->isPost())
      {
        Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
      else
      {
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }

    if (hasSsid && hasPassword)
    {
      const AsyncWebParameter *ssidParam = request->getParam("ssid", true);         // 修复：使用 const 类型
      const AsyncWebParameter *passwordParam = request->getParam("password", true); // 修复：使用 const 类型

      strncpy(wifiConfig.ssid, ssidParam->value().c_str(), sizeof(wifiConfig.ssid) - 1);
      wifiConfig.ssid[sizeof(wifiConfig.ssid) - 1] = '\0'; // 确保终止符

      strncpy(wifiConfig.password, passwordParam->value().c_str(), sizeof(wifiConfig.password) - 1);
      wifiConfig.password[sizeof(wifiConfig.password) - 1] = '\0'; // 确保终止符

      saveWiFiConfig();
      request->send(200, "text/plain; charset=utf-8", "WiFi配置已保存");

      // 添加调试信息
      Serial.println("Saved WiFi Config:");
      Serial.print("SSID: ");
      Serial.println(wifiConfig.ssid);
      Serial.print("Password: ");
      Serial.println(wifiConfig.password);
    }
    else
    {
      String errorMessage = "缺失参数: ";
      if (!hasSsid)
        errorMessage += "SSID ";
      if (!hasPassword)
        errorMessage += "Password";
      request->send(400, "text/plain; charset=utf-8", errorMessage);
    }
  }
  else
  {
    // 静态 HTML 模板 enctype='multipart/form-data'
    const char *htmlTemplate =
        "<!DOCTYPE html>"
        "<html><head><title>WiFi Configuration</title></head><body>"
        "<h1>WiFi Configuration</h1>"
        "<form action='/wifiConfigs' method='post' enctype='application/x-www-form-urlencoded'  >"
        "SSID: <input type='text' name='ssid' value='%s'><br>"
        "Password: <input type='password' name='password' value='%s'><br>"
        "<input type='submit' value='Save'>"
        "</form>"
        "</body></html>";

    // 动态内容缓冲区
    char buffer[1024]; // 根据实际需求调整缓冲区大小
    snprintf(buffer, sizeof(buffer),
             htmlTemplate,
             wifiConfig.ssid,
             wifiConfig.password);

    // 发送响应
    request->send(200, "text/html; charset=utf-8", buffer);
  }
}
void initWebServer()
{
 
  // 设置Web服务器并绑定到192.168.4.1
  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_POST, handleSet);
  server.on("/toggleMode", HTTP_POST, handleToggleMode);
  server.on("/wifiConfig", HTTP_GET, handleWiFiConfig);
  server.on("/wifiConfigs", HTTP_POST, handleWiFiConfig);
  // 确保Web服务器启动
  server.begin();
}