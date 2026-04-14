// dht22.h - Interfaz del driver DHT22
#ifndef DHT22_H
#define DHT22_H

#include <stdint.h>
#include <stdbool.h>

// Codigos de error
typedef enum {
    DHT22_OK = 0,
    DHT22_TIMEOUT = -1,
    DHT22_CHECKSUM_ERR = -2,
    DHT22_NO_RESPONSE = -3
} DHT22_Status;

// Estructura de datos del sensor
typedef struct {
    float temp_c;
    float humidity;
    bool valido;
    uint32_t timestamp;
} DHT22_Data_t;

// API publica del driver
DHT22_Status DHT22_Init(uint8_t pin);
DHT22_Status DHT22_Read(DHT22_Data_t *data);
const char* DHT22_StatusStr(DHT22_Status s);

#endif