// El dado electrónico
// El proyecto de fin de semana · Troxino.com
//
// Aprietas el botón, el display gira un momento y se queda en un número del 1 al 6.
// Cada tirada se cuenta y se manda al Monitor Serie para ver si el dado es justo.

const int SEGMENTOS[7] = {2, 3, 4, 5, 6, 7, 8};  // a, b, c, d, e, f, g (a D2 ... D8)
const int PIN_BOTON    = 9;                      // al otro lado del botón va GND
const int PIN_BUZZER   = 10;

// Qué segmentos prenden para cada número (1 = prendido), en orden a b c d e f g.
const byte NUMEROS[7][7] = {
  {0, 0, 0, 0, 0, 0, 0},  // 0: apagado
  {0, 1, 1, 0, 0, 0, 0},  // 1
  {1, 1, 0, 1, 1, 0, 1},  // 2
  {1, 1, 1, 1, 0, 0, 1},  // 3
  {0, 1, 1, 0, 0, 1, 1},  // 4
  {1, 0, 1, 1, 0, 1, 1},  // 5
  {1, 0, 1, 1, 1, 1, 1}   // 6
};

unsigned long cuenta[7];  // cuenta[1] = cuántas veces salió el 1, etc.
unsigned long tiradas = 0;

// Muestra un número (0 apaga el display).
void mostrar(int numero) {
  for (int i = 0; i < 7; i++) {
    digitalWrite(SEGMENTOS[i], NUMEROS[numero][i] ? HIGH : LOW);
  }
}

// El display "gira" por números al azar, cada vez más lento, y se queda en uno.
int tirar() {
  int final = random(1, 7);
  int espera = 40;
  while (espera < 300) {
    mostrar(random(1, 7));
    tone(PIN_BUZZER, 1800, 15);
    delay(espera);
    espera += 25;
  }
  mostrar(final);
  tone(PIN_BUZZER, 2600, 120);
  return final;
}

// Una línea por número: cuántas veces salió y una barrita para compararlas.
void mostrarCuenta() {
  Serial.print("Tirada ");
  Serial.println(tiradas);
  for (int n = 1; n <= 6; n++) {
    Serial.print(n);
    Serial.print(": ");
    Serial.print(cuenta[n]);
    Serial.print(" ");
    for (unsigned long i = 0; i < cuenta[n]; i++) {
      Serial.print("#");
    }
    Serial.println();
  }
}

void setup() {
  for (int i = 0; i < 7; i++) {
    pinMode(SEGMENTOS[i], OUTPUT);
  }
  pinMode(PIN_BOTON, INPUT_PULLUP);  // sin apretar = HIGH, apretado = LOW
  pinMode(PIN_BUZZER, OUTPUT);

  randomSeed(analogRead(A0));  // A0 sin conectar: ruido para variar la semilla
  Serial.begin(9600);
  Serial.println("Dado listo. Aprieta el boton.");

  // Prueba al arrancar: del 1 al 6.
  for (int n = 1; n <= 6; n++) {
    mostrar(n);
    delay(200);
  }
  mostrar(0);
}

void loop() {
  if (digitalRead(PIN_BOTON) == LOW) {
    randomSeed(micros());  // el momento exacto en que aprietas también cuenta
    int resultado = tirar();
    cuenta[resultado]++;
    tiradas++;
    mostrarCuenta();
    while (digitalRead(PIN_BOTON) == LOW) {
      delay(10);  // espera a que sueltes el botón
    }
    delay(50);
  }
}
