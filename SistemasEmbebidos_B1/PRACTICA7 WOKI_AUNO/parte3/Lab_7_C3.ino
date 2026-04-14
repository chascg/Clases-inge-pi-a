// Lab07_RGB_Controller_Simple.ino
// RGB Controller BLE - Versión para Arduino Uno

#include "DHT.h"

// ========== CONFIGURACIÓN HARDWARE (Arduino Uno) ==========
#define PIN_DHT 4
#define PIN_LED_R 9     // PWM
#define PIN_LED_G 10    // PWM
#define PIN_LED_B 11    // PWM
#define PIN_BTN 7

DHT dht(PIN_DHT, DHT22);

// ========== VARIABLES ==========
bool conectado = false;
bool notificando = true;
float temperatura = 0;
int r = 0, g = 0, b = 0;
String modoActual = "OFF";
bool modoArcoiris = false;
unsigned long lastNotify = 0;
unsigned long lastRainbow = 0;

// ========== SETUP ==========
void setup() {
    Serial.begin(115200);
    
    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    pinMode(PIN_BTN, INPUT_PULLUP);
    
    setRGB(0, 0, 0);
    dht.begin();
    
    Serial.println(F("\n=== RGB CONTROLLER BLE ==="));
    Serial.println(F("Dispositivo: ESP32-RGB-Controller"));
    Serial.println(F("Servicio: 12345678-1234-1234-1234-abcdef987654"));
    Serial.println(F("  - Color (Write) - RGB(255,0,0) etc"));
    Serial.println(F("  - Mode (Write) - OFF, WHITE, RAINBOW"));
    Serial.println(F("  - Temperature (Read, Notify)"));
    Serial.println(F("\nCOMANDOS:"));
    Serial.println(F("  conn         - Conectar cliente"));
    Serial.println(F("  disc         - Desconectar"));
    Serial.println(F("  color/R,G,B  - Color personalizado (ej: color/255,0,0)"));
    Serial.println(F("  color/red    - Rojo"));
    Serial.println(F("  color/green  - Verde"));
    Serial.println(F("  color/blue   - Azul"));
    Serial.println(F("  color/white  - Blanco"));
    Serial.println(F("  color/off    - Apagar"));
    Serial.println(F("  mode/rainbow - Modo arcoiris"));
    Serial.println(F("  mode/off     - Apagar"));
    Serial.println(F("  mode/white   - Blanco fijo"));
    Serial.println(F("  temp         - Leer temperatura"));
    Serial.println(F("  noti_on      - Activar notificaciones"));
    Serial.println(F("  noti_off     - Desactivar"));
    Serial.println(F("  status       - Ver estado"));
    Serial.println(F("  gatt         - Ver servicios BLE"));
    Serial.println(F("  help         - Ayuda"));
    Serial.println(F("========================\n"));
}

// ========== CONTROL RGB ==========
void setRGB(int red, int green, int blue) {
    r = red; g = green; b = blue;
    analogWrite(PIN_LED_R, r);
    analogWrite(PIN_LED_G, g);
    analogWrite(PIN_LED_B, b);
}

// ========== EFECTO ARCOÍRIS ==========
void rainbow() {
    static int paso = 0;
    paso++;
    
    int fase = paso % 1530;
    
    if (fase < 255) {
        setRGB(255, fase, 0);
    } else if (fase < 510) {
        setRGB(510 - fase, 255, 0);
    } else if (fase < 765) {
        setRGB(0, 255, fase - 510);
    } else if (fase < 1020) {
        setRGB(0, 1020 - fase, 255);
    } else if (fase < 1275) {
        setRGB(fase - 1020, 0, 255);
    } else {
        setRGB(255, 0, 1530 - fase);
    }
}

// ========== LEER SENSOR ==========
void leerSensor() {
    float t = dht.readTemperature();
    if (!isnan(t)) temperatura = t;
}

