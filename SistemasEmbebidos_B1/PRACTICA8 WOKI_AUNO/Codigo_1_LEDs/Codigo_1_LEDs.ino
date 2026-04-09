#include <Arduino.h>

#define LED_RAPIDO  2
#define LED_LENTO   4

void tarea_led_rapido(void *params) {
    pinMode(LED_RAPIDO, OUTPUT);
    Serial.printf("Tarea LED rapido: Core %d, Prioridad %d\n",
                  xPortGetCoreID(), uxTaskPriorityGet(NULL));
    for (;;) {
        digitalWrite(LED_RAPIDO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(200));
        digitalWrite(LED_RAPIDO, LOW);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void tarea_led_lento(void *params) {
    pinMode(LED_LENTO, OUTPUT);
    Serial.printf("Tarea LED lento: Core %d, Prioridad %d\n",
                  xPortGetCoreID(), uxTaskPriorityGet(NULL));
    for (;;) {
        digitalWrite(LED_LENTO, HIGH);
        vTaskDelay(pdMS_TO_TICKS(1000));
        digitalWrite(LED_LENTO, LOW);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void tarea_monitor(void *params) {
    Serial.printf("Tarea monitor: Core %d, Prioridad %d\n",
                  xPortGetCoreID(), uxTaskPriorityGet(NULL));
    for (;;) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        Serial.println(F("\n--- Monitor del sistema ---"));
        Serial.printf(" Heap libre: %u bytes\n", esp_get_free_heap_size());
        Serial.printf(" Tiempo activo: %lu s\n", millis()/1000);
        UBaseType_t hw = uxTaskGetStackHighWaterMark(NULL);
        Serial.printf(" Stack libre (monitor): %u words\n", hw);
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("== Lab 08: FreeRTOS Basico ==="));
    
    xTaskCreatePinnedToCore(tarea_led_rapido, "LED_Rapido", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(tarea_led_lento, "LED_Lento", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(tarea_monitor, "Monitor", 4096, NULL, 2, NULL, 1);
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(10000));
}