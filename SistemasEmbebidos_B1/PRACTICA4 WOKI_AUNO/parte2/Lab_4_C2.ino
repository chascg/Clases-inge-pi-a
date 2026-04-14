// Lab04_PWM_Frecuencia_Timer2.ino
// Generar PWM en pin 11 (OC2A) con Timer2
// Frecuencia y duty cycle configurables por Serial

void pwm_init(uint16_t frecuencia_hz, uint8_t duty_0_255) {
    // Calcular OCR para modo Fast PWM, prescaler 64
    // OCR2A = (F_CPU / (prescaler * frecuencia)) - 1
    uint8_t ocr = (uint8_t)((16000000UL / (64UL * frecuencia_hz)) - 1);
    
    TCCR2A = (1 << COM2A1) | (1 << WGM21) | (1 << WGM20);  // Fast PWM, no-inverting
    TCCR2B = (1 << CS22);                                   // Prescaler 64
    OCR2A = ocr;                                            // Periodo (frecuencia)
    OCR2B = duty_0_255;                                     // Duty cycle
    DDRB |= (1 << DDB3);                                    // Pin 11 (OC2A) como salida
}

void setup() {
    Serial.begin(9600);
    pwm_init(1000, 128);  // 1 kHz, 50% duty cycle
    
    Serial.println(F("=== Control PWM Timer2 ==="));
    Serial.println(F("PWM activo en pin 11 - 1 kHz, 50%"));
    Serial.println(F("Comandos:"));
    Serial.println(F("  Enviar numero 0-255: Cambiar duty cycle"));
    Serial.println(F("  Enviar 'fXXXX': Cambiar frecuencia (ej: f2000)"));
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        if (input.length() > 0) {
            if (input[0] == 'f' || input[0] == 'F') {
                // Cambiar frecuencia
                uint16_t nueva_frec = input.substring(1).toInt();
                if (nueva_frec > 0 && nueva_frec <= 10000) {
                    uint8_t duty_actual = OCR2B;
                    pwm_init(nueva_frec, duty_actual);
                    Serial.print(F("Frecuencia cambiada a: "));
                    Serial.print(nueva_frec);
                    Serial.println(F(" Hz"));
                } else {
                    Serial.println(F("Error: Frecuencia debe ser 1-10000 Hz"));
                }
            } else {
                // Cambiar duty cycle
                uint8_t nuevo_duty = input.toInt();
                if (nuevo_duty <= 255) {
                    OCR2B = nuevo_duty;
                    Serial.print(F("Duty cycle: "));
                    Serial.print(nuevo_duty);
                    Serial.print(F(" ("));
                    Serial.print((nuevo_duty * 100) / 255);
                    Serial.println(F("%)"));
                }
            }
        }
    }
}