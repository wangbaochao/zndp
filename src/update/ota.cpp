#include <ArduinoOTA.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "ota.h"
#include "config/config.h"
// #include "display/display.h"
#include <SPIFFS.h>

// 远程固件链接，只支持http
// const char* baseUpdateUrl = "http://192.168.0.108:9000/update/zndp/firmware.bin";
const char *updateJson = "http://192.168.0.108:9000/update/zndp/esp32_update.json";

// 设置当前代码版本
char *version = strdup(SOFTWARE_VERSION);

// 设置当前代码版本
char *htmlVersion = strdup(SOFTWARE_HTML_VERSION);

int need_ota_update = 0;
int i = 0;
String jsonBuffer;

// 获取远程 json 升级文件
String httpGETRequest(const char *serverName)
{
  WiFiClient client;
  HTTPClient http;
  String payload = "";
  // 连接目标网址
  http.begin(client, serverName);
  // 发送HTTP站点请求
  int httpCode = http.GET();
  if (httpCode > 0)
  {
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
    payload = http.getString();
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); // 关闭连接
  // 返回获得的数据用于Json处理
  return payload;
}

void updateHTMLFile(String htmlUrl)
{
  HTTPClient http;
  http.begin(htmlUrl);
  int httpCode = http.GET(); // 发送HTTP站点请求
  if (httpCode == HTTP_CODE_OK)
  {
    Serial.println("HTTP request succeeded, downloading HTML file...");

    // 打开文件以写入模式
    File file = SPIFFS.open("/templates/index.html", "w");
    if (!file)
    {
      Serial.println("Failed to open file for writing");
      http.end();
      return;
    }

    // 使用缓冲区逐块读取数据
    WiFiClient *stream = http.getStreamPtr();
    uint8_t buffer[64];
    int bytesRead;

    while ((bytesRead = stream->available()) > 0)
    {
      int bytesToRead = min(64, bytesRead);
      int readBytes = stream->readBytes(buffer, bytesToRead);
      if (readBytes > 0)
      {
        file.write(buffer, readBytes); // 写入文件
      }
    }

    file.close();
    Serial.println("HTML file updated successfully");
  }
  else
  {
    Serial.printf("Failed to download HTML file: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end(); // 关闭HTTP连接
}

// 依据json文件中版本号与本地版本号，判断是否需要进行更新
void isOrNotNeedUpdate()
{
  // 获取远程的升级 json ，判断内部版本与本地是否相同，判断是否需要升级
  jsonBuffer = httpGETRequest(updateJson);
  if (jsonBuffer.length() == 0)
  {
    Serial.println("获取远程升级文件失败");
    return;
  }
  Serial.println(jsonBuffer);

  // 创建一个 JSON 文档对象
  JsonDocument doc;

  // 解析 JSON 数据
  DeserializationError error = deserializeJson(doc, jsonBuffer);
  if (error)
  {
    // Serial.print("JSON 解析失败: ");
    Serial.println(error.c_str());
    return;
  }

  // 获取远程版本号
  const char *ota_version = doc["version"];
  if (!ota_version)
  {
    Serial.println("远程版本号获取失败");
    return;
  }

  // 获取 html 远程版本号
  const char *firmwareHtmlVersion = doc["firmwareHtmlVersion"];
  if (!firmwareHtmlVersion)
  {
    Serial.println("远程版本号获取失败");
    return;
  }

  // 获取下载url
  const char *firmwareUrl = doc["firmwareUrl"];
  if (!firmwareUrl)
  {
    Serial.println("远程下载地址获取失败");
    return;
  }

  // 获取html下载url
  const char *firmwareHtml = doc["firmwareHtml"];
  if (!firmwareHtml)
  {
    Serial.println("远程html下载地址获取失败");
    return;
  }

  Serial.println("---");

  // Serial.print("远程版本： ");
  Serial.println(ota_version);
  // Serial.print("本地版本： ");
  Serial.println(version);

  // 判断远程版本与本地版本是否相同
  if (String(htmlVersion) == String(firmwareHtmlVersion))
  {
    need_ota_update = 0;
    Serial.println("html 无需升级。。。");
  }
  else
  {
    Serial.println("html 需要升级。。。");
    updateHTMLFile(firmwareHtml);
  }

  // 判断远程版本与本地版本是否相同
  if (String(version) == String(ota_version))
  {
    need_ota_update = 0;
    Serial.println("无需升级。。。");
  }
  else
  {
    need_ota_update = 1;
    Serial.println("需要升级。。。");
    // Serial.print("OTA 升级地址为：");
    // showUpdate(String(ota_version));
    // updateHTMLFile(firmwareHtml);
    // updateHTMLFile(String(firmwareHtml));
    // 升级的完整链接，例如：http://example.cn/esp32/esp32_1_0_1.bin
    String fullUpdateUrl = firmwareUrl;
    Serial.println(String(fullUpdateUrl));
    WiFiClient client;
    // 获取远程 bin 文件进行升级

    t_httpUpdate_return ret = httpUpdate.update(client, fullUpdateUrl);
    Serial.println("获取远程 bin 文件进行升级");
    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("HTTP_UPDATE_NO_UPDATES");
      break;
    case HTTP_UPDATE_OK:
      ESP.restart();
      break;
    default:
      Serial.println(ret);
    }
  }
  need_ota_update = 0;
}

void initOTA()
{
  ArduinoOTA.setHostname("ESP32_OTA_Device"); // 设置设备名称
  ArduinoOTA.setPassword("12345678");         // 设置 OTA 密码
  ArduinoOTA.onStart([]()
                     {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }
        Serial.println("Start updating " + type); });
  ArduinoOTA.onEnd([]()
                   { Serial.println("\nEnd"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
  ArduinoOTA.onError([](ota_error_t error)
                     {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        } });
  ArduinoOTA.begin(); // 启动 OTA
  Serial.println("OTA initialized");
}

// 处理OTA事件
void handleOTA()
{
  ArduinoOTA.handle();
}
