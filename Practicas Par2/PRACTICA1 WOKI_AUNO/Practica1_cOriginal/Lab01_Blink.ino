// Lab01_Blink.ino
// Parpadeo de LED en pin 13 con mensajes en Monitor Serial

const int PIN_LED = 13;  // Pin del LED
int contador = 0;        // Cuenta los parpadeos

void setup() {
  pinMode(PIN_LED, OUTPUT);   // Configurar pin como salida
  
  Serial.begin(9600);         // Iniciar comunicación serial
  Serial.println("=== Lab 01: Blink Iniciado ===");
  Serial.println("Formato: [#] LED -> ESTADO");
}

void loop() {
  contador++;

  // Encender LED
  digitalWrite(PIN_LED, HIGH);
  Serial.print("[");
  Serial.print(contador);
  Serial.println("] LED -> ENCENDIDO");
  delay(500);  // Esperar 500 ms

  // Apagar LED
  digitalWrite(PIN_LED, LOW);
  Serial.print("[");
  Serial.print(contador);
  Serial.println("] LED -> APAGADO");
  delay(500);  // Esperar 500 ms
}