// ========== MOSTRAR ESTADO ==========
void mostrarStatus() {
    leerSensor();
    
    Serial.println(F("\n--- RGB CONTROLLER ---"));
    Serial.print(F("Cliente: ")); Serial.println(conectado ? F("CONECTADO") : F("DESCONECTADO"));
    Serial.print(F("RGB: (")); Serial.print(r); Serial.print(F(",")); Serial.print(g); Serial.print(F(",")); Serial.println(b);
    Serial.print(F("Modo: ")); Serial.println(modoActual);
    Serial.print(F("Temperatura: ")); Serial.print(temperatura); Serial.println(F(" °C"));
    Serial.print(F("Notificaciones: ")); Serial.println(notificando ? F("ACTIVAS") : F("INACTIVAS"));
    Serial.println(F("--------------------\n"));
}

// ========== MOSTRAR GATT ==========
void mostrarGATT() {
    leerSensor();
    
    Serial.println(F("\n--- TABLA GATT ---"));
    Serial.println(F("SERVICIO: 12345678-1234-1234-1234-abcdef987654"));
    Serial.println(F("  CARACTERÍSTICA COLOR: 12345678-1234-1234-1234-abcdef987655"));
    Serial.println(F("    [WRITE] 3 bytes (R,G,B)"));
    Serial.print(F("    Valor: RGB(")); Serial.print(r); Serial.print(F(",")); Serial.print(g); Serial.print(F(",")); Serial.println(b);
    Serial.println(F("  CARACTERÍSTICA MODE: 12345678-1234-1234-1234-abcdef987656"));
    Serial.println(F("    [WRITE] String"));
    Serial.print(F("    Valor: ")); Serial.println(modoActual);
    Serial.println(F("  CARACTERÍSTICA TEMP: 12345678-1234-1234-1234-abcdef987657"));
    Serial.println(F("    [READ, NOTIFY]"));
    Serial.print(F("    Valor: ")); Serial.print(temperatura); Serial.println(F(" °C"));
    Serial.println(F("----------------\n"));
}

// ========== CONEXIÓN ==========
void conectar() {
    Serial.println(F("\n🔵 Cliente CONECTADO"));
    conectado = true;
}

void desconectar() {
    Serial.println(F("\n⚪ Cliente DESCONECTADO"));
    conectado = false;
    modoArcoiris = false;
    setRGB(0, 0, 0);
    modoActual = "OFF";
}

// ========== COMANDOS ==========
void setColor(String valor) {
    if (!conectado) {
        Serial.println(F("❌ Sin cliente conectado"));
        return;
    }
    
    modoArcoiris = false;
    Serial.println(F("\n✏️ WRITE - Color"));
    
    if (valor == "red") {
        setRGB(255, 0, 0);
        modoActual = "RED";
        Serial.println(F("Color: ROJO"));
    }
    else if (valor == "green") {
        setRGB(0, 255, 0);
        modoActual = "GREEN";
        Serial.println(F("Color: VERDE"));
    }
    else if (valor == "blue") {
        setRGB(0, 0, 255);
        modoActual = "BLUE";
        Serial.println(F("Color: AZUL"));
    }
    else if (valor == "white") {
        setRGB(255, 255, 255);
        modoActual = "WHITE";
        Serial.println(F("Color: BLANCO"));
    }
    else if (valor == "off") {
        setRGB(0, 0, 0);
        modoActual = "OFF";
        Serial.println(F("Color: APAGADO"));
    }
    else {
        // Parsear R,G,B
        int rv, gv, bv;
        if (sscanf(valor.c_str(), "%d,%d,%d", &rv, &gv, &bv) == 3) {
            rv = constrain(rv, 0, 255);
            gv = constrain(gv, 0, 255);
            bv = constrain(bv, 0, 255);
            setRGB(rv, gv, bv);
            modoActual = "CUSTOM";
            Serial.print(F("Color personalizado: RGB("));
            Serial.print(rv); Serial.print(F(",")); Serial.print(gv); Serial.print(F(",")); Serial.println(bv);
        } else {
            Serial.println(F("❌ Formato: color/255,0,0"));
        }
    }
}

