#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

void initWebServer();

#endif