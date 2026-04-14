// Lab09_Integrador_RTOS_Simple.ino
// Proyecto Integrador FreeRTOS - Versión Simplificada

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "DHT.h"

// ========== CONFIGURACIÓN ==========
#define PIN_DHT 4
#define PIN_LED_R 25
#define PIN_LED_G 26
#define PIN_LED_B 27
#define PIN_BTN1 15
#define PIN_BTN2 16

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

DHT dht(PIN_DHT, DHT22);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ========== ESTRUCTURA DE DATOS ==========
struct DatoSensor {
    float temperatura;
    float humedad;
    unsigned long timestamp;
};

// ========== HANDLES RTOS ==========
QueueHandle_t colaDatos = NULL;
SemaphoreHandle_t mutexI2C = NULL;
SemaphoreHandle_t boton1Sem = NULL;
SemaphoreHandle_t boton2Sem = NULL;

// ========== VARIABLES ==========
bool sistemaActivo = true;
bool modoAutomatico = true;
int lecturas = 0;
int publicaciones = 0;
DatoSensor ultimoDato;
unsigned long tiempoInicio = 0;

// ========== FUNCIONES LED ==========
void setRGB(int r, int g, int b) {
    analogWrite(PIN_LED_R, r);
    analogWrite(PIN_LED_G, g);
    analogWrite(PIN_LED_B, b);
}

void ledVerde() { setRGB(0, 255, 0); delay(50); setRGB(0, 0, 0); }
void ledAzul() { setRGB(0, 0, 255); delay(100); setRGB(0, 0, 0); }
void ledRojoPausa() { setRGB(255, 0, 0); delay(500); setRGB(0, 0, 0); }

// ========== OLED SEGURO (CON MUTEX) ==========
void oledEscribir(const char* l1, const char* l2, const char* l3, const char* l4) {
    if (xSemaphoreTake(mutexI2C, pdMS_TO_TICKS(100)) == pdTRUE) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        
        display.setCursor(0, 0);   display.print(l1);
        display.setCursor(0, 16);  display.print(l2);
        display.setCursor(0, 32);  display.print(l3);
        display.setCursor(0, 48);  display.print(l4);
        
        display.display();
        xSemaphoreGive(mutexI2C);
    }
}

// ========== PUBLICACIÓN MQTT SIMULADA ==========
void publicarMQTT(DatoSensor dato) {
    publicaciones++;
    
    Serial.println("\n=== PUBLICACIÓN MQTT ===");
    Serial.printf("Temperatura: %.1f C\n", dato.temperatura);
    Serial.printf("Humedad: %.1f %%\n", dato.humedad);
    Serial.printf("Uptime: %lu s\n", dato.timestamp / 1000);
    Serial.printf("Publicación #%d\n", publicaciones);
    Serial.println("=======================\n");
    
    ledAzul();
}

// ========== ISR BOTONES ==========
void IRAM_ATTR isrBoton1() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(boton1Sem, &xHigherPriorityTaskWoken);
}

void IRAM_ATTR isrBoton2() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(boton2Sem, &xHigherPriorityTaskWoken);
}