void setMode(String modo) {
    if (!conectado) {
        Serial.println(F("❌ Sin cliente conectado"));
        return;
    }
    
    Serial.println(F("\n✏️ WRITE - Mode"));
    modo.toLowerCase();
    
    if (modo == "rainbow") {
        modoArcoiris = true;
        modoActual = "RAINBOW";
        Serial.println(F("Modo: ARCOÍRIS 🌈"));
    }
    else if (modo == "off") {
        modoArcoiris = false;
        setRGB(0, 0, 0);
        modoActual = "OFF";
        Serial.println(F("Modo: APAGADO"));
    }
    else if (modo == "white") {
        modoArcoiris = false;
        setRGB(255, 255, 255);
        modoActual = "WHITE";
        Serial.println(F("Modo: BLANCO"));
    }
    else {
        Serial.println(F("❌ Modos: rainbow, off, white"));
    }
}

void leerTemperatura() {
    if (!conectado) {
        Serial.println(F("❌ Sin cliente conectado"));
        return;
    }
    leerSensor();
    Serial.print(F("\n📖 Temperatura: ")); Serial.print(temperatura); Serial.println(F(" °C\n"));
}

void enviarNotificacion() {
    if (!conectado || !notificando) return;
    
    leerSensor();
    Serial.println(F("\n📤 NOTIFY - Temperature"));
    Serial.print(F("Valor: ")); Serial.print(temperatura); Serial.println(F(" °C"));
}

// ========== AYUDA ==========
void mostrarAyuda() {
    Serial.println(F("\n📋 COMANDOS:"));
    Serial.println(F("  conn         - Conectar"));
    Serial.println(F("  disc         - Desconectar"));
    Serial.println(F("  color/255,0,0- Color RGB"));
    Serial.println(F("  color/red    - Rojo"));
    Serial.println(F("  color/green  - Verde"));
    Serial.println(F("  color/blue   - Azul"));
    Serial.println(F("  color/white  - Blanco"));
    Serial.println(F("  color/off    - Apagar"));
    Serial.println(F("  mode/rainbow - Arcoiris"));
    Serial.println(F("  mode/off     - Apagar"));
    Serial.println(F("  mode/white   - Blanco"));
    Serial.println(F("  temp         - Leer temp"));
    Serial.println(F("  noti_on      - Notificaciones ON"));
    Serial.println(F("  noti_off     - Notificaciones OFF"));
    Serial.println(F("  status       - Estado"));
    Serial.println(F("  gatt         - Tabla GATT"));
    Serial.println(F("  help         - Ayuda\n"));
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
    else if (cmd.startsWith("color/")) {
        setColor(cmd.substring(6));
    }
    else if (cmd.startsWith("mode/")) {
        setMode(cmd.substring(5));
    }
    else if (cmd == "temp") {
        leerTemperatura();
    }
    else if (cmd == "noti_on") {
        notificando = true;
        Serial.println(F("✅ Notificaciones ACTIVADAS"));
    }
    else if (cmd == "noti_off") {
        notificando = false;
        Serial.println(F("⏸️ Notificaciones DESACTIVADAS"));
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

// ========== LOOP ==========
void loop() {
    // Comandos seriales
    if (Serial.available()) {
        String comando = Serial.readStringUntil('\n');
        procesarComando(comando);
    }
    
    // Efecto arcoíris
    if (modoArcoiris && millis() - lastRainbow >= 30) {
        lastRainbow = millis();
        rainbow();
    }
    
    // Notificaciones cada 5 segundos
    if (conectado && notificando) {
        if (millis() - lastNotify >= 5000) {
            lastNotify = millis();
            enviarNotificacion();
        }
    }
}