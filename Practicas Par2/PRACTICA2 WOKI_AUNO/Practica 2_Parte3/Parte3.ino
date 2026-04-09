/* **********************************************************************
 * LABORATORIO 02 - Parte 3
 * ADC con registros directos (sin analogRead)
 * 
 * Conexiones:
 * - Potenciómetro 10kΩ: centro → A0, extremos → 5V y GND
 * 
 * Explicación de registros ADC:
 * - ADMUX: Selecciona referencia de voltaje y canal
 * - ADCSRA: Controla habilitación, prescaler e inicio de conversión
 * - ADC: Registro de 16 bits con el resultado (10 bits útiles)
 * ********************************************************************** */

void adc_init() {
    // === CONFIGURACIÓN DE ADMUX (Multiplexor ADC) ===
    // Bits: REFS1 REFS0 ADLAR MUX4 MUX3 MUX2 MUX1 MUX0
    // REFS1=0, REFS0=1 -> Vref = AVcc (5V)
    // ADLAR=0 -> Resultado ajustado a la derecha (10 bits en ADC[9:0])
    // MUX[3:0]=0000 -> Canal ADC0 (pin A0)
    ADMUX = (1 << REFS0);  // Solo REFS0=1, los demás bits 0
    
    // === CONFIGURACIÓN DE ADCSRA (Control y Status) ===
    // Bits: ADEN ADSC ADATE ADIF ADIE ADPS2 ADPS1 ADPS0
    // ADEN=1 -> Habilita el ADC
    // ADPS2=1, ADPS1=1, ADPS0=1 -> Prescaler = 128
    // Frecuencia ADC = 16MHz / 128 = 125kHz (dentro del rango 50-200kHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    // ADSC y ADATE se mantienen en 0 (conversión única, no automática)
    // ADIF y ADIE se mantienen en 0 (sin interrupción)
}

uint16_t adc_leer(uint8_t canal) {
    // Validar que el canal sea 0-7
    canal = canal & 0x07;  // Máscara para asegurar solo 3 bits (0-7)
    
    // 1. Seleccionar el canal en ADMUX
    // Limpiar bits MUX[3:0] (bits 0-3) y luego poner el canal
    ADMUX = (ADMUX & 0xF0) | (canal & 0x0F);
    // ADMUX & 0xF0: conserva bits superiores (REFS1, REFS0, ADLAR)
    // | (canal & 0x0F): escribe los 4 bits del canal en MUX[3:0]
    
    // 2. Iniciar conversión (ADSC = 1)
    ADCSRA |= (1 << ADSC);
    
    // 3. Esperar a que termine (ADSC vuelve a 0 automáticamente)
    while (ADCSRA & (1 << ADSC)) {
        // Espera activa (polling)
        // Podríamos añadir un timeout por seguridad
    }
    
    // 4. Leer el resultado del registro ADC (16 bits)
    // ADC es un registro de 16 bits: ADCH (alto) y ADCL (bajo)
    // La macro ADC lee ambos automáticamente
    return ADC;
}

void setup() {
    adc_init();          // Inicializar ADC con registros
    Serial.begin(9600);
    
    Serial.println(F("=== PARTE 3: ADC con REGISTROS ==="));
    Serial.println(F("Configuración:"));
    Serial.println(F("  Vref = AVcc (5V)"));
    Serial.println(F("  Prescaler = 128 -> 125 kHz"));
    Serial.println(F("  Canal = A0 (PC0)"));
    Serial.println(F(""));
    Serial.println(F("Gira el potenciómetro y observa los valores"));
    Serial.println(F("============================================\n"));
    Serial.println(F("Canal\tValor ADC\tVoltaje (V)"));
    Serial.println(F("-----\t---------\t-----------"));
}

void loop() {
    // Leer el canal ADC0 (pin A0)
    uint16_t valorADC = adc_leer(0);
    
    // Calcular voltaje: V = (valorADC * 5.0V) / 1023
    // Usar 1023.0 para convertir a float correctamente
    float voltaje = valorADC * (5.0f / 1023.0f);
    
    // Mostrar resultados en el Monitor Serial
    Serial.print(F("A0\t"));
    Serial.print(valorADC);
    Serial.print(F("\t\t"));
    Serial.print(voltaje, 3);  // 3 decimales
    Serial.println(F(" V"));
    
    delay(200);  // Pequeña pausa para lectura cómoda
}