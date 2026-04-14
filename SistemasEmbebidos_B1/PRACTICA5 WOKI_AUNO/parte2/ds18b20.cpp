// ds18b20.cpp - Implementacion del protocolo 1-Wire
#include "ds18b20.h"
#include "hal_gpio.h"
#include <Arduino.h>

static uint8_t _pin;

// Enviar pulso de reset y detectar presencia
static bool onewire_reset() {
    HAL_GPIO_SetDir(_pin, HAL_GPIO_OUTPUT);
    HAL_GPIO_Write(_pin, HAL_GPIO_LOW);
    HAL_DelayUs(480);
    HAL_GPIO_SetDir(_pin, HAL_GPIO_INPUT);
    HAL_DelayUs(70);
    bool presencia = (HAL_GPIO_Read(_pin) == HAL_GPIO_LOW);
    HAL_DelayUs(410);
    return presencia;
}

// Escribir un bit en el bus 1-Wire
static void onewire_write_bit(bool bit) {
    HAL_GPIO_SetDir(_pin, HAL_GPIO_OUTPUT);
    HAL_GPIO_Write(_pin, HAL_GPIO_LOW);
    
    if (bit) {
        HAL_DelayUs(6);
        HAL_GPIO_SetDir(_pin, HAL_GPIO_INPUT);
        HAL_DelayUs(64);
    } else {
        HAL_DelayUs(60);
        HAL_GPIO_SetDir(_pin, HAL_GPIO_INPUT);
        HAL_DelayUs(10);
    }
}

// Escribir un byte en el bus 1-Wire
static void onewire_write_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        onewire_write_bit(byte & 0x01);
        byte >>= 1;
    }
}

// Leer un bit del bus 1-Wire
static bool onewire_read_bit() {
    HAL_GPIO_SetDir(_pin, HAL_GPIO_OUTPUT);
    HAL_GPIO_Write(_pin, HAL_GPIO_LOW);
    HAL_DelayUs(3);
    HAL_GPIO_SetDir(_pin, HAL_GPIO_INPUT);
    HAL_DelayUs(10);
    bool bit = (HAL_GPIO_Read(_pin) == HAL_GPIO_HIGH);
    HAL_DelayUs(53);
    return bit;
}

// Leer un byte del bus 1-Wire
static uint8_t onewire_read_byte() {
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (onewire_read_bit()) {
            byte |= (1 << i);
        }
    }
    return byte;
}

DS18B20_Status DS18B20_Init(uint8_t pin) {
    _pin = pin;
    HAL_GPIO_SetDir(_pin, HAL_GPIO_INPUT);
    return onewire_reset() ? DS18B20_OK : DS18B20_NO_DEV;
}

DS18B20_Status DS18B20_Read(DS18B20_Data_t *data) {
    data->valido = false;
    
    if (!onewire_reset()) return DS18B20_NO_DEV;
    
    onewire_write_byte(0xCC);
    onewire_write_byte(0x44);
    
    HAL_DelayMs(750);
    
    if (!onewire_reset()) return DS18B20_NO_DEV;
    
    onewire_write_byte(0xCC);
    onewire_write_byte(0xBE);
    
    uint8_t b0 = onewire_read_byte();
    uint8_t b1 = onewire_read_byte();
    
    int16_t raw = (int16_t)((b1 << 8) | b0);
    data->temp_c = raw / 16.0f;
    data->valido = true;
    data->timestamp = millis();
    
    return DS18B20_OK;
}

const char* DS18B20_StatusStr(DS18B20_Status s) {
    switch(s) {
        case DS18B20_OK:       return "OK";
        case DS18B20_NO_DEV:   return "ERROR: Sensor no detectado";
        case DS18B20_CRC_ERR:  return "ERROR: CRC invalido";
        case DS18B20_BUSY:     return "ERROR: Conversion en progreso";
        default:               return "ERROR: Desconocido";
    }
}