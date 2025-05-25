#include <U8g2lib.h>
#include <u8g2_wqy.h>
#include "display.h"
#include "wifi/wifi.h"
#include "config/config.h"

static unsigned long lastOLEDUpdateTime = 0; // 记录上次OLED更新时间

uint8_t displayOffsetX = 0;
uint8_t displayOffsetY = 0;

// OLED显示屏定义  U8G2_SSD1306_128X32_UNIVISION_F_SW_I2C u8g2(U8G2_R0, OLED_SCL, OLED_SDA, U8X8_PIN_NONE); U8G2_SH1107_128X128_F_SW_I2C
// U8G2_SH1107_128X128_1_SW_I2C u8g2(U8G2_R3, OLED_SCL, OLED_SDA, OLED_RESET);
U8G2_SH1107_128X128_1_HW_I2C u8g2(U8G2_R0, OLED_RESET, OLED_SCL, OLED_SDA);

// 新增开机动画
void showBootAnimation()
{

  u8g2.firstPage();
  do
  {
    u8g2.clearBuffer();

    // 创建放大动画效果
    for (int size = 0; size <= 64; size += 4)
    {
      int x = 64 - size / 2;
      int y = 64 - size / 2;
      u8g2.drawFrame(x, y, size, size);
      delay(10);
    }

    // 填充中心圆形
    for (int radius = 0; radius <= 32; radius += 2)
    {
      u8g2.drawDisc(64, 64, radius);
      delay(5);
    }

    // 显示文字
    u8g2.enableUTF8Print();
    u8g2.setFont(u8g2_font_wqy16_t_gb2312b);
    u8g2.setCursor(40, 50);
    u8g2.println("智能大棚");
    u8g2.setCursor(40, 70);
    u8g2.println("控制系统");
    u8g2.setCursor(30, 110);
    u8g2.println("(" + String(SOFTWARE_VERSION) + ")");

  } while (u8g2.nextPage());

  // 等待片刻
  // delay(2000);
}
unsigned long lastActivity = millis();
// 自动息屏 + 按键唤醒
void autoSleep()
{
  // 创建一个定时器，用于在1分钟后自动进入休眠模式
  if (millis() - lastActivity > 60000)
  {                       // 1分钟后
    u8g2.setPowerSave(1); // 开启省电模式（关闭显示）
  }
  else
  {
    u8g2.setPowerSave(0); // 关闭省电模式（恢复显示）
  }

  // 模拟按键检测
  // if (digitalRead(buttonPin) == LOW) {
  //   lastActivity = millis();
  // }
}

// 初始化OLED
void initOLED()
{
  // u8g2.setI2CAddress(0x74 << 1); // SH1107 默认地址 0x74
  u8g2.begin();
  // u8g2.setBusClock(100000); // 设置总线时钟
  // u8g2.setContrast(0x7F);   // 设置合适的对比度
  u8g2.setBusClock(400000); // 设置I2C时钟为400kHz（需硬件支持）:ml-citation{ref="2,8" data="citationList"}
  u8g2.setContrast(255);    // 调整对比度（范围0-255）:ml-citation{ref="8" data="citationList"}
  // u8g2.setPowerSave(1) ;// 关闭OLED休眠

  showBootAnimation();
}

// 绘制信号强度条
void drawWifiIcon(int x, int y)
{

  if (!WiFi.isConnected())
  {
    // 显示 WiFi 断开连接的状态
    u8g2.drawDisc(x + 5, y + 5, 1); // 中心圆点
    return;
  }
  // strength: 信号强度等级（1-3）
  int rssi = WiFi.RSSI(); // 获取信号强度
  // Serial.println("WiFi 信号强度:" + rssi);
  // 默认 strength 为 0，防止数值为空
  int strength = 0;
  // 只有在有效 RSSI 范围内才计算 strength
  // 根据常见的WiFi信号强度划分标准进行调整
  if (rssi <= -90)
  {
    strength = 1; // 最低信号强度
  }
  else if (rssi <= -50)
  {
    strength = 2; // 中等信号强度
  }
  else
  {
    strength = 3; // 最高信号强度
  }

  // Serial.println("WiFi 信号强度 等级:" + String(strength) + " 信号强度" + rssi);

  u8g2.drawDisc(x + 5, y + 5, 1); // 中心圆点
  if (strength >= 3)
  {
    //  Serial.println("外层圆弧" + String(strength));
    u8g2.drawCircle(x + 8, y + 2, 7, U8G2_DRAW_UPPER_RIGHT); // 外层圆弧
  }

  if (strength >= 2)
  {
    u8g2.drawCircle(x + 7, y + 3, 5, U8G2_DRAW_UPPER_RIGHT); // 中层圆弧
    // Serial.println("中层圆弧" + String(strength));
  }

  if (strength >= 1)
  {
    u8g2.drawCircle(x + 6, y + 4, 3, U8G2_DRAW_UPPER_RIGHT); // 内层圆弧
    // Serial.println("内层圆弧" + String(strength));
  }
}

