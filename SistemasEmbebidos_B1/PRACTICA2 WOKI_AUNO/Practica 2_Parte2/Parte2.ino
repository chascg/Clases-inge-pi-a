// Lab02_Parte2_Benchmark.ino
// Solo LED en pin 13 - NO necesita botón

void setup() {
  // Configurar pin 13 como salida usando registro
  DDRB |= (1 << 5);  // PB5 = pin 13
  
  Serial.begin(9600);
  Serial.println("=== PARTE 2: BENCHMARK ===");
  Serial.println("LED en pin 13 parpadeando a maxima velocidad");
  Serial.println("(Parecera que esta siempre encendido)");
}

void loop() {
  // Máxima velocidad de conmutación
  PORTB |= (1 << 5);   // LED ON
  PORTB &= ~(1 << 5);  // LED OFF
  // Esto se ejecuta a ~4 MHz!
}