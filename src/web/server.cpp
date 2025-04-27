#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "wifi/wifi.h"
#include "config/config.h"
#include "sensors/sensors.h"
#include "web/server.h"
#include <FS.h>
#include <SPIFFS.h>

// Web服务器 定义为全局变量
AsyncWebServer server(80);

// 处理请求并返回 HTML
void handleRequestHtml(AsyncWebServerRequest *request, const String &path)
{
  request->send(SPIFFS, path, "text/html; charset=utf-8");
}

// Web服务器处理
void handleRoot(AsyncWebServerRequest *request)
{
  // 直接从 SPIFFS 流式传输 HTML 文件
  handleRequestHtml(request, String("/templates/index.html"));
}

// 处理请求并返回 JSON 数据
void handleJsonRequest(AsyncWebServerRequest *request)
{
  // 创建一个 JSON 对象
  JsonDocument jsonDoc; // 根据实际需求调整缓冲区大小
  jsonDoc["version"] = SOFTWARE_VERSION;
  jsonDoc["temperature"] = temperature;
  jsonDoc["soilMoisture"] = soilMoisture;

  jsonDoc["fanState"] = config.fanState;
  jsonDoc["pumpState"] = config.pumpState;
  jsonDoc["lightState"] = config.lightState;
  jsonDoc["manualMode"] = config.manualMode;

  jsonDoc["wifiState"] = WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected";
  jsonDoc["tempThreshold"] = config.tempThreshold;
  jsonDoc["soilMoistureThreshold"] = config.soilMoistureThreshold;
  jsonDoc["tempPin"] = config.tempPin;
  jsonDoc["soilMoisturePin"] = config.soilMoisturePin;
  jsonDoc["totalHeap"]  = ESP.getHeapSize();
  // 获取内存使用情况
  jsonDoc["memory"] = ESP.getFreeHeap();
  //  获取 CPU 频率
  jsonDoc["cpu"] = ESP.getCpuFreqMHz();

  // 将 JSON 对象转换为字符串
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  // 发送 JSON 响应
  request->send(200, "application/json; charset=utf-8;", jsonResponse);
}

// 处理请求并返回操作成功 JSON 数据
void handleSuccess(AsyncWebServerRequest *request)
{
  // 创建一个 JSON 对象
  JsonDocument jsonDoc; // 根据实际需求调整缓冲区大小
  jsonDoc["code"] = 200;
  jsonDoc["msg"] = "成功";
  // 将 JSON 对象转换为字符串
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  request->send(200, "application/json; charset=utf-8;", jsonResponse);
}

// 处理请求并返回操作失败 JSON 数据
void handleError(AsyncWebServerRequest *request, int code = 500, const char *msg = "操作失败")
{
  // 创建一个 JSON 对象
  JsonDocument jsonDoc; // 根据实际需求调整缓冲区大小
  jsonDoc["code"] = code;
  jsonDoc["msg"] = msg;
  // 将 JSON 对象转换为字符串
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  request->send(200, "application/json; charset=utf-8;", jsonResponse);
}

// 阈值配置
void handleSet(AsyncWebServerRequest *request)
{
  if (request->hasParam("tempThreshold", true) || request->hasParam("soilMoistureThreshold", true))
  {
    if (request->hasParam("tempThreshold", true))
    {
      config.tempThreshold = request->getParam("tempThreshold", true)->value().toFloat();
    }
    if (request->hasParam("soilMoistureThreshold", true))
    {
      config.soilMoistureThreshold = request->getParam("soilMoistureThreshold", true)->value().toFloat();
    }
    saveConfig();
    // 返回操作成功 JSON 数据
    handleSuccess(request);
  }
  else
  {
    handleError(request, 400, "Invalid parameters");
  }
}

// 定义枚举类
enum class ModeType {
  MANUAL_MODE,
  FAN_STATE,
  PUMP_STATE,
  LIGHT_STATE,
  UNKNOWN // 用于处理未知模式
};

// 辅助函数：将字符串转换为枚举值
ModeType stringToModeType(const String &modeStr) {
  if (modeStr == "manualMode") return ModeType::MANUAL_MODE;
  if (modeStr == "fanState") return ModeType::FAN_STATE;
  if (modeStr == "pumpState") return ModeType::PUMP_STATE;
  if (modeStr == "lightState") return ModeType::LIGHT_STATE;
  return ModeType::UNKNOWN; // 默认返回未知模式
}

// 跳转
// request->redirect("/");
 


