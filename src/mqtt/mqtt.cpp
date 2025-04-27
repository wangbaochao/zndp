#include <PubSubClient.h>
#include <WiFi.h>
//#include "wifi/wifi.h"
#include "sensors/sensors.h"
#include "mqtt.h"
#include "config/config.h"

// MQTT配置
const char *mqtt_server = "mqtt.eclipseprojects.io";
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// MQTT回调函数
void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "dht/fan") {
    if (message == "ON") {
      digitalWrite(config.fanPin, HIGH);
      config.fanState = true;
    } else if (message == "OFF") {
      digitalWrite(config.fanPin, LOW);
      config.fanState = false;
    }
  } else if (String(topic) == "dht/pump") {
    if (message == "ON") {
      digitalWrite(config.pumpPin, HIGH);
      config.pumpState = true;
    } else if (message == "OFF") {
      digitalWrite(config.pumpPin, LOW);
      config.pumpState = false;
    }
  } else if (String(topic) == "dht/light") {
    if (message == "ON") {
      digitalWrite(config.lightPin, HIGH);
      config.lightState = true;
    } else if (message == "OFF") {
      digitalWrite(config.lightPin, LOW);
      config.lightState = false;
    }
  }
}

// MQTT连接
void reconnectMQTT() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32Client")) {
      mqttClient.subscribe("dht/fan");
      mqttClient.subscribe("dht/pump");
      mqttClient.subscribe("dht/light");
    } else {
      delay(5000);
    }
  }
}