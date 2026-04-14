// Lab09_Mutex_I2C_Simple.ino
// FreeRTOS Mutex para protección I2C (Versión simplificada)

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "DHT.h"

// ========== CONFIGURACIÓN ==========
#define PIN_DHT 4
#define PIN_LED_R 25
#define PIN_LED_G 26
#define PIN_LED_B 27

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

DHT dht(PIN_DHT, DHT22);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ========== HANDLES RTOS ==========
SemaphoreHandle_t mutexI2C = NULL;

// ========== VARIABLES ==========
float temperatura = 0;
float humedad = 0;
int accesos1 = 0, accesos2 = 0, accesos3 = 0;
int timeouts = 0;

// ========== FUNCIONES LED ==========
void setRGB(int r, int g, int b) {
    analogWrite(PIN_LED_R, r);
    analogWrite(PIN_LED_G, g);
    analogWrite(PIN_LED_B, b);
}

void ledVerde() { setRGB(0, 255, 0); }
void ledRojo() { setRGB(255, 0, 0); }
void ledApagar() { setRGB(0, 0, 0); }

// ========== FUNCIÓN SEGURA PARA OLED (CON MUTEX) ==========
bool oledEscribir(int tareaID, const char* linea1, const char* linea2, const char* linea3) {
    // Intentar tomar el mutex (esperar máximo 200ms)
    if (xSemaphoreTake(mutexI2C, pdMS_TO_TICKS(200)) == pdTRUE) {
        // --- SECCIÓN CRÍTICA: Solo una tarea puede acceder aquí ---
        ledVerde();
        
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        
        display.setCursor(0, 0);
        display.printf("Tarea %d", tareaID);
        
        display.setCursor(0, 16);
        display.print(linea1);
        
        display.setCursor(0, 32);
        display.print(linea2);
        
        display.setCursor(0, 48);
        display.print(linea3);
        
        display.display();
        
        ledApagar();
        // --- FIN SECCIÓN CRÍTICA ---
        
        xSemaphoreGive(mutexI2C);  // Liberar mutex
        return true;
    } else {
        // Timeout: no se pudo obtener el mutex
        timeouts++;
        ledRojo();
        delay(100);
        ledApagar();
        
        Serial.printf("[Tarea %d] Timeout! No se pudo acceder al I2C\n", tareaID);
        return false;
    }
}

// ========== LEER SENSOR ==========
void leerSensor() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
        temperatura = t;
        humedad = h;
    }
}

// ========== TAREA 1: Mostrar temperatura ==========
void tareaTemperatura(void *parameter) {
    char linea1[20], linea2[20], linea3[20];
    
    while (true) {
        leerSensor();
        
        snprintf(linea1, sizeof(linea1), "Temp: %.1f C", temperatura);
        snprintf(linea2, sizeof(linea2), "Hum: %.1f %%", humedad);
        snprintf(linea3, sizeof(linea3), "Accesos: %d", ++accesos1);
        
        if (oledEscribir(1, linea1, linea2, linea3)) {
            Serial.printf("[T1] OLED actualizado (#%d)\n", accesos1);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));  // Cada 2 segundos
    }
}

// ========== TAREA 2: Mostrar uptime ==========
void tareaUptime(void *parameter) {
    char linea1[20], linea2[20], linea3[20];
    
    while (true) {
        unsigned long segundos = millis() / 1000;
        unsigned long minutos = segundos / 60;
        
        snprintf(linea1, sizeof(linea1), "Uptime: %lu s", segundos);
        snprintf(linea2, sizeof(linea2), "Minutos: %lu", minutos);
        snprintf(linea3, sizeof(linea3), "Accesos: %d", ++accesos2);
        
        if (oledEscribir(2, linea1, linea2, linea3)) {
            Serial.printf("[T2] OLED actualizado (#%d)\n", accesos2);
        }
        
        vTaskDelay(pdMS_TO_TICKS(3500));  // Cada 3.5 segundos
    }
}

// ========== TAREA 3: Mostrar estadísticas ==========
void tareaEstadisticas(void *parameter) {
    char linea1[20], linea2[20], linea3[20];
    
    while (true) {
        snprintf(linea1, sizeof(linea1), "=== ESTADISTICAS ===");
        snprintf(linea2, sizeof(linea2), "Timeouts: %d", timeouts);
        snprintf(linea3, sizeof(linea3), "Accesos: %d", ++accesos3);
        
        if (oledEscribir(3, linea1, linea2, linea3)) {
            Serial.printf("[T3] OLED actualizado (#%d)\n", accesos3);
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));  // Cada 5 segundos
    }
}

// ========== TAREA MONITOR ==========
void tareaMonitor(void *parameter) {
    while (true) {
        Serial.println("\n╔════════════════════════════════════════╗");
        Serial.println("║        MONITOR DE MUTEX I2C           ║");
        Serial.println("╠════════════════════════════════════════╣");
        Serial.printf("║ Tarea 1 (Temp): %d accesos\n", accesos1);
        Serial.printf("║ Tarea 2 (Uptime): %d accesos\n", accesos2);
        Serial.printf("║ Tarea 3 (Stats): %d accesos\n", accesos3);
        Serial.printf("║ Timeouts: %d\n", timeouts);
        Serial.printf("║ Heap libre: %d bytes\n", ESP.getFreeHeap());
        Serial.println("╚════════════════════════════════════════╝\n");
        
        vTaskDelay(pdMS_TO_TICKS(8000));
    }
}

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n=== MUTEX I2C - PROTECCIÓN DE RECURSOS ===");
    
    // Configurar LED RGB
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    ledApagar();
    
    // Inicializar sensores
    dht.begin();
    Wire.begin();
    
    // Inicializar OLED
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("ERROR: No se encontró OLED!");
        while (true) {
            ledRojo();
            delay(500);
            ledApagar();
            delay(500);
        }
    }
    
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("MUTEX");
    display.display();
    delay(2000);
    
    // Crear mutex
    mutexI2C = xSemaphoreCreateMutex();
    if (mutexI2C == NULL) {
        Serial.println("ERROR: No se pudo crear el mutex!");
        while (true) {}
    }
    
    Serial.println("✅ Mutex I2C creado\n");
    
    // Crear tareas FreeRTOS
    xTaskCreate(tareaTemperatura, "Temperatura", 4096, NULL, 2, NULL);
    xTaskCreate(tareaUptime, "Uptime", 4096, NULL, 2, NULL);
    xTaskCreate(tareaEstadisticas, "Estadisticas", 4096, NULL, 2, NULL);
    xTaskCreate(tareaMonitor, "Monitor", 2048, NULL, 1, NULL);
    
    Serial.println("✅ Tareas creadas:");
    Serial.println("   • Tarea 1 (Temp) - Cada 2s");
    Serial.println("   • Tarea 2 (Uptime) - Cada 3.5s");
    Serial.println("   • Tarea 3 (Stats) - Cada 5s");
    Serial.println("   • Monitor - Cada 8s");
    
    Serial.println("\n🚀 Sistema iniciado!");
    Serial.println("   LED Verde = Acceso I2C (Mutex tomado)");
    Serial.println("   LED Rojo = Timeout (Mutex ocupado)\n");
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(10000));
}