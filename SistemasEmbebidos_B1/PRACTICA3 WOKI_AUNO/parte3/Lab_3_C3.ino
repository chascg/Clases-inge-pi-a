// Lab03_PCINT_LED_Toggle.ino
// Usa Pin Change Interrupt en PCINT0 (pin 8)
// Cada pulsación cambia el estado del LED en pin 9

volatile bool toggle = false;
const byte PIN_LED = 9;
const byte PIN_BTN = 8;

// ISR para PCINT0 (pines 8-13)
ISR(PCINT0_vect) {
    static unsigned long last = 0;
    unsigned long now = millis();
    if (now - last > 150) { // anti-rebote simple
        toggle = !toggle;
        last = now;
    }
}

void setup() {
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_BTN, INPUT_PULLUP);

    // Habilitar PCINT para pin 8
    PCMSK0 |= (1 << PCINT0); // PCINT0 corresponde a pin 8
    PCICR |= (1 << PCIE0);   // Habilitar interrupciones del grupo PCINT0

    Serial.begin(9600);
    Serial.println(F("PCINT en pin 8 listo. LED en pin 9 cambia con cada pulsacion."));
}

void loop() {
    static bool last_toggle = false;
    if (toggle != last_toggle) {
        last_toggle = toggle;
        digitalWrite(PIN_LED, toggle);
        Serial.print(F("LED: "));
        Serial.println(toggle ? "ON" : "OFF");
    }
}