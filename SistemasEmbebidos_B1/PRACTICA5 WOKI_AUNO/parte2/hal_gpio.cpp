// hal_gpio.cpp - Implementacion para Arduino/AVR
#include "hal_gpio.h"
#include <Arduino.h>

void HAL_GPIO_SetDir(uint8_t pin, HAL_GPIO_Dir dir) {
    pinMode(pin, dir == HAL_GPIO_OUTPUT ? OUTPUT : INPUT);
}

void HAL_GPIO_Write(uint8_t pin, HAL_GPIO_Level level) {
    digitalWrite(pin, level == HAL_GPIO_HIGH ? HIGH : LOW);
}

HAL_GPIO_Level HAL_GPIO_Read(uint8_t pin) {
    return digitalRead(pin) == HIGH ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
}

void HAL_DelayUs(uint16_t us) {
    delayMicroseconds(us);
}

void HAL_DelayMs(uint16_t ms) {
    delay(ms);
}