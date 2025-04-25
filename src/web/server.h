#ifndef SERVER_H
#define SERVER_H

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
//  // Web服务器 定义为全局变量
// AsyncWebServer server(80);

// 声明 Web 服务器对象为 extern，避免头文件中直接定义全局变量
extern AsyncWebServer server;

void initWebServer();


#endif