// Lab07_BLE_Sensor_Simple.ino
// BLE Sensor Simulado - Versión Ligera para Arduino

#include "DHT.h"

// ========== CONFIGURACIÓN ==========
#define PIN_DHT 4
#define PIN_LED 2

DHT dht(PIN_DHT, DHT22);

// ========== VARIABLES ==========
bool conectado = false;
bool notificando = false;
float temperatura = 0;
float humedad = 0;
unsigned long lastNotify = 0;
int notifCount = 0;

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_LED, OUTPUT);
    digitalWrite(PIN_LED, LOW);
    
    dht.begin();
    
    Serial.println(F("\n=== BLE SENSOR SIMULADO ==="));
    Serial.println(F("Dispositivo: ESP32-Lab07-Sensor"));
    Serial.println(F("Servicio: 12345678-1234-1234-1234-123456789abc"));
    Serial.println(F("\nCOMANDOS:"));
    Serial.println(F("  conn      - Conectar cliente"));
    Serial.println(F("  disc      - Desconectar"));
    Serial.println(F("  read      - Leer temperatura"));
    Serial.println(F("  noti_on   - Activar notificaciones"));
    Serial.println(F("  noti_off  - Desactivar notificaciones"));
    Serial.println(F("  led_on    - Encender LED (comando vía BLE)"));
    Serial.println(F("  led_off   - Apagar LED (comando vía BLE)"));
    Serial.println(F("  status    - Ver estado"));
    Serial.println(F("  help      - Esta ayuda"));
    Serial.println(F("========================\n"));
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

// ========== MOSTRAR ESTADO ==========
void mostrarStatus() {
    leerSensor();
    
    Serial.println(F("\n--- ESTADO BLE ---"));
    Serial.print(F("Cliente: ")); Serial.println(conectado ? F("CONECTADO") : F("DESCONECTADO"));
    Serial.print(F("Notificaciones: ")); Serial.println(notificando ? F("ACTIVAS") : F("INACTIVAS"));
    Serial.print(F("Temperatura: ")); Serial.print(temperatura); Serial.println(F(" °C"));
    Serial.print(F("Humedad: ")); Serial.print(humedad); Serial.println(F(" %"));
    Serial.print(F("LED: ")); Serial.println(digitalRead(PIN_LED) ? F("ON") : F("OFF"));
    Serial.print(F("Notificaciones enviadas: ")); Serial.println(notifCount);
    Serial.println(F("--------------------\n"));
}

// ========== SIMULAR CONEXIÓN ==========
void simularConnect() {
    Serial.println(F("\n🔵 Cliente CONECTADO"));
    conectado = true;
    digitalWrite(PIN_LED, HIGH);
    delay(200);
    digitalWrite(PIN_LED, LOW);
}

// ========== SIMULAR DESCONEXIÓN ==========
void simularDisconnect() {
    Serial.println(F("\n⚪ Cliente DESCONECTADO"));
    conectado = false;
    notificando = false;
    digitalWrite(PIN_LED, LOW);
}

// ========== SIMULAR READ ==========
void simularRead() {
    if (!conectado) {
        Serial.println(F("❌ Error: No hay cliente conectado"));
        return;
    }
    
    leerSensor();
    Serial.println(F("\n📖 READ - Temperatura"));
    Serial.print(F("Valor: ")); Serial.print(temperatura); Serial.println(F(" °C"));
}

// ========== SIMULAR NOTIFICACIÓN ==========
void simularNotify() {
    if (!conectado) {
        return;
    }
    
    leerSensor();
    notifCount++;
    
    Serial.println(F("\n📤 NOTIFICACIÓN BLE"));
    Serial.print(F("Temperatura: ")); Serial.print(temperatura); Serial.println(F(" °C"));
    Serial.print(F("#")); Serial.println(notifCount);
}

