// Lab09_Queue_Productor_Simple.ino
// FreeRTOS Queue - Productor Consumidor (Versión para ESP32)

#include "DHT.h"

// ========== CONFIGURACIÓN ==========
#define PIN_DHT 4
#define PIN_LED_R 25
#define PIN_LED_G 26
#define PIN_LED_B 27

DHT dht(PIN_DHT, DHT22);

// ========== ESTRUCTURA PARA LA COLA ==========
struct DatoSensor {
    float temperatura;
    float humedad;
    unsigned long timestamp;
};

// ========== HANDLES ==========
QueueHandle_t colaDatos = NULL;

// ========== CONTADORES ==========
int producidos = 0;
int consumidos1 = 0;
int consumidos2 = 0;
int perdidos = 0;

// ========== FUNCIONES LED ==========
void setRGB(int r, int g, int b) {
    analogWrite(PIN_LED_R, r);
    analogWrite(PIN_LED_G, g);
    analogWrite(PIN_LED_B, b);
}

void ledVerde() {
    setRGB(0, 255, 0);
    delay(50);
    setRGB(0, 0, 0);
}

void ledAzul() {
    setRGB(0, 0, 255);
    delay(30);
    setRGB(0, 0, 0);
}

void ledRojo() {
    setRGB(255, 0, 0);
    delay(200);
    setRGB(0, 0, 0);
}

// ========== TAREA PRODUCTOR ==========
void tareaProductor(void *parameter) {
    DatoSensor dato;
    
    while (true) {
        // Leer sensor
        dato.temperatura = dht.readTemperature();
        dato.humedad = dht.readHumidity();
        dato.timestamp = millis();
        
        if (!isnan(dato.temperatura) && !isnan(dato.humedad)) {
            // Enviar a la cola (sin esperar si está llena)
            if (xQueueSend(colaDatos, &dato, 0) == pdTRUE) {
                producidos++;
                ledVerde();
                Serial.print("[PRODUCTOR] #");
                Serial.print(producidos);
                Serial.print(": T=");
                Serial.print(dato.temperatura);
                Serial.print(" H=");
                Serial.println(dato.humedad);
            } else {
                perdidos++;
                ledRojo();
                Serial.print("[PRODUCTOR] COLA LLENA! Perdido #");
                Serial.println(perdidos);
            }
        } else {
            Serial.println("[PRODUCTOR] Error leyendo DHT22");
            ledRojo();
        }
        
        // Esperar 3 segundos
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

// ========== TAREA CONSUMIDOR 1 (Serial) ==========
void tareaConsumidor1(void *parameter) {
    DatoSensor dato;
    
    while (true) {
        // Esperar dato (bloqueante)
        if (xQueueReceive(colaDatos, &dato, portMAX_DELAY) == pdTRUE) {
            consumidos1++;
            ledAzul();
            
            Serial.println("\n=== CONSUMIDOR 1 (SERIAL) ===");
            Serial.print("Temperatura: "); Serial.print(dato.temperatura); Serial.println(" °C");
            Serial.print("Humedad: "); Serial.print(dato.humedad); Serial.println(" %");
            Serial.print("Timestamp: "); Serial.println(dato.timestamp);
            Serial.print("Datos en cola: "); Serial.println(uxQueueMessagesWaiting(colaDatos));
            Serial.print("Consumidos: "); Serial.println(consumidos1);
            Serial.println("==============================\n");
        }
    }
}

// ========== TAREA CONSUMIDOR 2 (Procesamiento) ==========
void tareaConsumidor2(void *parameter) {
    DatoSensor dato;
    
    while (true) {
        // Esperar dato con timeout de 5 segundos
        if (xQueueReceive(colaDatos, &dato, 5000 / portTICK_PERIOD_MS) == pdTRUE) {
            consumidos2++;
            
            Serial.print("[CONSUMIDOR 2] #");
            Serial.print(consumidos2);
            
            // Procesar alertas
            if (dato.temperatura > 30) {
                Serial.println(": ⚠️ ALERTA! Temperatura alta!");
                setRGB(255, 100, 0);  // Naranja
                delay(500);
                setRGB(0, 0, 0);
            } else if (dato.humedad < 40) {
                Serial.println(": ⚠️ ALERTA! Humedad baja!");
                setRGB(0, 255, 255);  // Cyan
                delay(500);
                setRGB(0, 0, 0);
            } else {
                Serial.println(": ✅ Condiciones normales");
            }
        } else {
            Serial.println("[CONSUMIDOR 2] Timeout - No hay datos");
        }
    }
}

// ========== TAREA MONITOR ==========
void tareaMonitor(void *parameter) {
    while (true) {
        Serial.println("\n╔══════════════════════════════╗");
        Serial.println("║     MONITOR DE SISTEMA       ║");
        Serial.println("╠══════════════════════════════╣");
        Serial.print("║ Elementos en cola: ");
        Serial.print(uxQueueMessagesWaiting(colaDatos));
        Serial.println("/5");
        Serial.print("║ Producidos: "); Serial.println(producidos);
        Serial.print("║ Consumidos (Serial): "); Serial.println(consumidos1);
        Serial.print("║ Consumidos (Proceso): "); Serial.println(consumidos2);
        Serial.print("║ Perdidos: "); Serial.println(perdidos);
        Serial.print("║ Heap libre: ");
        Serial.print(ESP.getFreeHeap());
        Serial.println(" bytes");
        Serial.println("╚══════════════════════════════╝\n");
        
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== QUEUE PRODUCTOR-CONSUMIDOR ===");
    Serial.println("ESP32 - FreeRTOS Multitarea");
    
    // Configurar pines LED
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    setRGB(0, 0, 0);
    
    dht.begin();
    
    // Crear cola con capacidad para 5 elementos
    colaDatos = xQueueCreate(5, sizeof(DatoSensor));
    
    if (colaDatos == NULL) {
        Serial.println("ERROR: No se pudo crear la cola");
        while (true) {}
    }
    
    Serial.println("Cola creada (capacidad: 5 elementos)");
    
    // Crear tareas (FreeRTOS)
    xTaskCreate(tareaProductor, "Productor", 4096, NULL, 3, NULL);
    xTaskCreate(tareaConsumidor1, "Consumidor1", 4096, NULL, 2, NULL);
    xTaskCreate(tareaConsumidor2, "Consumidor2", 4096, NULL, 2, NULL);
    xTaskCreate(tareaMonitor, "Monitor", 2048, NULL, 1, NULL);
    
    Serial.println("Tareas creadas:");
    Serial.println("  - Productor (prioridad 3) - Lee DHT22 cada 3s");
    Serial.println("  - Consumidor 1 (prioridad 2) - Muestra en Serial");
    Serial.println("  - Consumidor 2 (prioridad 2) - Procesa alertas");
    Serial.println("  - Monitor (prioridad 1) - Estadísticas cada 10s");
    Serial.println("\nSistema iniciado!\n");
}

void loop() {
    // El loop está vacío porque FreeRTOS maneja todo
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}