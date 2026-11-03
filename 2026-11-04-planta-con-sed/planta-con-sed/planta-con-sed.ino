// La planta que avisa cuando tiene sed
// El proyecto de fin de semana · Troxino.com
//
// Lectura del sensor: más alta = tierra más seca.
// Verde: bien · Amarillo: pronto toca regar · Rojo + chirrido: tiene sed.

const int PIN_SENSOR_VCC = 7;   // alimenta el módulo solo al medir
const int PIN_SENSOR     = A0;  // salida analógica del módulo (AO)
const int LED_VERDE      = 3;
const int LED_AMARILLO   = 4;
const int LED_ROJO       = 5;
const int PIN_BUZZER     = 9;

// Ajusta estos dos números con lo que veas en el Monitor Serie.
const int UMBRAL_PRONTO = 550;  // de aquí para arriba: amarillo
const int UMBRAL_SED    = 750;  // de aquí para arriba: rojo y chirrido

const unsigned long CADA_LECTURA = 2000;   // ms entre lecturas
const unsigned long CADA_AVISO   = 60000;  // ms entre chirridos mientras siga seca

unsigned long ultimoAviso = 0;
bool teniaSed = false;

// Prende el módulo, promedia 5 lecturas y lo vuelve a apagar.
// Así pasa corriente por la horquilla solo un instante y se corroe mucho menos.
int leerHumedad() {
  digitalWrite(PIN_SENSOR_VCC, HIGH);
  delay(10);  // deja que se estabilice

  long suma = 0;
  for (int i = 0; i < 5; i++) {
    suma += analogRead(PIN_SENSOR);
    delay(5);
  }

  digitalWrite(PIN_SENSOR_VCC, LOW);
  return suma / 5;
}

// Deja prendido solo el LED que le pases.
void mostrar(int led) {
  digitalWrite(LED_VERDE, led == LED_VERDE ? HIGH : LOW);
  digitalWrite(LED_AMARILLO, led == LED_AMARILLO ? HIGH : LOW);
  digitalWrite(LED_ROJO, led == LED_ROJO ? HIGH : LOW);
}

// Tres pitidos cortos y agudos, como un grillo.
void chirrido() {
  for (int i = 0; i < 3; i++) {
    tone(PIN_BUZZER, 2800, 60);
    delay(130);
  }
}

void setup() {
  pinMode(PIN_SENSOR_VCC, OUTPUT);
  digitalWrite(PIN_SENSOR_VCC, LOW);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  Serial.begin(9600);
  Serial.println("Planta lista. Lectura: alta = seca, baja = humeda.");

  // Prueba rápida al arrancar: los tres LEDs y un pitido.
  mostrar(LED_VERDE);    delay(250);
  mostrar(LED_AMARILLO); delay(250);
  mostrar(LED_ROJO);     delay(250);
  tone(PIN_BUZZER, 2000, 100);
}

void loop() {
  int lectura = leerHumedad();
  Serial.print("Lectura: ");
  Serial.println(lectura);

  if (lectura >= UMBRAL_SED) {
    mostrar(LED_ROJO);
    unsigned long ahora = millis();
    // Chirrea en cuanto se seca y luego cada minuto, no sin parar.
    if (!teniaSed || ahora - ultimoAviso >= CADA_AVISO) {
      chirrido();
      ultimoAviso = ahora;
    }
    teniaSed = true;
  } else if (lectura >= UMBRAL_PRONTO) {
    mostrar(LED_AMARILLO);
    teniaSed = false;
  } else {
    mostrar(LED_VERDE);
    teniaSed = false;
  }

  delay(CADA_LECTURA);
}
