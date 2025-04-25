#include <PubSubClient.h>
#include <WiFi.h>
//#include "wifi/wifi.h"
#include "sensors/sensors.h"
#include "mqtt.h"

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
      digitalWrite(FAN_PIN, HIGH);
      fanState = true;
    } else if (message == "OFF") {
      digitalWrite(FAN_PIN, LOW);
      fanState = false;
    }
  } else if (String(topic) == "dht/pump") {
    if (message == "ON") {
      digitalWrite(PUMP_PIN, HIGH);
      pumpState = true;
    } else if (message == "OFF") {
      digitalWrite(PUMP_PIN, LOW);
      pumpState = false;
    }
  } else if (String(topic) == "dht/light") {
    if (message == "ON") {
      digitalWrite(LIGHT_PIN, HIGH);
      lightState = true;
    } else if (message == "OFF") {
      digitalWrite(LIGHT_PIN, LOW);
      lightState = false;
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