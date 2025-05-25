#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "wifi/wifi.h"
#include "mqtt/mqtt.h"
#include "config/config.h"
#include "sensors/sensors.h"
#include "web/server.h"
#include "update/ota.h"
#include "display/display.h"
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

// void handleGetClientCount(AsyncWebServerRequest *request) {
//   JsonDocument jsonDoc;
//    // 获取当前连接数
//   jsonDoc["clientCount"] = 
//   String jsonResponse;
//   serializeJson(jsonDoc, jsonResponse);
  
//   request->send(200, "application/json", jsonResponse);
// }


// 处理请求并返回 JSON 数据
void handleJsonRequest(AsyncWebServerRequest *request)
{
  // 创建一个 JSON 对象
  JsonDocument jsonDoc; // 根据实际需求调整缓冲区大小
  jsonDoc["version"] = SOFTWARE_VERSION;
  // 传感器数据
  jsonDoc["temperature"] = temperature;
  jsonDoc["soilMoisture"] = soilMoisture;
  jsonDoc["dht11Temperature"] = dht11Temperature;
  jsonDoc["dht11SoilMoisture"] = dht11SoilMoisture;
  // 状态配置信息
  jsonDoc["fanState"] = config.fanState;
  jsonDoc["pumpState"] = config.pumpState;
  jsonDoc["lightState"] = config.lightState;
  jsonDoc["manualMode"] = config.manualMode;
  // mqtt 配置
  jsonDoc["mqttServer"] = String(config.mqttServer); 
  jsonDoc["mqttUser"] = String(config.mqttUser);
  jsonDoc["mqttPassword"] = String(config.mqttPassword);
  jsonDoc["mqttTopic"] = String(config.mqttTopic);
  jsonDoc["mqttClientId"] = String(config.mqttClientId);
  jsonDoc["mqttPort"] = config.mqttPort;
  // mqtt 状态
  jsonDoc["mqttState"] = isMQTTConnected() ? "Connected" : "Disconnected";
  // 引脚配置
  jsonDoc["fanPin"] = config.fanPin;     // 风扇引脚
  jsonDoc["pumpPin"] = config.pumpPin;   // 水泵引脚
  jsonDoc["lightPin"] = config.lightPin; // 灯引脚
  jsonDoc["tempPin"] = config.tempPin;  // 温度传感器引脚
  jsonDoc["soilMoisturePin"] = config.soilMoisturePin; // 土壤湿度传感器引脚
  jsonDoc["dhtPin"] = config.dhtPin; // 土壤湿度传感器引脚

  jsonDoc["wifiState"] = WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected";
  // 阈值
  jsonDoc["tempThreshold"] = config.tempThreshold;
  jsonDoc["soilMoistureThreshold"] = config.soilMoistureThreshold;
  
  jsonDoc["totalHeap"] = ESP.getHeapSize();
  // 获取内存使用情况
  jsonDoc["memory"] = ESP.getFreeHeap();
  //  获取 CPU 频率
  jsonDoc["cpu"] = ESP.getCpuFreqMHz();

  // 获取led 对比度
  jsonDoc["contrast"] = config.contrast;

  // 将 JSON 对象转换为字符串
  String jsonResponse;
  serializeJson(jsonDoc, jsonResponse);

  // 发送 JSON 响应，确保字符集为 UTF-8
  request->send(200, "application/json; charset=utf-8", jsonResponse);
}

