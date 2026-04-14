// Lab04_Timer1_CTC_Reloj.ino
// Timer1 modo CTC - interrupciones precisas cada 1 segundo
// Implementa un reloj HH:MM:SS con LED parpadeante

volatile uint8_t segundos = 0;
volatile uint8_t minutos = 0;
volatile uint8_t horas = 0;
volatile bool tick = false;

ISR(TIMER1_COMPA_vect) {
    tick = true;
    segundos++;
    if (segundos >= 60) { 
        segundos = 0; 
        minutos++; 
    }
    if (minutos >= 60) { 
        minutos = 0; 
        horas++; 
    }
    if (horas >= 24) { 
        horas = 0; 
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("Reloj software con Timer1 CTC"));
    Serial.println(F("Formato HH:MM:SS"));
    
    pinMode(13, OUTPUT);  // LED indicador de segundos
    
    cli();                // Deshabilitar interrupciones
    TCCR1A = 0;           // Limpiar registro A
    TCCR1B = 0;           // Limpiar registro B
    TCNT1 = 0;            // Resetear contador
    
    // OCR1A = (16000000 / (1024 * 1)) - 1 = 15624
    OCR1A = 15624;
    TCCR1B |= (1 << WGM12);                    // Modo CTC
    TCCR1B |= (1 << CS12) | (1 << CS10);       // Prescaler 1024
    TIMSK1 |= (1 << OCIE1A);                   // Habilitar interrupción por comparación
    sei();               // Rehabilitar interrupciones
}

void loop() {
    if (tick) {
        tick = false;
        
        // Parpadeo LED cada segundo
        digitalWrite(13, !digitalRead(13));
        
        // Imprimir reloj HH:MM:SS
        if (horas < 10) Serial.print('0');
        Serial.print(horas);
        Serial.print(':');
        if (minutos < 10) Serial.print('0');
        Serial.print(minutos);
        Serial.print(':');
        if (segundos < 10) Serial.print('0');
        Serial.println(segundos);
    }
}