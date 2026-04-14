// Lab_5_C1.ino - Código principal de prueba
#include "hal_gpio.h"

void setup() {
    Serial.begin(9600);
    Serial.println(F("=== Prueba de HAL GPIO ==="));
    
    // Configurar pines
    HAL_GPIO_SetDir(13, HAL_GPIO_OUTPUT);  // LED como salida
    HAL_GPIO_SetDir(2, HAL_GPIO_INPUT);    // Botón como entrada
    
    Serial.println(F("Configuracion completada"));
    Serial.println(F("Presiona el boton en pin 2 para encender LED"));
}

void loop() {
    // Leer botón
    HAL_GPIO_Level estado_boton = HAL_GPIO_Read(2);
    
    if (estado_boton == HAL_GPIO_LOW) {  // Botón presionado
        HAL_GPIO_Write(13, HAL_GPIO_HIGH);  // Encender LED
        Serial.println(F("LED ENCENDIDO"));
        HAL_DelayMs(200);
    } else {
        HAL_GPIO_Write(13, HAL_GPIO_LOW);   // Apagar LED
    }
    
    HAL_DelayMs(50);
}