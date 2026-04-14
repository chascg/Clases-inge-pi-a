// Lab03_Encoder_Rotatorio.ino
const int PIN_A = 2;
const int PIN_B = 3;
const int PIN_BTN = 4;

volatile int32_t posicion = 0;
volatile bool btn_click = false;

void ISR_encoder() {
    if (digitalRead(PIN_B) == HIGH) {
        posicion++;
    } else {
        posicion--;
    }
}

void ISR_boton() {
    btn_click = true;
}

void setup() {
    pinMode(PIN_A, INPUT_PULLUP);
    pinMode(PIN_B, INPUT_PULLUP);
    pinMode(PIN_BTN, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(PIN_A), ISR_encoder, FALLING);
    attachInterrupt(digitalPinToInterrupt(PIN_BTN), ISR_boton, FALLING);

    Serial.begin(9600);
    Serial.println(F("Encoder listo. Gira o presiona."));
}

int32_t pos_anterior = 1;

void loop() {
    noInterrupts();
    int32_t pos_local = posicion;
    interrupts();

    if (pos_local != pos_anterior) {
        Serial.print(F("Posicion: "));
        Serial.println(pos_local);
        pos_anterior = pos_local;
    }

    if (btn_click) {
        btn_click = false;
        posicion = 0;
        Serial.println(F("RESET -> posicion = 0"));
    }
}