void drawUncheckedIcon()
{
  // 在这里绘制非选中图标的代码
  // u8g2.drawFrame(30, 5, 15, 15); // 边框
  u8g2.setFont(u8g2_font_wqy12_t_gb2312b); // 确保字体已设置

  u8g2.drawRFrame(30 + displayOffsetX, 5 + displayOffsetY, 15 + displayOffsetX, 15 + displayOffsetY, 2); // x, y, width, height, radius
  u8g2.setCursor(33 + displayOffsetX, 8 + displayOffsetY);
  u8g2.println(config.fanState ? "开" : "关"); // 风扇状态

  u8g2.drawRFrame(50 + displayOffsetX, 5 + displayOffsetY, 15 + displayOffsetX, 15 + displayOffsetY, 2);
  u8g2.setCursor(53 + displayOffsetX, 8 + displayOffsetY);
  u8g2.println(config.pumpState ? "开" : "关"); // 水泵状态

  u8g2.drawRFrame(70 + displayOffsetX, 5 + displayOffsetY, 15 + displayOffsetX, 15 + displayOffsetY, 2);
  u8g2.setCursor(73 + displayOffsetX, 8 + displayOffsetY);
  u8g2.println(config.lightState ? "开" : "关"); // 灯状态

  u8g2.drawRFrame(90 + displayOffsetX, 5 + displayOffsetY, 15 + displayOffsetX, 15 + displayOffsetY, 2);
  u8g2.setCursor(93 + displayOffsetX, 8 + displayOffsetY);
  u8g2.println(false ? "开" : "关"); // 灯状态

  u8g2.drawRFrame(110 + displayOffsetX, 5 + displayOffsetY, 15 + displayOffsetX, 15 + displayOffsetY, 2);
  u8g2.setCursor(113 + displayOffsetX, 8 + displayOffsetY);
  u8g2.println(config.manualMode ? "手" : "自"); // 模式
}

// 更新OLED显示
void updateOLED(float temperature, float soilMoisture, float dht11Temperature, float dht11SoilMoisture)
{
  // 每2秒更新一次OLED显示
  if (millis() - lastOLEDUpdateTime >= 2000)
  {
    // Serial.println("每2秒更新一次OLED显示");
    lastOLEDUpdateTime = millis();

    // 切换显示偏移以避免烧屏
    // displayOffsetX = (displayOffsetX + 1) % 2; // 循环0-1
    // displayOffsetY = (displayOffsetY + 1) % 2; // 循环0-1

    u8g2.clearBuffer();
    u8g2.enableUTF8Print();
    u8g2.setFontPosTop();
    u8g2.firstPage();
    do
    {
      // u8g2.drawRFrame(0, 0, 128, 128, 5); // 边框
      // // 顶部显示
      // drawWifiIcon(5, 10); // 动态绘制 wifi 图标

      u8g2.drawRFrame(displayOffsetX, displayOffsetY, 128 - displayOffsetX, 128 - displayOffsetY, 5); // 边框
      // 顶部显示
      drawWifiIcon(5 + displayOffsetX, 10 + displayOffsetY); // 动态绘制 wifi 图标

      drawUncheckedIcon();
      // 底部显示
      u8g2.setCursor(5 + displayOffsetX, 105 + displayOffsetY);


      if (WiFi.status() == WL_CONNECTED){
        IPAddress ip = WiFi.localIP();
        u8g2.println("ip: " + ip.toString());  
      } 
      else if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA) // 判断 ap 是否开启
      {
        Serial.println("AP已开启"); 
        u8g2.println("ip: 192.168.4.1"  );  
        u8g2.print(" ap: " + String(WiFi.softAPgetStationNum()));
      } else {
        u8g2.println("wifi 未连接,热点已关闭");
      }

      // 中间显示
      u8g2.setFont(u8g2_font_wqy16_t_gb2312b);
      u8g2.setCursor(5 + displayOffsetX, 25 + displayOffsetY);
      u8g2.println("大棚温度:" + String(temperature, 1) + "°C");
      u8g2.setCursor(5 + displayOffsetX, 45 + displayOffsetY);
      u8g2.println("土壤湿度:" + String(soilMoisture, 1) + "%");
      u8g2.setCursor(5 + displayOffsetX, 65 + displayOffsetY);
      u8g2.println("空气温度:" + String(dht11Temperature, 1) + "%");
      u8g2.setCursor(5 + displayOffsetX, 85 + displayOffsetY);
      u8g2.println("空气湿度:" + String(dht11SoilMoisture, 1) + "%");
    } while (u8g2.nextPage());
  }
}

// 设置亮度级别
void setBrightness(uint8_t level)
{
  u8g2.setContrast(level); // 设置对比度级别 (0-255)
}

// 显示升级
void showUpdate(String ota_version)
{
  u8g2.clearBuffer();
  u8g2.enableUTF8Print();
  u8g2.setFont(u8g2_font_wqy16_t_gb2312b);
  u8g2.firstPage();
  do
  {
    u8g2.setCursor(0, 15);
    u8g2.println("正在升级至" + ota_version);
    u8g2.setCursor(0, 32);
    u8g2.println("请勿断开电源");
    u8g2.setCursor(0, 49);
    u8g2.println("或重启...");
  } while (u8g2.nextPage());
}