void handleToggleMode(AsyncWebServerRequest *request)
{
  // Serial.print("server.cpp handleToggleMode: ");
  // Serial.println(request->hasParam("modeType", true));

  if (request->hasParam("modeType", true))
  {
    String modeTypeStr = request->getParam("modeType", true)->value();
    ModeType modeType = stringToModeType(modeTypeStr);
    // Serial.print("server.cpp handleToggleMode modeType: ");
    // Serial.println(modeTypeStr);  
    switch (modeType)
    {
      case ModeType::MANUAL_MODE:
        config.manualMode = (request->getParam("manualMode", true)->value() == "true") ? true : false;  
        break;
      case ModeType::FAN_STATE:
        config.fanState = (request->getParam("fanState", true)->value() == "true") ? true : false; 
        break;
      case ModeType::PUMP_STATE:
        config.pumpState = (request->getParam("pumpState", true)->value() == "true") ? true : false; 
        break;
      case ModeType::LIGHT_STATE:
        config.lightState = (request->getParam("lightState", true)->value() == "true") ? true : false; 
        break;
      case ModeType::UNKNOWN:
        handleError(request, 400, "未知的模式类型");  
        break;
      default:
        handleError(request, 400, "无效的模式类型");
        break;
    }
    saveConfig();
    updateDeviceStates();
    handleSuccess(request); // 返回操作成功
  }
  else
  {
    handleError(request, 400, "处理切换模式请求参数错误");
  }
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
        // Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
      else
      {
        // Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
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

      // Serial.print("handleWiFiConfig SSID: ");
      // Serial.println(config.ssid);
      // Serial.print("handleWiFiConfig Password: ");
      // Serial.println(config.password);

      saveConfig();
      request->send(200, "text/plain; charset=utf-8", "WiFi配置已保存");

      // 添加调试信息
      // Serial.println("Saved WiFi Config:");
      // Serial.print("SSID: ");
      // Serial.println(config.ssid);
      // Serial.print("Password: ");
      // Serial.println(config.password);
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
        "<html><head><title>WiFi配置</title>"
        "<style>"
        "body { font-family: Arial, sans-serif; background-color: #f4f4f4; margin: 0; padding: 20px; }"
        "h1 { color: #333; }"
        "form { margin-top: 20px; }"
        "input[type='text'], input[type='password'], input[type='submit'] { padding: 10px; font-size: 16px; margin-top: 10px; }"
        "input[type='submit'] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }"
        "input[type='submit']:hover { background-color: #45a049; }"
        "</style>"
        "</head><body>"
        "<h1>WiFi配置</h1>"
        "<form action='/wifiConfigs' method='post' enctype='application/x-www-form-urlencoded'>"
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

// 处理设置引脚请求 0: tempPin 1: soilMoisturePin
void handleSetPin(AsyncWebServerRequest *request)
{
  if (request->hasParam("tempPin", true) || request->hasParam("soilMoisturePin", true))
  {
    if (request->hasParam("tempPin", true))
    {
      config.tempPin = request->getParam("tempPin", true)->value().toInt();
     //  config.tempThreshold = request->getParam("tempThreshold", true)->value().toFloat();
    }
    if (request->hasParam("soilMoisturePin", true))
    {
      // config.soilMoisturePin = request->getParam("soilMoisturePin", true).value();
      config.soilMoisturePin = request->getParam("soilMoisturePin", true)->value().toInt();
    }
    saveConfig();
    handleSuccess(request); // 返回操作成功
  }
  else
  {
    handleError(request, 400, "Invalid parameters"); // 返回参数错误
  }
}

// 初始化Web服务器
void initWebServer()
{
  // 初始化 SPIFFS
  if (!SPIFFS.begin(true))
  {
    // Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  else
  {
    // Serial.println("SPIFFS mounted successfully");
  }

  // 检查 index.html 文件是否存在
  if (!SPIFFS.exists("/templates/index.html"))
  {
    // Serial.println("index.html file not found in SPIFFS");
    // return;
  }
  else
  {
    // Serial.println("index.html file found in SPIFFS");
  }

  //  .sendHeader("Access-Control-Allow-Origin", "*"); // 允许跨域访问
  // 设置Web服务器并绑定到192.168.4.1
  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_POST, handleSet);                // 处理阈值参数设置请求
  server.on("/toggleMode", HTTP_POST, handleToggleMode);  // 处理切换模式请求
  server.on("/wifiConfig", HTTP_GET, handleWiFiConfig);   // 处理WiFi配置请求
  server.on("/wifiConfigs", HTTP_POST, handleWiFiConfig); // 处理WiFi配置请求
  server.on("/api/data", HTTP_GET, handleJsonRequest);    // 处理JSON数据请求
  server.on("/setPin", HTTP_POST, handleSetPin);          // 处理设置引脚请求
  // server.on("/setSoilMoisturePin", HTTP_POST, handleSetSoilMoisturePin);
  // 确保Web服务器启动
  server.begin();
  // Serial.println("Web server started");
}
