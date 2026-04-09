
#include <Arduino.h>
#include <DHTesp.h>

// Definición de pines
#define PIN_DHT22   13
#define PIN_LED     2   // Opcional, podés omitir

// Crear objeto del sensor
DHTesp dhtSensor;

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println(F("\n╔════════════════════════════════╗"));
    Serial.println(F("║   LECTOR DE DHT22 - ESP32     ║"));
    Serial.println(F("╚════════════════════════════════╝\n"));
    
    // Inicializar LED opcional
    pinMode(PIN_LED, OUTPUT);
    
    // Inicializar sensor DHT22
    dhtSensor.setup(PIN_DHT22, DHTesp::DHT22);
    
    // Verificar si el sensor se inicializó correctamente
    if (dhtSensor.getStatus() == DHTesp::ERROR_NONE) {
        Serial.println("[OK] Sensor DHT22 inicializado correctamente");
    } else {
        Serial.println("[ERROR] No se pudo inicializar el DHT22");
        Serial.println("        Verificar conexiones y resistencia pull-up");
    }
    
    Serial.println(F("\nIniciando lecturas cada 2 segundos...\n"));
    Serial.println(F("═══════════════════════════════════════════"));
}

void loop() {
    // Esperar 2 segundos entre lecturas
    delay(2000);
    
    // Leer temperatura y humedad
    float temperatura = dhtSensor.getTemperature();
    float humedad = dhtSensor.getHumidity();
    
    // Parpadear LED para indicar lectura (opcional)
    digitalWrite(PIN_LED, HIGH);
    delay(50);
    digitalWrite(PIN_LED, LOW);
    
    // Verificar si las lecturas son válidas
    if (isnan(temperatura) || isnan(humedad)) {
        Serial.println("[ERROR] Lectura fallida");
        Serial.println("        ¿Pusiste la resistencia de 4.7kΩ a 10kΩ?");
        return;
    }
    
    // Mostrar resultados en pantalla
    Serial.println(F("┌─────────────────────────────────┐"));
    Serial.print  (F("│ 🌡️  Temperatura: "));
    Serial.print(temperatura);
    Serial.println(F(" °C     │"));
    
    Serial.print  (F("│ 💧 Humedad:      "));
    Serial.print(humedad);
    Serial.println(F(" %       │"));
    
    // Calcular punto de rocío aproximado
    float puntoRocio = calcularPuntoRocio(temperatura, humedad);
    Serial.print  (F("│ 💨 Punto de rocío: "));
    Serial.print(puntoRocio);
    Serial.println(F(" °C   │"));
    
    Serial.println(F("└─────────────────────────────────┘"));
}

// Función para calcular punto de rocío aproximado
float calcularPuntoRocio(float temp, float humedad) {
    // Fórmula aproximada de Magnus
    float a = 17.27;
    float b = 237.7;
    float alpha = log(humedad / 100.0) + (a * temp) / (b + temp);
    return (b * alpha) / (a - alpha);
}