// 处理请求并返回操作成功 JSON 数据
void handleSuccess(AsyncWebServerRequest *request)
{
  // 创建一个 JSON 对象
  JsonDocument jsonDoc;
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
  JsonDocument jsonDoc;
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
enum class ModeType
{
  MANUAL_MODE,
  FAN_STATE,
  PUMP_STATE,
  LIGHT_STATE,
  UNKNOWN // 用于处理未知模式
};

// 将字符串转换为枚举值
ModeType stringToModeType(const String &modeStr)
{
  if (modeStr == "manualMode")
    return ModeType::MANUAL_MODE;
  if (modeStr == "fanState")
    return ModeType::FAN_STATE;
  if (modeStr == "pumpState")
    return ModeType::PUMP_STATE;
  if (modeStr == "lightState")
    return ModeType::LIGHT_STATE;
  return ModeType::UNKNOWN; // 默认返回未知模式
}

// 跳转
// request->redirect("/");
void handleToggleMode(AsyncWebServerRequest *request)
{
  if (request->hasParam("modeType", true))
  {
    String modeTypeStr = request->getParam("modeType", true)->value();
    ModeType modeType = stringToModeType(modeTypeStr);
    // // Serial.print("server.cpp handleToggleMode modeType: ");
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
    // int paramsCount = request->params();
    // for (int i = 0; i < paramsCount; i++)
    // {
    //   const AsyncWebParameter *p = request->getParam(i);
    //   if (p->isPost())
    //   {
    //     // Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
    //   }
    //   else
    //   {
    //     // Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
    //   }
    // }

    if (hasSsid && hasPassword)
    {
      const AsyncWebParameter *ssidParam = request->getParam("ssid", true);
      const AsyncWebParameter *passwordParam = request->getParam("password", true);
      strncpy(config.ssid, ssidParam->value().c_str(), sizeof(config.ssid) - 1); // 确保字符串安全
      config.ssid[sizeof(config.ssid) - 1] = '\0';                               // 确保终止符
      strncpy(config.password, passwordParam->value().c_str(), sizeof(config.password) - 1);
      config.password[sizeof(config.password) - 1] = '\0'; // 确保终止符
      saveConfig();
      updateConnectWifi();
      request->send(200, "text/plain; charset=utf-8", "WiFi配置已保存");
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

// 处理设置引脚请求 
void handleSetPin(AsyncWebServerRequest *request)
{
  if (request->hasParam("tempPin", true) || request->hasParam("soilMoisturePin", true) 
  || request->hasParam("fanPin", true) || request->hasParam("lightPin", true) 
  || request->hasParam("pumpPin", true)  || request->hasParam("dhtPin", true)  )  
  {
    if (request->hasParam("tempPin", true))
    {
      config.tempPin = request->getParam("tempPin", true)->value().toInt();
    }
    if (request->hasParam("soilMoisturePin", true))
    {
      config.soilMoisturePin = request->getParam("soilMoisturePin", true)->value().toInt();
    }
    if (request->hasParam("fanPin", true))
    {
      config.fanPin = request->getParam("fanPin", true)->value().toInt();
    }
    if (request->hasParam("lightPin", true))
    {
      config.lightPin = request->getParam("lightPin", true)->value().toInt();
    }
    if (request->hasParam("pumpPin", true))
    {
      config.pumpPin = request->getParam("pumpPin", true)->value().toInt();
    }
    if (request->hasParam("dhtPin", true))
    {
      config.dhtPin = request->getParam("dhtPin", true)->value().toInt();
    }
    saveConfig();
    handleSuccess(request); // 返回操作成功
  }
  else
  {
    handleError(request, 400, "Invalid parameters"); // 返回参数错误
  }
}

void handleSetMqtt(AsyncWebServerRequest *request)
{
  if (request->hasParam("mqttServer", true) || request->hasParam("mqttUser", true) || request->hasParam("mqttPassword", true) || request->hasParam("mqttTopic", true) || request->hasParam("mqttClientId", true))
  {
    if (request->hasParam("mqttServer", true))
    {
      strncpy(config.mqttServer, request->getParam("mqttServer", true)->value().c_str(), sizeof(config.mqttServer) - 1); // 确保字符串安全
      config.mqttServer[sizeof(config.mqttServer) - 1] = '\0';                                                           // 确保终止符
    }
    else
    {
      handleError(request, 400, "MQTT服务器地址参数错误");
    }
    if (request->hasParam("mqttUser", true))
    {
      strncpy(config.mqttUser, request->getParam("mqttUser", true)->value().c_str(), sizeof(config.mqttUser) - 1); // 确保字符串安全
      config.mqttUser[sizeof(config.mqttUser) - 1] = '\0';                                                         // 确保终止符
    }
    else
    {
      handleError(request, 400, "MQTT用户名参数错误");
    }
    if (request->hasParam("mqttPassword", true))
    {
      strncpy(config.mqttPassword, request->getParam("mqttPassword", true)->value().c_str(), sizeof(config.mqttPassword) - 1); // 确保字符串安全
      config.mqttPassword[sizeof(config.mqttPassword) - 1] = '\0';                                                             // 确保终止符
    }
    else
    {
      // 密码为空
      config.mqttPassword[0] = '\0';
    }
    if (request->hasParam("mqttTopic", true))
    {
      strncpy(config.mqttTopic, request->getParam("mqttTopic", true)->value().c_str(), sizeof(config.mqttTopic) - 1); // 确保字符串安全
      config.mqttTopic[sizeof(config.mqttTopic) - 1] = '\0';                                                          // 确保终止符
    }
    else
    {
      handleError(request, 400, "MQTT主题参数错误");
    }
    if (request->hasParam("mqttClientId", true))
    {
      strncpy(config.mqttClientId, request->getParam("mqttClientId", true)->value().c_str(), sizeof(config.mqttClientId) - 1); // 确保字符串安全
      config.mqttClientId[sizeof(config.mqttClientId) - 1] = '\0';                                                             // 确保终止符
    }
    else
    {
      handleError(request, 400, "MQTT客户端ID参数错误");
    }
    if (request->hasParam("mqttPort", true))
    {     
      config.mqttPort = request->getParam("mqttPort", true)->value().toInt();                                                       // 确保终止符
    }
    else
    {
      handleError(request, 400, "MQTT客户端ID参数错误");
    }

    saveConfig();
    // initMQTT();
    // reconnectMQTT();
    handleSuccess(request);
  }
}

void handleSystemOperation(AsyncWebServerRequest *request)
{
  if (request->hasParam("reboot", true))
  {
    // 重启 ESP32
    ESP.restart();
  }
  else if (request->hasParam("update", true))
  {
    updateState = true;
  }
  else
  {
    handleError(request, 400, "Invalid parameters");
  }
}
void handleSetContrast(AsyncWebServerRequest *request){
  if (request->hasParam("contrast", true))
  {
    config.contrast = request->getParam("contrast", true)->value().toInt();
    saveConfig();
    setBrightness(config.contrast);
    handleSuccess(request);
  }
  else
  {
    handleError(request, 400, "Invalid parameters");
  }
}

// 初始化Web服务器
void initWebServer()
{
  // 初始化 SPIFFS
  if (!SPIFFS.begin(true))
  {
     Serial.println("初始化 SPIFFS 时出错");
    // return;
  }
  else
  {
     Serial.println("SPIFFS 初始化成功");
  }

  // 检查 index.html 文件是否存在
  if (!SPIFFS.exists("/templates/index.html"))
  {
     Serial.println("在 SPIFFS 中找不到 index.html 文件");
    // return;
  }
  else
  {
     Serial.println("在 SPIFFS 中找到 index.html 文件");
  }

  //  .sendHeader("Access-Control-Allow-Origin", "*"); // 允许跨域访问
  // 设置Web服务器并绑定到192.168.4.1  http://192.168.4.1/wifiConfigs

  server.on("/", HTTP_GET, handleRoot);
  server.on("/set", HTTP_POST, handleSet);                // 处理阈值参数设置请求
  server.on("/toggleMode", HTTP_POST, handleToggleMode);  // 处理切换模式请求
  server.on("/wifiConfig", HTTP_GET, handleWiFiConfig);   // 处理WiFi配置请求
  server.on("/wifiConfigs", HTTP_POST, handleWiFiConfig); // 处理WiFi配置请求
  server.on("/api/data", HTTP_GET, handleJsonRequest);    // 处理JSON数据请求
  server.on("/setPin", HTTP_POST, handleSetPin);          // 处理设置引脚请求
  server.on("/setMqtt", HTTP_POST, handleSetMqtt);        // 保存mqtt 设置
  server.on("/systemOperation", HTTP_POST, handleSystemOperation);        // 处理系统操作请求
  // 设置对比度
  server.on("/setContrast", HTTP_POST, handleSetContrast);

  // server.on("/setSoilMoisturePin", HTTP_POST, handleSetSoilMoisturePin);
  // 确保Web服务器启动
  server.begin();
  // Serial.println("Web server started");
}
