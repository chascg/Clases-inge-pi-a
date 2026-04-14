// ds18b20.h - Interfaz del driver DS18B20
#ifndef DS18B20_H
#define DS18B20_H

#include <stdint.h>
#include <stdbool.h>

// Codigos de error
typedef enum {
    DS18B20_OK = 0,
    DS18B20_NO_DEV = -1,
    DS18B20_CRC_ERR = -2,
    DS18B20_BUSY = -3
} DS18B20_Status;

// Estructura de datos del sensor
typedef struct {
    float temp_c;
    bool valido;
    uint32_t timestamp;
} DS18B20_Data_t;

// API publica del driver
DS18B20_Status DS18B20_Init(uint8_t pin);
DS18B20_Status DS18B20_Read(DS18B20_Data_t *data);
const char* DS18B20_StatusStr(DS18B20_Status s);

#endif