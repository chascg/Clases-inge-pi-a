const int PIN_LED = 13;
int contador = 0;

void setup() {
  pinMode(PIN_LED, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  contador++;

  // Encender LED
  digitalWrite(PIN_LED, HIGH);
  Serial.print("[#");
  Serial.print(contador);
  Serial.print("] t=");
  Serial.print(millis());
  Serial.println(" ms LED -> ENCENDIDO");
  delay(500);

  // Apagar LED
  digitalWrite(PIN_LED, LOW);
  Serial.print("[#");
  Serial.print(contador);
  Serial.print("] t=");
  Serial.print(millis());
  Serial.println(" ms LED -> APAGADO");
  delay(500);
}