#ifndef MQTT_H
#define MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>

extern const char *mqtt_server;
extern WiFiClient espClient;
extern PubSubClient mqttClient;

void mqttCallback(char *topic, byte *payload, unsigned int length);
void reconnectMQTT();
void initMQTT();
void updateMQTT(float temperature, float soilMoisture);
bool isMQTTConnected();

#endif