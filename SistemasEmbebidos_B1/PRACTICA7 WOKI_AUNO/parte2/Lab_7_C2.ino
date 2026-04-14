// Lab07_Weather_Station_Simple.ino
// BLE Weather Station - Versión Ligera para Arduino

#include "DHT.h"

// ========== CONFIGURACIÓN ==========
#define PIN_DHT 4
#define PIN_LED1 2      // LED interno
#define PIN_LED2 5      // LED externo
#define PIN_BTN 7       // Botón (en Uno usar pin 7)

DHT dht(PIN_DHT, DHT22);

// ========== VARIABLES ==========
bool conectado = false;
bool notificando = false;
float temperatura = 0;
float humedad = 0;
bool led1State = false;
bool led2State = false;
unsigned long lastNotify = 0;
int notifCount = 0;

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);
    pinMode(PIN_BTN, INPUT_PULLUP);
    
    digitalWrite(PIN_LED1, LOW);
    digitalWrite(PIN_LED2, LOW);
    
    dht.begin();
    
    Serial.println(F("\n=== WEATHER STATION BLE ==="));
    Serial.println(F("Dispositivo: ESP32-Weather-Station"));
    Serial.println(F("Servicio: Environmental Sensing (0x181A)"));
    Serial.println(F("  - Temperature (0x2A6E) - Read, Notify"));
    Serial.println(F("  - Humidity (0x2A6F) - Read, Notify"));
    Serial.println(F("Servicio Custom: Comandos - Write"));
    Serial.println(F("\nCOMANDOS:"));
    Serial.println(F("  conn      - Conectar cliente"));
    Serial.println(F("  disc      - Desconectar"));
    Serial.println(F("  temp      - Leer temperatura"));
    Serial.println(F("  hum       - Leer humedad"));
    Serial.println(F("  noti_on   - Activar notificaciones"));
    Serial.println(F("  noti_off  - Desactivar notificaciones"));
    Serial.println(F("  led1_on   - Encender LED1 (vía BLE)"));
    Serial.println(F("  led1_off  - Apagar LED1"));
    Serial.println(F("  led2_on   - Encender LED2"));
    Serial.println(F("  led2_off  - Apagar LED2"));
    Serial.println(F("  all_on    - Encender ambos LEDs"));
    Serial.println(F("  all_off   - Apagar ambos LEDs"));
    Serial.println(F("  status    - Ver estado"));
    Serial.println(F("  services  - Ver servicios BLE"));
    Serial.println(F("  help      - Ayuda"));
    Serial.println(F("===========================\n"));
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
    
    Serial.println(F("\n--- WEATHER STATION ---"));
    Serial.print(F("Cliente: ")); Serial.println(conectado ? F("CONECTADO") : F("DESCONECTADO"));
    Serial.print(F("Notificaciones: ")); Serial.println(notificando ? F("ACTIVAS") : F("INACTIVAS"));
    Serial.print(F("Temperatura: ")); Serial.print(temperatura); Serial.println(F(" °C"));
    Serial.print(F("Humedad: ")); Serial.print(humedad); Serial.println(F(" %"));
    Serial.print(F("LED1 (int): ")); Serial.println(led1State ? F("ON") : F("OFF"));
    Serial.print(F("LED2 (ext): ")); Serial.println(led2State ? F("ON") : F("OFF"));
    Serial.print(F("Notificaciones enviadas: ")); Serial.println(notifCount);
    Serial.println(F("------------------------\n"));
}

// ========== LISTAR SERVICIOS ==========
void mostrarServicios() {
    Serial.println(F("\n--- SERVICIOS BLE ---"));
    Serial.println(F("1. Environmental Sensing (0x181A)"));
    Serial.println(F("   - Temperature (0x2A6E) - Read, Notify"));
    Serial.print(F("     Valor: ")); Serial.print(temperatura); Serial.println(F(" °C"));
    Serial.println(F("   - Humidity (0x2A6F) - Read, Notify"));
    Serial.print(F("     Valor: ")); Serial.print(humedad); Serial.println(F(" %"));
    Serial.println(F("2. Custom Service (12345678-1234-1234-1234-123456789def)"));
    Serial.println(F("   - Custom Command - Write"));
    Serial.println(F("----------------------\n"));
}

// ========== CONEXIÓN/DESCONEXIÓN ==========
void conectar() {
    Serial.println(F("\n🔵 Cliente CONECTADO"));
    conectado = true;
    digitalWrite(PIN_LED1, HIGH);
    delay(200);
    digitalWrite(PIN_LED1, LOW);
}

void desconectar() {
    Serial.println(F("\n⚪ Cliente DESCONECTADO"));
    conectado = false;
    notificando = false;
    digitalWrite(PIN_LED1, LOW);
    digitalWrite(PIN_LED2, LOW);
    led1State = false;
    led2State = false;
}

// ========== LECTURAS ==========
void leerTemperatura() {
    if (!conectado) {
        Serial.println(F("❌ Sin cliente conectado"));
        return;
    }
    leerSensor();
    Serial.println(F("\n📖 READ - Temperature"));
    Serial.print(F("Valor: ")); Serial.print(temperatura); Serial.println(F(" °C"));
}

void leerHumedad() {
    if (!conectado) {
        Serial.println(F("❌ Sin cliente conectado"));
        return;
    }
    leerSensor();
    Serial.println(F("\n📖 READ - Humidity"));
    Serial.print(F("Valor: ")); Serial.print(humedad); Serial.println(F(" %"));
}

