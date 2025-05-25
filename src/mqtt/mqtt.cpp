#include <PubSubClient.h>
#include <WiFi.h>
#include "sensors/sensors.h"
#include "mqtt.h"
#include "config/config.h"
#include <ArduinoJson.h>

#include <string.h>
#include <stdlib.h>

static unsigned long lastMqttUpdateTime = 0; // 记录上次 更新时间
static int mqttRetryCount = 0;               // MQTT连接重试次数
// 定义最大重试次数
static const int MAX_MQTT_RETRY_COUNT = 50;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// 添加传感器数据变量
// float temperature = 0.0;
// float soilMoisture = 0.0;

//

      // if (!mqttClient.connected())
      // {
      //   if (millis() - lastMqttUpdateTime >= 6000)
      //   {
      //     Serial.println("每6秒重试一次mqtt");
      //     lastMqttUpdateTime = millis();
      //     if (retryConnection("MQTT", reconnectMQTT, mqttRetryCount))
      //     {
      //       initMQTT();
      //     };
      //   }
      // }
      // else
      // {
      //   mqttRetryCount = 0;
      //   if (millis() - lastMqttUpdateTime >= 10000)
      //   {
      //     Serial.println("MQTT连接成功");
      //     lastMqttUpdateTime = millis();
      //     updateMQTT(temperature, soilMoisture);
      //   }
      // }

// 重试连接逻辑
static bool retryConnection(const char *connectionType, void (*connectFunction)(), int &retryCount)
{
  if (retryCount < MAX_MQTT_RETRY_COUNT)
  {
    Serial.printf("%s未连接，尝试重新连接... (重试次数: %d)\n", connectionType, retryCount + 1);
    connectFunction();
    retryCount++;
    return true;
  }
  else
  {
    Serial.printf("%s连接失败，已达到最大重试次数，停止重试...\n", connectionType);
    return false;
  }
}


// MQTT回调函数
void mqttCallback(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }

  // 解析JSON数据
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error)
  {
    // Serial.print("JSON parse failed: ");
    Serial.println(error.c_str());
    return;
  }

  // 获取设备状态
  const char *device = doc["device"]; // 设备名称
  const char *action = doc["action"]; // 动作

  if (strcmp(device, "fan") == 0)
  {
    if (strcmp(action, "ON") == 0)
    {
      digitalWrite(config.fanPin, HIGH);
      config.fanState = true;
    }
    else if (strcmp(action, "OFF") == 0)
    {
      digitalWrite(config.fanPin, LOW);
      config.fanState = false;
    }
  }
  else if (strcmp(device, "pump") == 0)
  {
    if (strcmp(action, "ON") == 0)
    {
      digitalWrite(config.pumpPin, HIGH);
      config.pumpState = true;
    }
    else if (strcmp(action, "OFF") == 0)
    {
      digitalWrite(config.pumpPin, LOW);
      config.pumpState = false;
    }
  }
  else if (strcmp(device, "light") == 0)
  {
    if (strcmp(action, "ON") == 0)
    {
      digitalWrite(config.lightPin, HIGH);
      config.lightState = true;
    }
    else if (strcmp(action, "OFF") == 0)
    {
      digitalWrite(config.lightPin, LOW);
      config.lightState = false;
    }
  }
}

// MQTT连接
void reconnectMQTT()
{
  if (mqttClient.connect(config.mqttClientId, config.mqttUser, config.mqttPassword))
  {
    Serial.println("connected");
    // 订阅单一主题
    mqttClient.subscribe("dht/control");
  }
  else
  {
    // Serial.print("failed, rc=");
    // Serial.print(mqttClient.state());
    Serial.println(" try again in 5 seconds");
  }
}

void initMQTT()
{
  mqttClient.setServer(config.mqttServer, config.mqttPort);
  mqttClient.setCallback(mqttCallback);
}

// 发布Home Assistant自动发现消息
// void publishDiscoveryMessage(const char *unique_id, const char *device_class, const char *state_topic, const char *unit_of_measurement, const char *icon)
// {

//   // char modelStr[50];
//   // snprintf(modelStr, sizeof(modelStr), "ZNDP Model (%s)", SOFTWARE_VERSION);

