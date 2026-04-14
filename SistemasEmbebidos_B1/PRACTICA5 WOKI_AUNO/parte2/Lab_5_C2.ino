// Lab_5_C2.ino - Laboratorio 05: Drivers y Arquitectura por Capas
// Version simplificada para Wokwi

#include <Arduino.h>
#include "hal_gpio.h"
#include "ds18b20.h"
#include "dht22.h"

DS18B20_Data_t sensor_ds18b20;
DHT22_Data_t sensor_dht22;

const int LED_STATUS = 13;
bool led_state = false;

// Contadores para no saturar el Serial con errores
int dht22_errors = 0;
int ds18b20_errors = 0;

void setup() {
    Serial.begin(9600);
    delay(1000);
    
    pinMode(LED_STATUS, OUTPUT);
    digitalWrite(LED_STATUS, LOW);
    
    Serial.println("Laboratorio 05 - Drivers y Arquitectura por Capas");
    Serial.println("================================================");
    
    // Inicializar DS18B20
    Serial.print("DS18B20 (pin 7): ");
    DS18B20_Status s1 = DS18B20_Init(7);
    Serial.println(DS18B20_StatusStr(s1));
    
    // Inicializar DHT22
    Serial.print("DHT22 (pin 8): ");
    DHT22_Status s2 = DHT22_Init(8);
    Serial.println(DHT22_StatusStr(s2));
    
    Serial.println("\nLecturas de sensores:\n");
}

void loop() {
    led_state = !led_state;
    digitalWrite(LED_STATUS, led_state);
    
    // ========== LECTURA DS18B20 ==========
    DS18B20_Status s1 = DS18B20_Read(&sensor_ds18b20);
    
    Serial.print("DS18B20 -> ");
    if (s1 == DS18B20_OK) {
        Serial.print("Temp: ");
        Serial.print(sensor_ds18b20.temp_c, 2);
        Serial.print(" C");
        ds18b20_errors = 0;
    } else {
        ds18b20_errors++;
        Serial.print("ERROR (");
        Serial.print(ds18b20_errors);
        Serial.print(")");
    }
    
    Serial.print("  |  ");
    
    // ========== LECTURA DHT22 ==========
    DHT22_Status s2 = DHT22_Read(&sensor_dht22);
    
    Serial.print("DHT22 -> ");
    if (s2 == DHT22_OK) {
        Serial.print("Temp: ");
        Serial.print(sensor_dht22.temp_c, 2);
        Serial.print(" C  Hum: ");
        Serial.print(sensor_dht22.humidity, 1);
        Serial.print(" %");
        dht22_errors = 0;
    } else {
        dht22_errors++;
        Serial.print("ERROR (");
        Serial.print(dht22_errors);
        Serial.print(")");
    }
    
    Serial.println();
    
    delay(2000);
}