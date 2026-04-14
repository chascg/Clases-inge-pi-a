// dht22.cpp - Implementacion del driver DHT22
#include "dht22.h"
#include "hal_gpio.h"
#include <Arduino.h>

static uint8_t _pin;
static uint8_t rawData[5];  // Cambiado de 'data' a 'rawData' para evitar conflicto

// Esperar hasta que el pin tenga cierto nivel, con timeout
static bool wait_for_level(HAL_GPIO_Level level, uint16_t timeout_us) {
    uint16_t count = 0;
    while (HAL_GPIO_Read(_pin) != level) {
        HAL_DelayUs(1);
        count++;
        if (count > timeout_us) return false;
    }
    return true;
}

DHT22_Status DHT22_Init(uint8_t pin) {
    _pin = pin;
    HAL_GPIO_SetDir(_pin, HAL_GPIO_INPUT);
    HAL_GPIO_Write(_pin, HAL_GPIO_HIGH);
    HAL_DelayMs(1000);
    
    DHT22_Data_t dummy;
    return DHT22_Read(&dummy);
}

DHT22_Status DHT22_Read(DHT22_Data_t *sensorData) {
    sensorData->valido = false;
    
    // Paso 1: Enviar señal de start
    HAL_GPIO_SetDir(_pin, HAL_GPIO_OUTPUT);
    HAL_GPIO_Write(_pin, HAL_GPIO_LOW);
    HAL_DelayMs(1);
    
    HAL_GPIO_Write(_pin, HAL_GPIO_HIGH);
    HAL_DelayUs(30);
    
    // Paso 2: Cambiar a modo lectura
    HAL_GPIO_SetDir(_pin, HAL_GPIO_INPUT);
    
    // Paso 3: Esperar respuesta del sensor
    if (!wait_for_level(HAL_GPIO_LOW, 100)) {
        return DHT22_NO_RESPONSE;
    }
    if (!wait_for_level(HAL_GPIO_HIGH, 100)) {
        return DHT22_NO_RESPONSE;
    }
    if (!wait_for_level(HAL_GPIO_LOW, 100)) {
        return DHT22_NO_RESPONSE;
    }
    
    // Paso 4: Leer 40 bits (5 bytes)
    for (uint8_t i = 0; i < 5; i++) {
        rawData[i] = 0;
        for (uint8_t j = 0; j < 8; j++) {
            if (!wait_for_level(HAL_GPIO_HIGH, 100)) {
                return DHT22_TIMEOUT;
            }
            
            uint16_t count = 0;
            while (HAL_GPIO_Read(_pin) == HAL_GPIO_HIGH) {
                HAL_DelayUs(1);
                count++;
                if (count > 100) break;
            }
            
            rawData[i] <<= 1;
            if (count > 40) {
                rawData[i] |= 1;
            }
        }
    }
    
    // Paso 5: Verificar checksum
    uint8_t checksum = rawData[0] + rawData[1] + rawData[2] + rawData[3];
    if (checksum != rawData[4]) {
        return DHT22_CHECKSUM_ERR;
    }
    
    // Paso 6: Calcular valores
    uint16_t raw_humidity = (rawData[0] << 8) | rawData[1];
    uint16_t raw_temp = (rawData[2] << 8) | rawData[3];
    
    float temp = raw_temp;
    if (raw_temp & 0x8000) {
        temp = -(raw_temp & 0x7FFF);
    }
    
    sensorData->humidity = raw_humidity / 10.0f;
    sensorData->temp_c = temp / 10.0f;
    sensorData->valido = true;
    sensorData->timestamp = millis();
    
    return DHT22_OK;
}

const char* DHT22_StatusStr(DHT22_Status s) {
    switch(s) {
        case DHT22_OK:            return "OK";
        case DHT22_TIMEOUT:       return "ERROR: Timeout en comunicacion";
        case DHT22_CHECKSUM_ERR:  return "ERROR: Checksum invalido";
        case DHT22_NO_RESPONSE:   return "ERROR: Sensor no responde";
        default:                  return "ERROR: Desconocido";
    }
}