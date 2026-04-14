#include <Servo.h>

// Pines
#define BTN1 2
#define BTN2 3
#define BTN3 4
#define BTN4 5
#define LED_VERDE 7
#define LED_ROJO 8
#define LED_AMARILLO 12
#define BUZZER 6
#define SERVO_PIN 9

// Codigo correcto
const int codigoCorrecto[4] = {1, 2, 3, 4};

// Estados
enum Estado { ESPERA, INGRESANDO, VERIFICANDO, ABIERTO, BLOQUEADO_CORTO, BLOQUEADO_LARGO };
Estado estado = ESPERA;

int intento[4];
int indice = 0;
int fallosConsecutivos = 0;
int intentoNumero = 0;

unsigned long tiempoInicio = 0;
const int TIEMPO_ABIERTO = 5000;
const int TIEMPO_BLOQUEO_CORTO = 5000;
const int TIEMPO_BLOQUEO_LARGO = 30000;

Servo servo;

//  Control de flanco
bool estadoAnterior[4] = {false, false, false, false};

bool leerBoton(int pin, int idx) {
  bool lectura = digitalRead(pin);

  // Detecta solo cuando presiona
  if (lectura && !estadoAnterior[idx]) {
    estadoAnterior[idx] = true;
    return true;
  }

  if (!lectura) {
    estadoAnterior[idx] = false;
  }

  return false;
}

void setup() {
  Serial.begin(9600);

  pinMode(BTN1, INPUT);
  pinMode(BTN2, INPUT);
  pinMode(BTN3, INPUT);
  pinMode(BTN4, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  servo.attach(SERVO_PIN);
  servo.write(0);

  Serial.println("Sistema listo");
}

void loop() {

  switch(estado) {

    case ESPERA:
      indice = 0;
      for(int i=0;i<4;i++) intento[i]=0;

      digitalWrite(LED_ROJO, LOW);
      digitalWrite(LED_VERDE, LOW);

      estado = INGRESANDO;
    break;

    case INGRESANDO:
      // Parpadeo led amarillo
      static unsigned long lastBlink = 0;
      if (millis() - lastBlink > 300) {
        digitalWrite(LED_AMARILLO, !digitalRead(LED_AMARILLO));
        lastBlink = millis();
      }

      if (leerBoton(BTN1,0)) { intento[indice]=1; indice++; }
      else if (leerBoton(BTN2,1)) { intento[indice]=2; indice++; }
      else if (leerBoton(BTN3,2)) { intento[indice]=3; indice++; }
      else if (leerBoton(BTN4,3)) { intento[indice]=4; indice++; }

      if (indice == 4) estado = VERIFICANDO;
    break;

    case VERIFICANDO: {
      bool ok = true;

      for(int i=0;i<4;i++){
        if(intento[i] != codigoCorrecto[i]) ok = false;
      }

      intentoNumero++;

      Serial.print("Intento ");
      Serial.print(intentoNumero);
      Serial.print(": ");

      for(int i=0;i<4;i++) Serial.print(intento[i]);

      if (ok) {
        Serial.println(" -> CORRECTO");

        fallosConsecutivos = 0;
        estado = ABIERTO;
        tiempoInicio = millis();

        servo.write(90);
        digitalWrite(LED_VERDE, HIGH);

        tone(BUZZER, 1000, 200);
        delay(250);
        tone(BUZZER, 1500, 200);

      } else {
        Serial.println(" -> INCORRECTO");

        fallosConsecutivos++;
        digitalWrite(LED_ROJO, HIGH);

        for(int i=0;i<3;i++){
          tone(BUZZER, 300, 200);
          delay(300);
        }
        noTone(BUZZER);

        tiempoInicio = millis();

        if (fallosConsecutivos >= 3) {
          estado = BLOQUEADO_LARGO;
        } else {
          estado = BLOQUEADO_CORTO;
        }
      }
    } break;

    case ABIERTO:
      if (millis() - tiempoInicio >= TIEMPO_ABIERTO) {
        servo.write(0);
        digitalWrite(LED_VERDE, LOW);
        estado = ESPERA;
      }
    break;

    case BLOQUEADO_CORTO:
      if (millis() - tiempoInicio >= TIEMPO_BLOQUEO_CORTO) {
        digitalWrite(LED_ROJO, LOW);
        estado = ESPERA;
      }
    break;

    case BLOQUEADO_LARGO:
      digitalWrite(LED_ROJO, HIGH);

      if (millis() - tiempoInicio >= TIEMPO_BLOQUEO_LARGO) {
        digitalWrite(LED_ROJO, LOW);
        fallosConsecutivos = 0;
        estado = ESPERA;
      }
    break;

  }
}