// ========== SIMULAR ESCRITURA (comando) ==========
void simularWrite(String comando) {
    if (!conectado) {
        Serial.println(F("❌ Error: No hay cliente conectado"));
        return;
    }
    
    Serial.println(F("\n✏️ WRITE - Comando recibido"));
    Serial.print(F("Valor: ")); Serial.println(comando);
    
    if (comando == "LED_ON") {
        digitalWrite(PIN_LED, HIGH);
        Serial.println(F("✅ LED ENCENDIDO"));
    } else if (comando == "LED_OFF") {
        digitalWrite(PIN_LED, LOW);
        Serial.println(F("✅ LED APAGADO"));
    } else if (comando == "STATUS") {
        Serial.print(F("Estado: Temp=")); Serial.print(temperatura); Serial.println(F("°C"));
    } else {
        Serial.println(F("⚠️ Comando desconocido"));
    }
}

// ========== MOSTRAR TABLA GATT ==========
void mostrarGATT() {
    Serial.println(F("\n--- TABLA GATT ---"));
    Serial.println(F("SERVICIO: 12345678-1234-1234-1234-123456789abc"));
    Serial.println(F("  CARACTERÍSTICA TEMP: 12345678-1234-1234-1234-123456789abd"));
    Serial.println(F("    Propiedades: READ | NOTIFY"));
    Serial.print(F("    Notificaciones: ")); Serial.println(notificando ? F("ACTIVAS") : F("INACTIVAS"));
    Serial.println(F("  CARACTERÍSTICA CMD: 12345678-1234-1234-1234-123456789abe"));
    Serial.println(F("    Propiedades: WRITE"));
    Serial.println(F("------------------\n"));
}

// ========== MOSTRAR AYUDA ==========
void mostrarAyuda() {
    Serial.println(F("\n📋 COMANDOS:"));
    Serial.println(F("  conn      - Conectar"));
    Serial.println(F("  disc      - Desconectar"));
    Serial.println(F("  read      - Leer temperatura"));
    Serial.println(F("  noti_on   - Activar notificaciones"));
    Serial.println(F("  noti_off  - Desactivar notificaciones"));
    Serial.println(F("  led_on    - LED ON (vía BLE)"));
    Serial.println(F("  led_off   - LED OFF (vía BLE)"));
    Serial.println(F("  status    - Ver estado"));
    Serial.println(F("  gatt      - Ver tabla GATT"));
    Serial.println(F("  help      - Esta ayuda\n"));
}

// ========== PROCESAR COMANDOS ==========
void procesarComando(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd == "help") {
        mostrarAyuda();
    }
    else if (cmd == "conn") {
        simularConnect();
    }
    else if (cmd == "disc") {
        simularDisconnect();
    }
    else if (cmd == "read") {
        simularRead();
    }
    else if (cmd == "noti_on") {
        if (!conectado) {
            Serial.println(F("❌ Conecta primero con 'conn'"));
        } else {
            notificando = true;
            Serial.println(F("✅ Notificaciones ACTIVADAS"));
        }
    }
    else if (cmd == "noti_off") {
        notificando = false;
        Serial.println(F("⏸️ Notificaciones DESACTIVADAS"));
    }
    else if (cmd == "led_on") {
        simularWrite("LED_ON");
    }
    else if (cmd == "led_off") {
        simularWrite("LED_OFF");
    }
    else if (cmd == "status") {
        mostrarStatus();
    }
    else if (cmd == "gatt") {
        mostrarGATT();
    }
    else if (cmd.length() > 0) {
        Serial.println(F("❌ Comando no reconocido. Escribe 'help'"));
    }
}

// ========== LOOP PRINCIPAL ==========
void loop() {
    // Comandos seriales
    if (Serial.available()) {
        String comando = Serial.readStringUntil('\n');
        procesarComando(comando);
    }
    
    // Notificaciones automáticas cada 3 segundos
    if (conectado && notificando) {
        unsigned long ahora = millis();
        if (ahora - lastNotify >= 3000) {
            lastNotify = ahora;
            simularNotify();
        }
    }
}