// Lab06_Wokwi_Parte1_Simple.ino
// Servidor Web Simulado - Versión Simple

#include "DHT.h"

// ========== CONFIGURACIÓN ==========
#define PIN_DHT 4
#define PIN_LED1 5
#define PIN_LED2 6      // En Arduino Uno usa pin 6
#define PIN_LED_INT 2

DHT dht(PIN_DHT, DHT22);

// ========== VARIABLES ==========
float temp = 0, hum = 0;
bool led1State = false, led2State = false;

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    
    // Configurar pines
    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);
    pinMode(PIN_LED_INT, OUTPUT);
    digitalWrite(PIN_LED1, LOW);
    digitalWrite(PIN_LED2, LOW);
    digitalWrite(PIN_LED_INT, LOW);
    
    // Iniciar sensor
    dht.begin();
    
    // Mensaje de inicio
    Serial.println("\n=== SERVIDOR WEB SIMULADO ===");
    Serial.println("IP: 192.168.1.100");
    Serial.println("\nCOMANDOS:");
    Serial.println("  temp     - Ver temperatura");
    Serial.println("  hum      - Ver humedad");
    Serial.println("  todo     - Ver todos los datos");
    Serial.println("  led1on   - Encender LED1");
    Serial.println("  led1off  - Apagar LED1");
    Serial.println("  led2on   - Encender LED2");
    Serial.println("  led2off  - Apagar LED2");
    Serial.println("  led2tog  - Alternar LED2");
    Serial.println("  help     - Mostrar ayuda");
    Serial.println("=============================\n");
}

// ========== LEER SENSOR ==========
void leerSensor() {
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    
    if (!isnan(t) && !isnan(h)) {
        temp = t;
        hum = h;
        // Parpadeo indicador
        digitalWrite(PIN_LED_INT, HIGH);
        delay(50);
        digitalWrite(PIN_LED_INT, LOW);
    }
}

// ========== MOSTRAR DATOS ==========
void mostrarTemperatura() {
    leerSensor();
    Serial.print("Temperatura: ");
    Serial.print(temp);
    Serial.println(" °C");
}

void mostrarHumedad() {
    leerSensor();
    Serial.print("Humedad: ");
    Serial.print(hum);
    Serial.println(" %");
}

void mostrarTodo() {
    leerSensor();
    Serial.println("\n--- ESTADO ACTUAL ---");
    Serial.print("Temperatura: "); Serial.print(temp); Serial.println(" °C");
    Serial.print("Humedad: "); Serial.print(hum); Serial.println(" %");
    Serial.print("LED1: "); Serial.println(led1State ? "ON" : "OFF");
    Serial.print("LED2: "); Serial.println(led2State ? "ON" : "OFF");
    Serial.println("---------------------\n");
}

// ========== CONTROL LEDS ==========
void led1On() {
    led1State = true;
    digitalWrite(PIN_LED1, HIGH);
    Serial.println("LED1: ENCENDIDO");
}

void led1Off() {
    led1State = false;
    digitalWrite(PIN_LED1, LOW);
    Serial.println("LED1: APAGADO");
}

void led2On() {
    led2State = true;
    digitalWrite(PIN_LED2, HIGH);
    Serial.println("LED2: ENCENDIDO");
}

void led2Off() {
    led2State = false;
    digitalWrite(PIN_LED2, LOW);
    Serial.println("LED2: APAGADO");
}

void led2Toggle() {
    if (led2State) {
        led2Off();
    } else {
        led2On();
    }
}

void mostrarAyuda() {
    Serial.println("\nCOMANDOS DISPONIBLES:");
    Serial.println("  temp     - Temperatura");
    Serial.println("  hum      - Humedad");
    Serial.println("  todo     - Todos los datos");
    Serial.println("  led1on   - LED1 ON");
    Serial.println("  led1off  - LED1 OFF");
    Serial.println("  led2on   - LED2 ON");
    Serial.println("  led2off  - LED2 OFF");
    Serial.println("  led2tog  - Alternar LED2");
    Serial.println("  help     - Esta ayuda\n");
}

// ========== PROCESAR COMANDOS ==========
void procesarComando(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd == "help") {
        mostrarAyuda();
    }
    else if (cmd == "temp") {
        mostrarTemperatura();
    }
    else if (cmd == "hum") {
        mostrarHumedad();
    }
    else if (cmd == "todo") {
        mostrarTodo();
    }
    else if (cmd == "led1on") {
        led1On();
    }
    else if (cmd == "led1off") {
        led1Off();
    }
    else if (cmd == "led2on") {
        led2On();
    }
    else if (cmd == "led2off") {
        led2Off();
    }
    else if (cmd == "led2tog") {
        led2Toggle();
    }
    else if (cmd.length() > 0) {
        Serial.println("Comando no reconocido. Escribe 'help'");
    }
}

// ========== LOOP ==========
void loop() {
    if (Serial.available()) {
        String comando = Serial.readStringUntil('\n');
        procesarComando(comando);
    }
}