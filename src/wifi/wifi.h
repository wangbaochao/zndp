#ifndef WIFI_H
#define WIFI_H

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
 
void createWiFiAP();
void initConnectWiFi();
void updateConnectWifi();

#endif