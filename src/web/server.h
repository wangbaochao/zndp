#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>

// 声明 Web 服务器对象为 extern，避免头文件中直接定义全局变量
extern AsyncWebServer server;

void initWebServer();

#endif