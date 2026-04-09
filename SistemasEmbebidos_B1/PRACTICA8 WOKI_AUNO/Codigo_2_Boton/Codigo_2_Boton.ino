#include <Arduino.h>

#define PIN_BTN 15
#define PIN_LED 2

SemaphoreHandle_t xSemBtn = NULL;

void IRAM_ATTR ISR_boton() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSemBtn, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void tarea_procesar_boton(void *params) {
    pinMode(PIN_LED, OUTPUT);
    uint32_t conteo = 0;
    for (;;) {
        if (xSemaphoreTake(xSemBtn, portMAX_DELAY) == pdTRUE) {
            conteo++;
            digitalWrite(PIN_LED, !digitalRead(PIN_LED));
            Serial.printf("[Core %d] Boton #%u — LED: %s\n",
                          xPortGetCoreID(), conteo,
                          digitalRead(PIN_LED) ? "ON" : "OFF");
        }
    }
}

void setup() {
    Serial.begin(115200);
    pinMode(PIN_BTN, INPUT_PULLUP);
    
    xSemBtn = xSemaphoreCreateBinary();
    attachInterrupt(digitalPinToInterrupt(PIN_BTN), ISR_boton, FALLING);
    xTaskCreatePinnedToCore(tarea_procesar_boton, "ProcBtn", 2048, NULL, 3, NULL, 1);
    Serial.println(F("Sistema listo. Presiona el boton."));
}

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}