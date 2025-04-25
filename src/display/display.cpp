#include <U8g2lib.h>
#include <u8g2_wqy.h>
#include "display.h"

// OLED显示屏定义
U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE);

// 初始化OLED
void initOLED() {
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
}

// 更新OLED显示
void updateOLED(float temperature, float soilMoisture) {
  u8g2.clearBuffer();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy12_t_gb2312);
  u8g2.firstPage();
  do {
    u8g2.setCursor(0, 15);
    u8g2.println("温度: " + String(temperature) + " °C");
    u8g2.setCursor(0, 31);
    u8g2.println("湿度: " + String(soilMoisture) + " %");
  } while (u8g2.nextPage());
}