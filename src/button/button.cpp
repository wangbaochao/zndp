#include "button.h"
#include <Arduino.h>


// 定义按钮引脚
#define BUTTON_PIN 2

// 定义按钮状态
bool buttonState = false;

void buttonInit() {
  // 初始化按钮引脚为输入模式
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

// 读取按钮状态
bool readButtonState() {
  return digitalRead(BUTTON_PIN);
}

// 检查按钮是否被按下
bool isButtonPressed() {
  bool currentState = readButtonState();
  if (currentState != buttonState) {
    buttonState = currentState;
    return !buttonState; // 返回按钮是否被按下（低电平为按下）
  }
  return false;
}

// 长按
bool isLongPress() {
    static unsigned long lastPressTime = 0;
    static bool isLongPress = false;

    unsigned long currentTime = millis();

    if (isButtonPressed()) {
        if (currentTime - lastPressTime >= 1000) {
            isLongPress = true;
        }
    } else {
        isLongPress = false;
        lastPressTime = currentTime;
    }

    return isLongPress;
}