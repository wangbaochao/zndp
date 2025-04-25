#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>
#include <u8g2_wqy.h>

#define OLED_SDA 41
#define OLED_SCL 42
#define OLED_RESET -1
// OLED显示屏定义
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32      // 修改为32，适配0.91寸显示屏
#define OLED_I2C_ADDRESS 0x3C // 修改为0x3C标准地址

extern U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2;

void initOLED();
void updateOLED(float temperature, float soilMoisture);

#endif