// ========== NOTIFICACIONES ==========
void enviarNotificacion() {
    if (!conectado) return;
    
    leerSensor();
    notifCount++;
    
    Serial.println(F("\n📤 NOTIFY - Environmental Sensing"));
    Serial.print(F("Temperature: ")); Serial.print(temperatura); Serial.println(F(" °C"));
    Serial.print(F("Humidity: ")); Serial.print(humedad); Serial.println(F(" %"));
    Serial.print(F("#")); Serial.println(notifCount);
    
    // Parpadeo indicador
    digitalWrite(PIN_LED1, HIGH);
    delay(50);
    digitalWrite(PIN_LED1, LOW);
}

// ========== COMANDOS (ESCRITURA BLE) ==========
void ejecutarComando(String cmd) {
    if (!conectado) {
        Serial.println(F("❌ Sin cliente conectado"));
        return;
    }
    
    Serial.println(F("\n✏️ WRITE - Comando recibido"));
    Serial.print(F("Valor: ")); Serial.println(cmd);
    
    if (cmd == "LED1_ON") {
        led1State = true;
        digitalWrite(PIN_LED1, HIGH);
        Serial.println(F("✅ LED1 ENCENDIDO"));
    }
    else if (cmd == "LED1_OFF") {
        led1State = false;
        digitalWrite(PIN_LED1, LOW);
        Serial.println(F("✅ LED1 APAGADO"));
    }
    else if (cmd == "LED2_ON") {
        led2State = true;
        digitalWrite(PIN_LED2, HIGH);
        Serial.println(F("✅ LED2 ENCENDIDO"));
    }
    else if (cmd == "LED2_OFF") {
        led2State = false;
        digitalWrite(PIN_LED2, LOW);
        Serial.println(F("✅ LED2 APAGADO"));
    }
    else if (cmd == "ALL_ON") {
        led1State = led2State = true;
        digitalWrite(PIN_LED1, HIGH);
        digitalWrite(PIN_LED2, HIGH);
        Serial.println(F("✅ AMBOS LEDs ENCENDIDOS"));
    }
    else if (cmd == "ALL_OFF") {
        led1State = led2State = false;
        digitalWrite(PIN_LED1, LOW);
        digitalWrite(PIN_LED2, LOW);
        Serial.println(F("✅ AMBOS LEDs APAGADOS"));
    }
    else {
        Serial.println(F("⚠️ Comando desconocido"));
    }
}

// ========== AYUDA ==========
void mostrarAyuda() {
    Serial.println(F("\n📋 COMANDOS:"));
    Serial.println(F("  conn      - Conectar"));
    Serial.println(F("  disc      - Desconectar"));
    Serial.println(F("  temp      - Leer temperatura"));
    Serial.println(F("  hum       - Leer humedad"));
    Serial.println(F("  noti_on   - Activar notificaciones"));
    Serial.println(F("  noti_off  - Desactivar notificaciones"));
    Serial.println(F("  led1_on   - LED1 ON"));
    Serial.println(F("  led1_off  - LED1 OFF"));
    Serial.println(F("  led2_on   - LED2 ON"));
    Serial.println(F("  led2_off  - LED2 OFF"));
    Serial.println(F("  all_on    - Ambos LEDs ON"));
    Serial.println(F("  all_off   - Ambos LEDs OFF"));
    Serial.println(F("  status    - Ver estado"));
    Serial.println(F("  services  - Ver servicios BLE"));
    Serial.println(F("  help      - Ayuda\n"));
}

// ========== PROCESAR COMANDOS ==========
void procesarComando(String cmd) {
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd == "help") {
        mostrarAyuda();
    }
    else if (cmd == "conn") {
        conectar();
    }
    else if (cmd == "disc") {
        desconectar();
    }
    else if (cmd == "temp") {
        leerTemperatura();
    }
    else if (cmd == "hum") {
        leerHumedad();
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
    else if (cmd == "led1_on") {
        ejecutarComando("LED1_ON");
    }
    else if (cmd == "led1_off") {
        ejecutarComando("LED1_OFF");
    }
    else if (cmd == "led2_on") {
        ejecutarComando("LED2_ON");
    }
    else if (cmd == "led2_off") {
        ejecutarComando("LED2_OFF");
    }
    else if (cmd == "all_on") {
        ejecutarComando("ALL_ON");
    }
    else if (cmd == "all_off") {
        ejecutarComando("ALL_OFF");
    }
    else if (cmd == "status") {
        mostrarStatus();
    }
    else if (cmd == "services") {
        mostrarServicios();
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
    
    // Botón físico
    static bool lastBtn = HIGH;
    bool btn = digitalRead(PIN_BTN);
    if (btn == LOW && lastBtn == HIGH && conectado) {
        delay(50);
        if (digitalRead(PIN_BTN) == LOW) {
            Serial.println(F("\n🔘 Botón presionado"));
            enviarNotificacion();
        }
    }
    lastBtn = btn;
    
    // Notificaciones automáticas cada 4 segundos
    if (conectado && notificando) {
        unsigned long ahora = millis();
        if (ahora - lastNotify >= 4000) {
            lastNotify = ahora;
            enviarNotificacion();
        }
    }
}