// ========== TAREA SENSOR (PRODUCTOR) ==========
void tareaSensor(void *parameter) {
    DatoSensor dato;
    
    while (true) {
        if (sistemaActivo) {
            dato.temperatura = dht.readTemperature();
            dato.humedad = dht.readHumidity();
            dato.timestamp = millis();
            
            if (!isnan(dato.temperatura) && !isnan(dato.humedad)) {
                ultimoDato = dato;
                lecturas++;
                
                if (xQueueSend(colaDatos, &dato, 0) == pdTRUE) {
                    ledVerde();
                    Serial.printf("[SENSOR] #%d: T=%.1f H=%.1f\n", 
                        lecturas, dato.temperatura, dato.humedad);
                } else {
                    Serial.println("[SENSOR] COLA LLENA!");
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

// ========== TAREA DISPLAY ==========
void tareaDisplay(void *parameter) {
    DatoSensor dato;
    char l1[20], l2[20], l3[20], l4[20];
    
    while (true) {
        if (xQueuePeek(colaDatos, &dato, pdMS_TO_TICKS(1000)) == pdTRUE) {
            snprintf(l1, sizeof(l1), "RTOS INTEGRADOR");
            snprintf(l2, sizeof(l2), "T:%.1fC H:%.1f%%", dato.temperatura, dato.humedad);
            snprintf(l3, sizeof(l3), "Pub:%d Lec:%d", publicaciones, lecturas);
            snprintf(l4, sizeof(l4), "%s %s", 
                sistemaActivo ? "ACTIVO" : "PAUSADO",
                modoAutomatico ? "AUTO" : "MANUAL");
            
            oledEscribir(l1, l2, l3, l4);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// ========== TAREA MQTT ==========
void tareaMQTT(void *parameter) {
    DatoSensor dato;
    unsigned long lastPub = 0;
    
    Serial.println("[MQTT] Conectando a WiFi...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("[MQTT] WiFi conectado! IP: 192.168.1.150");
    Serial.println("[MQTT] Conectando a broker...");
    vTaskDelay(pdMS_TO_TICKS(500));
    Serial.println("[MQTT] Conectado!\n");
    
    while (true) {
        unsigned long ahora = millis();
        if (sistemaActivo && (ahora - lastPub >= 10000)) {
            lastPub = ahora;
            if (xQueuePeek(colaDatos, &dato, 0) == pdTRUE) {
                publicarMQTT(dato);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ========== TAREA BOTONES ==========
void tareaBotones(void *parameter) {
    unsigned long lastPress1 = 0, lastPress2 = 0;
    
    while (true) {
        if (xSemaphoreTake(boton1Sem, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (millis() - lastPress1 > 200) {
                lastPress1 = millis();
                sistemaActivo = !sistemaActivo;
                Serial.printf("[BOTON 1] Sistema: %s\n", sistemaActivo ? "ACTIVO" : "PAUSADO");
                if (!sistemaActivo) ledRojoPausa();
            }
        }
        
        if (xSemaphoreTake(boton2Sem, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (millis() - lastPress2 > 200) {
                lastPress2 = millis();
                modoAutomatico = !modoAutomatico;
                Serial.printf("[BOTON 2] Modo: %s\n", modoAutomatico ? "AUTOMATICO" : "MANUAL");
            }
        }
    }
}

// ========== TAREA MONITOR ==========
void tareaMonitor(void *parameter) {
    while (true) {
        Serial.println("\n╔══════════════════════════════════╗");
        Serial.println("║     MONITOR DEL SISTEMA         ║");
        Serial.println("╠══════════════════════════════════╣");
        Serial.printf("║ Cola: %d/5 elementos\n", uxQueueMessagesWaiting(colaDatos));
        Serial.printf("║ Lecturas: %d\n", lecturas);
        Serial.printf("║ Publicaciones: %d\n", publicaciones);
        Serial.printf("║ Sistema: %s\n", sistemaActivo ? "ACTIVO" : "PAUSADO");
        Serial.printf("║ Modo: %s\n", modoAutomatico ? "AUTO" : "MANUAL");
        Serial.printf("║ Heap libre: %d bytes\n", ESP.getFreeHeap());
        Serial.println("╚══════════════════════════════════╝\n");
        
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    delay(1000);
    tiempoInicio = millis();
    
    Serial.println("\n=== PROYECTO INTEGRADOR RTOS ===");
    Serial.println("Queue + Mutex + OLED + MQTT + Botones\n");
    
    // Configurar pines
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    pinMode(PIN_BTN1, INPUT_PULLUP);
    pinMode(PIN_BTN2, INPUT_PULLUP);
    
    setRGB(0, 0, 0);
    dht.begin();
    Wire.begin();
    
    // Inicializar OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("ERROR: OLED no encontrada!");
        while (true) { delay(1000); }
    }
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 25);
    display.println("RTOS");
    display.display();
    delay(2000);
    
    // Crear objetos RTOS
    colaDatos = xQueueCreate(5, sizeof(DatoSensor));
    mutexI2C = xSemaphoreCreateMutex();
    boton1Sem = xSemaphoreCreateBinary();
    boton2Sem = xSemaphoreCreateBinary();
    
    if (!colaDatos || !mutexI2C || !boton1Sem || !boton2Sem) {
        Serial.println("ERROR: No se crearon objetos RTOS!");
        while (true) {}
    }
    
    // Crear tareas
    xTaskCreate(tareaSensor, "Sensor", 4096, NULL, 3, NULL);
    xTaskCreate(tareaDisplay, "Display", 4096, NULL, 2, NULL);
    xTaskCreate(tareaMQTT, "MQTT", 4096, NULL, 2, NULL);
    xTaskCreate(tareaBotones, "Botones", 2048, NULL, 1, NULL);
    xTaskCreate(tareaMonitor, "Monitor", 2048, NULL, 1, NULL);
    
    // Configurar interrupciones
    attachInterrupt(digitalPinToInterrupt(PIN_BTN1), isrBoton1, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN2), isrBoton2, FALLING);
    
    Serial.println("✅ Tareas creadas:");
    Serial.println("   • Sensor (Productor) - Cada 3s");
    Serial.println("   • Display (Consumidor) - Cada 1s");
    Serial.println("   • MQTT (Consumidor) - Cada 10s");
    Serial.println("   • Botones (ISR) - Interrupciones");
    Serial.println("   • Monitor - Cada 10s\n");
    
    Serial.println("🎮 CONTROLES:");
    Serial.println("   Botón GPIO15 = Pausar/Reanudar sistema");
    Serial.println("   Botón GPIO16 = Cambiar modo AUTO/MANUAL\n");
    Serial.println("🚀 Sistema iniciado!\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(10000));
}