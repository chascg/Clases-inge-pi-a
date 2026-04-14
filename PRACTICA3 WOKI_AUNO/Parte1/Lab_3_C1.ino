// Lab03_INT_Contador_LED.ino
volatile uint32_t pulso = 0;
volatile bool nuevo_dato = false;

void ISR_pulso() {
    pulso++;
    nuevo_dato = true;
}

void setup() {
    pinMode(2, INPUT_PULLUP);
    pinMode(13, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(2), ISR_pulso, FALLING);
    Serial.begin(9600);
    Serial.println(F("Lab 03 - Contador de pulsos (INT0)"));
    Serial.println(F("Presiona el boton en pin 2"));
}

void loop() {
    if (nuevo_dato) {
        nuevo_dato = false;
        noInterrupts();
        uint32_t copia_pulsos = pulso;
        interrupts();

        Serial.print(F("Pulso: "));
        Serial.println(copia_pulsos);

        // LED parpadea brevemente en cada pulso
        digitalWrite(13, HIGH);
        delay(50);
        digitalWrite(13, LOW);
    }
}