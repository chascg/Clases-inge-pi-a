/* **********************************************************************
 * LABORATORIO 02 - Parte 1
 * GPIO con registros directos del ATmega328P

 * Conexiones:
 * - Pin 13 (PB5) -> LED + 220Ω > GND
 * - Pin 2  (PD2) -> Botón -> GND (usa pull-up interno)
 * ********************************************************************** */

void setup() {
    Serial.begin(9600);
    
    // === CONFIGURACIÓN DE PINES ===
    
    // 1. Configurar PB5 (pin 13) como SALIDA
    // Equivalente a: pinMode(13, OUTPUT)
    DDRB |= (1 << DDB5);     // DDB5 es el bit 5 del registro DDRB
    // (1 << DDB5) = 0b00100000
    // DDRB |= 0b00100000  -> pone el bit 5 en 1 (salida)
    
    // 2. Configurar PD2 (pin 2) como ENTRADA con pull-up
    // Equivalente a: pinMode(2, INPUT_PULLUP)
    DDRD &= ~(1 << DDD2);    // DDD2 = 0 -> entrada (clear bit 2)
    PORTD |= (1 << PD2);     // PD2 = 1 -> activa pull-up interno
    
    // === VERIFICACIÓN POR MONITOR SERIAL ===
    Serial.println(F("=== PARTE 1: GPIO con REGISTROS ==="));
    Serial.println(F("Configuraciones:"));
    Serial.print(F("DDRB = 0b")); Serial.println(DDRB, BIN);
    Serial.print(F("DDRD = 0b")); Serial.println(DDRD, BIN);
    Serial.print(F("PORTD = 0b")); Serial.println(PORTD, BIN);
    Serial.println(F(""));
    Serial.println(F("Presiona el botón (pin 2) para encender el LED"));
    Serial.println(F("============================================\n"));
}

void loop() {
    // === LECTURA DEL BOTÓN CON REGISTRO PIND ===
    // PIND2 es el bit 2 del registro PIND
    // Si el botón está presionado, el pin se conecta a GND -> nivel LOW
    // !(PIND & (1 << PIND2)) es TRUE cuando el pin está en LOW
    
    if (!(PIND & (1 << PIND2))) {   // Botón presionado (LOW)
        // Encender LED: poner bit 5 de PORTB en 1
        PORTB |= (1 << PORTB5);     // PORTB5 = bit 5
        Serial.println(F("Botón: PRESIONADO → LED ON"));
    } 
    else {                           // Botón no presionado (HIGH por pull-up)
        // Apagar LED: poner bit 5 de PORTB en 0
        PORTB &= ~(1 << PORTB5);    // Clear bit 5
    }
    
    delay(50);  // Pequeño debounce
}