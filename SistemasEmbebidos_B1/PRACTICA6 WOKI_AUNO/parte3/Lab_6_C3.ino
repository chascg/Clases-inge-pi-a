// Lab06_Parte3_Uno_Ligero.ino
// Versión para Arduino Uno (usa solo 18KB de flash)

#include "DHT.h"

#define PIN_DHT 4
#define PIN_LED1 5
#define PIN_LED2 6
#define PIN_LED_INT 2
#define PIN_BTN1 7
#define PIN_BTN2 8

DHT dht(PIN_DHT, DHT22);

float temp = 0, hum = 0;
bool led1 = false, led2 = false;
bool autoMode = true;
unsigned long lastRead = 0;
unsigned long lastPub = 0;
int pubCount = 0;
float maxTemp = 30.0;
float minHum = 40.0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED1, OUTPUT);
  pinMode(PIN_LED2, OUTPUT);
  pinMode(PIN_LED_INT, OUTPUT);
  pinMode(PIN_BTN1, INPUT_PULLUP);
  pinMode(PIN_BTN2, INPUT_PULLUP);
  dht.begin();
  
  Serial.println(F("\n=== IoT Dashboard ==="));
  Serial.println(F("Comandos: s=status,1on,1off,2on,2off,auto,man,pub,help\n"));
}

void loop() {
  // Comandos seriales
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    cmd.toLowerCase();
    
    if (cmd == "s" || cmd == "status") mostrarStatus();
    else if (cmd == "1on") { autoMode = false; led1 = true; digitalWrite(PIN_LED1, HIGH); Serial.println(F("LED1 ON")); }
    else if (cmd == "1off") { autoMode = false; led1 = false; digitalWrite(PIN_LED1, LOW); Serial.println(F("LED1 OFF")); }
    else if (cmd == "2on") { autoMode = false; led2 = true; digitalWrite(PIN_LED2, HIGH); Serial.println(F("LED2 ON")); }
    else if (cmd == "2off") { autoMode = false; led2 = false; digitalWrite(PIN_LED2, LOW); Serial.println(F("LED2 OFF")); }
    else if (cmd == "auto") { autoMode = true; Serial.println(F("Modo AUTO")); }
    else if (cmd == "man") { autoMode = false; Serial.println(F("Modo MANUAL")); }
    else if (cmd == "pub") publicarMQTT();
    else if (cmd == "help") mostrarAyuda();
    else if (cmd.length() > 0) Serial.println(F("? help"));
  }
  
  // Lectura periódica
  unsigned long now = millis();
  if (now - lastRead >= 3000) {
    lastRead = now;
    leerSensor();
    if (autoMode) controlAuto();
  }
  
  // Publicación periódica
  if (now - lastPub >= 5000) {
    lastPub = now;
    publicarMQTT();
  }
  
  // Botones
  static bool lastBtn1 = HIGH, lastBtn2 = HIGH;
  bool btn1 = digitalRead(PIN_BTN1);
  bool btn2 = digitalRead(PIN_BTN2);
  
  if (btn1 == LOW && lastBtn1 == HIGH) { delay(50); if(digitalRead(PIN_BTN1)==LOW) publicarMQTT(); }
  if (btn2 == LOW && lastBtn2 == HIGH) { delay(50); if(digitalRead(PIN_BTN2)==LOW) { autoMode = !autoMode; Serial.print(F("Modo: ")); Serial.println(autoMode?F("AUTO"):F("MANUAL")); } }
  
  lastBtn1 = btn1;
  lastBtn2 = btn2;
}

void leerSensor() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t) && !isnan(h)) {
    temp = t;
    hum = h;
    digitalWrite(PIN_LED_INT, HIGH);
    delay(50);
    digitalWrite(PIN_LED_INT, LOW);
  }
}

void controlAuto() {
  if (temp > maxTemp && !led1) { led1 = true; digitalWrite(PIN_LED1, HIGH); Serial.println(F("Auto: LED1 ON")); }
  else if (temp <= maxTemp-1 && led1) { led1 = false; digitalWrite(PIN_LED1, LOW); Serial.println(F("Auto: LED1 OFF")); }
  
  if (hum < minHum && !led2) { led2 = true; digitalWrite(PIN_LED2, HIGH); Serial.println(F("Auto: LED2 ON")); }
  else if (hum >= minHum+5 && led2) { led2 = false; digitalWrite(PIN_LED2, LOW); Serial.println(F("Auto: LED2 OFF")); }
}

void publicarMQTT() {
  pubCount++;
  Serial.println(F("\n--- MQTT Pub ---"));
  Serial.print(F("Temp: ")); Serial.print(temp); Serial.println(F(" C"));
  Serial.print(F("Hum: ")); Serial.print(hum); Serial.println(F("%"));
  Serial.print(F("LED1: ")); Serial.println(led1?F("ON"):F("OFF"));
  Serial.print(F("LED2: ")); Serial.println(led2?F("ON"):F("OFF"));
  Serial.print(F("#")); Serial.println(pubCount);
}

void mostrarStatus() {
  leerSensor();
  Serial.println(F("\n--- STATUS ---"));
  Serial.print(F("Temp: ")); Serial.print(temp); Serial.println(F(" C"));
  Serial.print(F("Hum: ")); Serial.print(hum); Serial.println(F("%"));
  Serial.print(F("LED1: ")); Serial.println(led1?F("ON"):F("OFF"));
  Serial.print(F("LED2: ")); Serial.println(led2?F("ON"):F("OFF"));
  Serial.print(F("Modo: ")); Serial.println(autoMode?F("AUTO"):F("MANUAL"));
  Serial.print(F("Pub: ")); Serial.println(pubCount);
  if(temp > maxTemp) { Serial.print(F("!Alerta Temp > ")); Serial.println(maxTemp); }
  if(hum < minHum) { Serial.print(F("!Alerta Hum < ")); Serial.println(minHum); }
}

void mostrarAyuda() {
  Serial.println(F("\nComandos: s,1on,1off,2on,2off,auto,man,pub,help"));
}