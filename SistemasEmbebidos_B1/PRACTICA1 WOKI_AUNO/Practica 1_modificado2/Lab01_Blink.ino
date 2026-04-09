const int PIN_LED = 13;

void setup() {
  pinMode(PIN_LED, OUTPUT);
}

void loop() {

  // --- S (3 cortos) ---
  for(int i = 0; i < 3; i++){
    digitalWrite(PIN_LED, HIGH);
    delay(200);
    digitalWrite(PIN_LED, LOW);
    delay(200);
  }

  // --- O (3 largos) ---
  for(int i = 0; i < 3; i++){
    digitalWrite(PIN_LED, HIGH);
    delay(600);
    digitalWrite(PIN_LED, LOW);
    delay(200);
  }

  // --- S (3 cortos) ---
  for(int i = 0; i < 3; i++){
    digitalWrite(PIN_LED, HIGH);
    delay(200);
    digitalWrite(PIN_LED, LOW);
    delay(200);
  }

  // Pausa entre repeticiones
  delay(2000);
}