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
      "<html><head><title>温室环境控制系统</title></head><body>"
      "<h1>温室环境控制系统</h1>"
      "<p>温&nbsp;&nbsp;度: %s °C</p>"
      "<p>土壤湿度: %s %%</p>"
      "<p>风&nbsp;&nbsp;扇: %s</p>"
      "<p>水&nbsp;&nbsp;泵: %s</p>"
      "<p>电&nbsp;&nbsp;灯: %s</p>"
      "<form action='/toggleMode' method='POST'>"
      "<p>工作模式: %s</p> <input type='submit' value='切换模式'>"
      "</form>"

      "<p>WiFi状态: %s</p>"
      "<form action='/set' method='POST'>"
      "温度阈值: <input type='number' name='tempThreshold' value='%s'><br>"
      "湿度阈值: <input type='number' name='soilMoistureThreshold' value='%s'><br>"
      "<input type='submit' value='保存阈值'>"
      "</form>"

      // "<form id='toggleModeForm' action='/toggleMode' method='POST'>"
      // "  <input type='radio' name='mode' value='%s' onclick='this.form.submit();' checked> 自动<br>"
      // "  <input type='radio' name='mode' value='%s' onclick='this.form.submit();'> 手动<br>"
      // "</form>"
      "<a href='/wifiConfig'>设置 WiFi</a>"
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
           String(config.soilMoistureThreshold).c_str()

  );

  // 发送响应
  request->send(200, "text/html; charset=utf-8", buffer);
}

void handleSet(AsyncWebServerRequest *request)
{
  if (request->hasParam("tempThreshold", true) && request->hasParam("soilMoistureThreshold", true))
  {
    config.tempThreshold = request->getParam("tempThreshold", true)->value().toFloat();
    config.soilMoistureThreshold = request->getParam("soilMoistureThreshold", true)->value().toFloat();
    saveConfig();
    // request->send(200, "text/plain; charset=utf-8", "Settings saved");
    // 跳转
    request->redirect("/");
  }
  else
  {
    request->send(400, "text/plain; charset=utf-8", "Invalid parameters");
  }
}

void handleToggleMode(AsyncWebServerRequest *request)
{
  config.manualMode = !config.manualMode; // 切换模式
  saveConfig();                           // 保存配置到 EEPROM
  // 跳转
  request->redirect("/");
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
      const AsyncWebParameter *ssidParam = request->getParam("ssid", true);
      const AsyncWebParameter *passwordParam = request->getParam("password", true);

      strncpy(config.ssid, ssidParam->value().c_str(), sizeof(config.ssid) - 1); // 确保字符串安全
      config.ssid[sizeof(config.ssid) - 1] = '\0';                               // 确保终止符

      strncpy(config.password, passwordParam->value().c_str(), sizeof(config.password) - 1);
      config.password[sizeof(config.password) - 1] = '\0'; // 确保终止符

      Serial.print("handleWiFiConfig SSID: ");
      Serial.println(config.ssid);
      Serial.print("handleWiFiConfig Password: ");
      Serial.println(config.password);

      saveConfig();
      request->send(200, "text/plain; charset=utf-8", "WiFi配置已保存");

      // 添加调试信息
      Serial.println("Saved WiFi Config:");
      Serial.print("SSID: ");
      Serial.println(config.ssid);
      Serial.print("Password: ");
      Serial.println(config.password);
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
        "<html><head><title>WiFi配置</title></head><body>"
        "<h1>WiFi配置</h1>"
        "<form action='/wifiConfigs' method='post' enctype='application/x-www-form-urlencoded'  >"
        "WiFi SSID: <input type='text' name='ssid' value='%s'><br>"
        "WiFi 密码: <input type='password' name='password' value='%s'><br>"
        "<input type='submit' value='保存'>"
        "</form>"
        "</body></html>";

    // 动态内容缓冲区
    char buffer[1024]; // 根据实际需求调整缓冲区大小
    snprintf(buffer, sizeof(buffer),
             htmlTemplate,
             config.ssid,
             config.password);

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