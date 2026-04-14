#ifndef HAL_GPIO_H
#define HAL_GPIO_H
#include <stdint.h>

typedef enum { 
    HAL_GPIO_INPUT = 0, 
    HAL_GPIO_OUTPUT = 1 
} HAL_GPIO_Dir;

typedef enum { 
    HAL_GPIO_LOW = 0, 
    HAL_GPIO_HIGH = 1 
} HAL_GPIO_Level;

void HAL_GPIO_SetDir(uint8_t pin, HAL_GPIO_Dir dir);
void HAL_GPIO_Write(uint8_t pin, HAL_GPIO_Level level);
HAL_GPIO_Level HAL_GPIO_Read(uint8_t pin);
void HAL_DelayUs(uint16_t us);
void HAL_DelayMs(uint16_t ms);

#endif