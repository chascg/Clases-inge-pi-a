// Lab04_Timer1_Tonos_Musicales.ino
// Usa Timer1 en modo CTC para generar notas musicales precisas
// Reproduce una melodía simple: "Cumpleaños Feliz"

// Frecuencias de notas musicales (Hz)
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_REST 0

// Melodía: Cumpleaños Feliz
const uint16_t melody[] = {
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_F4, NOTE_E4,  // Cum-ple-a-ños fe-liz
    NOTE_C4, NOTE_C4, NOTE_D4, NOTE_C4, NOTE_G4, NOTE_F4,  // Cum-ple-a-ños fe-liz
    NOTE_C4, NOTE_C4, NOTE_C5, NOTE_A4, NOTE_F4, NOTE_E4, NOTE_D4,  // Cum-ple-a-ños que-rí-do
    NOTE_B4, NOTE_B4, NOTE_A4, NOTE_F4, NOTE_G4, NOTE_F4   // Cum-ple-a-ños fe-liz
};

const uint16_t noteDurations[] = {
    500, 500, 500, 500, 500, 1000,
    500, 500, 500, 500, 500, 1000,
    500, 500, 500, 500, 500, 500, 1000,
    500, 500, 500, 500, 500, 1000
};

const int melodyLength = sizeof(melody) / sizeof(melody[0]);
volatile bool notePlaying = false;
volatile int currentNote = 0;

void setupTimer1Tone(uint16_t frequency) {
    if (frequency == 0) {
        TCCR1A = 0;  // Deshabilitar salida
        return;
    }
    
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    
    // Modo CTC con toggle en OC1A (pin 9)
    TCCR1A |= (1 << COM1A0);  // Toggle OC1A on compare match
    
    // Calcular OCR1A para la frecuencia deseada
    // Frecuencia de toggle = frecuencia_nota * 2 (porque toggle cambia cada medio ciclo)
    uint32_t ocr_value = (16000000UL / (1 * frequency * 2)) - 1;
    OCR1A = (uint16_t)ocr_value;
    
    TCCR1B |= (1 << WGM12);   // Modo CTC
    TCCR1B |= (1 << CS10);    // Prescaler 1 (sin prescaler)
    TIMSK1 |= (1 << OCIE1A);  // Habilitar interrupción
    
    // Configurar pin 9 como salida
    DDRB |= (1 << DDB1);      // Pin 9 = OC1A
    sei();
}

void playMelody() {
    Serial.println(F("Reproduciendo: Cumpleaños Feliz"));
    
    for (int i = 0; i < melodyLength; i++) {
        currentNote = i;
        setupTimer1Tone(melody[i]);
        delay(noteDurations[i]);
        
        // Pequeña pausa entre notas
        setupTimer1Tone(0);
        delay(50);
    }
    
    setupTimer1Tone(0);  // Silencio al final
    Serial.println(F("Melodía completada!"));
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("=== Generador de Tonos con Timer1 ==="));
    Serial.println(F("Comandos:"));
    Serial.println(F("  'play' - Reproducir melodía"));
    Serial.println(F("  'fXXX'  - Tocar frecuencia específica (ej: f440)"));
    Serial.println(F("  'stop'  - Detener tono"));
    
    pinMode(9, OUTPUT);
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input.trim();
        
        if (input == "play") {
            playMelody();
        } else if (input == "stop") {
            setupTimer1Tone(0);
            Serial.println(F("Tono detenido"));
        } else if (input[0] == 'f' || input[0] == 'F') {
            uint16_t freq = input.substring(1).toInt();
            if (freq > 0 && freq <= 20000) {
                setupTimer1Tone(freq);
                Serial.print(F("Reproduciendo: "));
                Serial.print(freq);
                Serial.println(F(" Hz"));
            }
        }
    }
}