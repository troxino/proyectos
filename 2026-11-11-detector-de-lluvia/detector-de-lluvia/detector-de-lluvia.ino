// El detector de lluvia
// El proyecto de fin de semana · Troxino.com
//
// Caen gotas en el sensor: suena la alarma "¡mete la ropa!", el LED se pone azul
// y el servo cierra un techo de cartón sobre el tendedero.
// Si llueve fuerte (aguacero), el LED parpadea en morado y suena otra alarma.
// Cuando lleva 10 segundos seco, el LED vuelve a verde y el techo se abre.
//
// Lectura del sensor: 0 = seco, más alta = más agua sobre las pistas.

#include <Servo.h>

const int PIN_SENSOR_VCC = 7;   // alimenta el sensor solo al medir
const int PIN_SENSOR     = A0;  // pin S del sensor
const int LED_ROJO       = 3;   // LED RGB de cátodo común
const int LED_VERDE      = 5;
const int LED_AZUL       = 6;
const int PIN_SERVO      = 9;   // cable naranja del servo
const int PIN_BUZZER     = 10;

// Ajusta estos dos números con lo que veas en el Monitor Serie.
const int UMBRAL_LLUVIA   = 150;  // de aquí para arriba: está lloviendo
const int UMBRAL_AGUACERO = 450;  // de aquí para arriba: aguacero

// Ángulos del servo. Si tu techo queda al revés, intercámbialos.
const int TECHO_ABIERTO = 0;
const int TECHO_CERRADO = 90;

const unsigned long CADA_LECTURA     = 500;    // ms entre lecturas
const unsigned long TIEMPO_PARA_SECO = 10000;  // ms seguidos sin lluvia para abrir el techo

const int SECO     = 0;
const int LLUVIA   = 1;
const int AGUACERO = 2;

Servo techo;
int posicionTecho = TECHO_ABIERTO;
int estado = SECO;
unsigned long ultimaGota = 0;
bool parpadeo = false;

// Prende el sensor, promedia 5 lecturas y lo vuelve a apagar.
// Así pasa corriente por las pistas solo un instante y se corroen mucho menos.
int leerSensor() {
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

// Prende cada color del LED RGB (1 = prendido, 0 = apagado).
void color(int rojo, int verde, int azul) {
  digitalWrite(LED_ROJO, rojo ? HIGH : LOW);
  digitalWrite(LED_VERDE, verde ? HIGH : LOW);
  digitalWrite(LED_AZUL, azul ? HIGH : LOW);
}

// Mueve el techo poco a poco: se ve mejor y le pide menos corriente al USB.
void moverTecho(int destino) {
  int paso = destino > posicionTecho ? 1 : -1;
  while (posicionTecho != destino) {
    posicionTecho += paso;
    techo.write(posicionTecho);
    delay(12);
  }
}

// "¡Mete la ropa!": tres notas que suben, dos veces.
void alarmaLluvia() {
  for (int vez = 0; vez < 2; vez++) {
    tone(PIN_BUZZER, 1319, 150);  // Mi
    delay(180);
    tone(PIN_BUZZER, 1568, 150);  // Sol
    delay(180);
    tone(PIN_BUZZER, 2093, 300);  // Do agudo
    delay(450);
  }
}

// Aguacero: seis pitidos rápidos.
void alarmaAguacero() {
  for (int i = 0; i < 6; i++) {
    tone(PIN_BUZZER, 2600, 70);
    delay(120);
  }
}

// Convierte la lectura en SECO, LLUVIA o AGUACERO.
int clasificar(int lectura) {
  if (lectura >= UMBRAL_AGUACERO) {
    return AGUACERO;
  }
  if (lectura >= UMBRAL_LLUVIA) {
    return LLUVIA;
  }
  return SECO;
}

void setup() {
  pinMode(PIN_SENSOR_VCC, OUTPUT);
  digitalWrite(PIN_SENSOR_VCC, LOW);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  techo.attach(PIN_SERVO);
  techo.write(TECHO_ABIERTO);

  Serial.begin(9600);

  // Prueba rápida al arrancar: rojo, verde, azul y un pitido.
  color(1, 0, 0); delay(300);
  color(0, 1, 0); delay(300);
  color(0, 0, 1); delay(300);
  tone(PIN_BUZZER, 2000, 100);
  color(0, 1, 0);

  Serial.println("Detector de lluvia listo. Lectura: 0 = seco, alta = mojado.");
}

void loop() {
  int lectura = leerSensor();
  int nivel = clasificar(lectura);

  Serial.print("Lectura: ");
  Serial.println(lectura);

  if (nivel == SECO) {
    // Solo abre el techo cuando lleva un rato seco, no con cada gota que se escurre.
    if (estado != SECO && millis() - ultimaGota >= TIEMPO_PARA_SECO) {
      estado = SECO;
      color(0, 1, 0);
      Serial.println(">> Ya paro. Abro el techo.");
      moverTecho(TECHO_ABIERTO);
    }
  } else {
    ultimaGota = millis();

    if (estado == SECO) {
      estado = LLUVIA;
      color(0, 0, 1);
      Serial.println(">> Esta lloviendo: mete la ropa. Cierro el techo.");
      alarmaLluvia();
      moverTecho(TECHO_CERRADO);
    }

    if (nivel == AGUACERO && estado != AGUACERO) {
      estado = AGUACERO;
      Serial.println(">> Aguacero.");
      alarmaAguacero();
    } else if (nivel == LLUVIA && estado == AGUACERO) {
      estado = LLUVIA;
      Serial.println(">> Bajo a lluvia.");
    }
  }

  // Azul fijo con lluvia; morado que parpadea con aguacero.
  if (estado == LLUVIA) {
    color(0, 0, 1);
  } else if (estado == AGUACERO) {
    parpadeo = !parpadeo;
    color(parpadeo, 0, parpadeo);
  }

  delay(CADA_LECTURA);
}