//   JsonDocument doc;
//   doc["name"] = unique_id; // 设备名称
//   doc["unique_id"] = unique_id; // 设备唯一标识符
//   doc["device_class"] = device_class; // 传感器类别
//   doc["state_topic"] = state_topic; // 状态主题
//   doc["unit_of_measurement"] = unit_of_measurement; // 单位
//   // doc["icon"] = icon; // 图标
//   // doc["device"]["identifiers"] = "zndp_device";  // 设备标识符
//   // doc["device"]["name"] = "ZNDP Device";         // 设备名称
//   // doc["device"]["manufacturer"] = "wangbaochao"; // 设备制造商
//   // doc["device"]["model"] = "ZNDP Model";         // 设备型号

//   JsonObject device = doc["device"].to<JsonObject>();
//   device["manufacturer"] = "Waveshare";
//   device["model"] = "ESP32-S3-SIM7670G-4G";
//   // device["identifiers"] = "zndp_device";
//   // 确保 "identifiers" 是一个数组
//   // JsonArray identifiers = device.createNestedArray("identifiers");
//   // identifiers.add("zndp_device");

//   char jsonBuffer[5120];
//   serializeJson(doc, jsonBuffer);
//   // 检查JSON缓冲区是否为空
//   if (strlen(jsonBuffer) == 0)
//   {
//     Serial.println("JSON serialization failed: Buffer is empty");
//     return;
//   }
//   // 发布发现消息到homeassistant/sensor/<unique_id>/config  homeassistant/sensor/dht/temperature/config
//   String topicStr = "homeassistant/sensor/dht/" + String(unique_id) + "/config";
//   const char *topic = topicStr.c_str();
//   // 检查主题是否为空
//   if (strlen(topic) == 0)
//   {
//     Serial.println("Topic generation failed: Topic is empty");
//     return;
//   }
//   // 检查MQTT连接状态
//   if (!mqttClient.connected())
//   {
//     Serial.println("MQTT not connected, cannot publish data");
//     return;
//   }

//   // 发布消息并检查返回值
//   bool publishResult = mqttClient.publish(topic, jsonBuffer);
//   if (publishResult)
//   {
//     // Serial.print("Published to topic: ");
//     Serial.println(topic);
//     // Serial.print("Published JSON data: ");
//     Serial.println(jsonBuffer);
//   }
//   else
//   {
//     // Serial.print("Failed to publish to topic: ");
//     Serial.println(topic);
//     // Serial.print("JSON data: ");
//     Serial.println(jsonBuffer);
//   }

//   // 使用完成后释放内存
//    // free(jsonBuffer);
// }

void updateMQTT(float temp, float moisture)
{
  temperature = temp;
  soilMoisture = moisture;

  if (!mqttClient.connected() || mqttClient.state() == 0) {
    reconnectMQTT();
  }

  mqttClient.loop(); // 确保MQTT循环运行

  if (mqttClient.connected())
  {


    // Serial.print("MQTT state11 code: ");
    Serial.println(mqttClient.state());

    // 创建JSON对象
    JsonDocument doc;
    doc["temperature"] = temperature;
    doc["soil_moisture"] = soilMoisture;

    // 序列化JSON数据
    char jsonBuffer[200];
    serializeJson(doc, jsonBuffer);
     
    // 发布消息并检查返回值
    bool publishResult = mqttClient.publish("dht/data", jsonBuffer);
    if (publishResult)
    {
      // Serial.print("Published to topic: ");
      Serial.println("dht/data");
      // Serial.print("Published JSON data: ");
      Serial.println(jsonBuffer);
    }
    else
    {
      // Serial.print("Failed to publish to topic: ");
      // Serial.print("JSON data: ");
      Serial.println(jsonBuffer);
      // Serial.print("MQTT state code: ");
      Serial.println(mqttClient.state());
      if(mqttClient.state() == 0){
        reconnectMQTT();
      }
    }

    // Serial.print("Published JSON data: ");
    Serial.println(jsonBuffer);
  }
  else
  {
    Serial.println("MQTT not connected, cannot publish data");
  }
}

// 获取MQTT 连接状态
bool isMQTTConnected()
{
  return mqttClient.connected();
}