const int PIN_LED = 13;

void setup() {
  pinMode(PIN_LED, OUTPUT);
}

void loop() {
  // Encendido 200 ms
  digitalWrite(PIN_LED, HIGH);
  delay(200);

  // Apagado 800 ms
  digitalWrite(PIN_LED, LOW);
  delay(800);
}