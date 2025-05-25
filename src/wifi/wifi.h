#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>


extern int8_t rssi;
 
void createWiFiAP();
void initConnectWiFi();
void updateConnectWifi();
void checkWiFiStatus();

#endif