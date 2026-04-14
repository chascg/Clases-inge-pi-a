// lab05_dht22.ino - Prueba del sensor DHT22
#include "dht22.h"
#include "hal_gpio.h"

DHT22_Data_t sensor_dht22;
const int LED_STATUS = 13;
bool led_state = false;

void setup() {
    Serial.begin(9600);
    pinMode(LED_STATUS, OUTPUT);
    
    Serial.println(F("=== Laboratorio 05 - Driver DHT22 ==="));
    Serial.println();
    
    // Inicializar DHT22 en pin 8
    Serial.print(F("DHT22 Init (pin 8): "));
    DHT22_Status s = DHT22_Init(8);
    Serial.println(DHT22_StatusStr(s));
    
    if (s != DHT22_OK) {
        Serial.println(F("⚠️  Verifica las conexiones del DHT22"));
        Serial.println(F("   - VCC a 5V"));
        Serial.println(F("   - DATA a pin 8 con resistencia 10kΩ a 5V"));
        Serial.println(F("   - GND a GND"));
    }
    
    Serial.println(F("\n--- Leyendo DHT22 cada 2 segundos ---"));
}

void loop() {
    // Parpadeo LED indicador de actividad
    led_state = !led_state;
    digitalWrite(LED_STATUS, led_state);
    
    // Leer DHT22
    Serial.println(F("\n[DHT22]"));
    DHT22_Status s = DHT22_Read(&sensor_dht22);
    
    if (s == DHT22_OK) {
        Serial.print(F("  ✅ Temperatura: "));
        Serial.print(sensor_dht22.temp_c, 1);
        Serial.println(F(" °C"));
        
        Serial.print(F("  💧 Humedad: "));
        Serial.print(sensor_dht22.humidity, 1);
        Serial.println(F(" %"));
        
        Serial.print(F("  ⏱️  Timestamp: "));
        Serial.println(sensor_dht22.timestamp);
    } else {
        Serial.print(F("  ❌ "));
        Serial.println(DHT22_StatusStr(s));
    }
    
    delay(2000);  // Leer cada 2 segundos
}