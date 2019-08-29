#include <EEPROM.h>

int rueda_Izquierda_T = 2;  //rueda_Izquierda_Tierra
int rueda_Izquierda_C = 3;  //rueda_Izquierda_Corriente
int rueda_Derecha_C = 4;    //rueda_Derecha_Corriente
int rueda_Derecha_T = 5;    //rueda_Derecha_Tierra

//ultrasonico
const int EchoPin = 7;
const int TriggerPin = 6;
const int LedPin = 13;

//sensor de color
const int s0 = 43;
const int s1 = 41;
const int s2 = 24;
const int s3 = 25;
const int out = 26;
int redLed = 2;
int greenLed = 3;
int blueLed = 4;
// Variables
int red = 0;
int green = 0;
int blue = 0;
bool Play = true;   //modo automatico

//estructura ruta
struct ObjRuta {
  char name_[4];
  char movimientos[500];
};
int movimientos = 0;
ObjRuta ruta1;
ObjRuta ruta2;
ObjRuta tmp;
int rutasguardadas = 0;

bool estadonombre = false;
bool estadomanual = false;
bool estadorecorrer = false;
int c_nombre = 0;
char tmp_nombre[8];

void setup() {
  rutasguardadas = EEPROM.read(0);    //verifico cuantas rutas hemops guardado

  memset(&tmp_nombre, '\0', sizeof(tmp_nombre));

  Serial.begin(9600);
  Serial.print("MODULO CONECTADO ");
  Serial.println("#");

  //ruedas
  pinMode(rueda_Izquierda_T, OUTPUT);
  pinMode(rueda_Izquierda_C, OUTPUT);
  pinMode(rueda_Derecha_T, OUTPUT);
  pinMode(rueda_Derecha_C, OUTPUT);

  //sensor de color
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);
  pinMode(out, INPUT);
  digitalWrite(s0, HIGH);
  digitalWrite(s1, HIGH);

  //ULTRASONICO
  pinMode(EchoPin, INPUT);
  pinMode(TriggerPin, OUTPUT);

  //LimpiarRutas();
}

void loop() {

  if (Serial.available())
  {
    char VarChar = Serial.read();
    //Serial.println(VarChar);
    Apagar();

    if (VarChar == '0') {
      //iniciamos modo manual.

      estadomanual = true;
      estadonombre = false;
      movimientos = 0;  //iniciaalizamos los movimientos en 0
      memset(&tmp, 0, sizeof(tmp)); //limpiamos el temporal.

    }
    else if (VarChar == '1' && estadomanual == false)
    {
      //iniciamos modo automatico
      Automatico();
    }
    else if (VarChar == '1' && estadomanual == true )
    {
      //ya voy a guardar la ruta FALTA LA CANCELACION DE GUARDAR
      EstadoNombre();
      estadomanual = false;
    }
    else if (VarChar == '2')
    {
      Adelante();
      delay(100);
      Apagar();
      delay(50);

      tmp.movimientos[movimientos] = '2';
      movimientos++;
    }
    else if (VarChar == '3')
    {
      Atras();
      delay(100);
      Apagar();
      delay(50);

      tmp.movimientos[movimientos] = '3';
      movimientos++;
    }
    else if (VarChar == '4')
    {
      Derecha();
      delay(100);
      Apagar();
      delay(50);

      tmp.movimientos[movimientos] = '4';
      movimientos++;
    }
    else if (VarChar == '5')
    {
      Izquierda();
      delay(100);
      Apagar();
      delay(50);

      tmp.movimientos[movimientos] = '5';
      movimientos++;
    }
    else if (VarChar == '6') {
      //limpiar rutas
      LimpiarRutas();
      rutasguardadas = 0;
      EEPROM.write(0, rutasguardadas);     //guardo cuantas rutas tengo guardas
      Serial.println("Rutas Limpiadas.#");
    }
    else if (VarChar == '8') {
      //estado recorrer
      Serial.println("Realizar Recorrido#");
      EstadoRecorrer();
    }
    else if (VarChar == '7') {
      //ver rutas
      memset(&ruta1, 0, sizeof(ruta1));
      memset(&ruta2, 0, sizeof(ruta2));
      EEPROM.get(0 + sizeof(int), ruta1);
      EEPROM.get(0 + sizeof(int) + sizeof(ObjRuta), ruta2);

      if (strcasecmp(ruta1.name_, "") == 0  && strcasecmp(ruta2.name_, "") == 0) {
        Serial.println("#");
      } else {
        Serial.print(ruta1.name_[0]);
        Serial.print(ruta1.name_[1]);
        Serial.print(ruta1.name_[2]);
        Serial.print(ruta1.name_[3]);
        Serial.print("-");
        Serial.print(ruta2.name_[0]);
        Serial.print(ruta2.name_[1]);
        Serial.print(ruta2.name_[2]);
        Serial.print(ruta2.name_[3]);
        Serial.println("#");
      }
      memset(&ruta1, 0, sizeof(ruta1));
      memset(&ruta2, 0, sizeof(ruta2));
    }
    else {

    }
  }
}

void EstadoNombre() {

  char tmp_nombre[4];
  memset(&tmp_nombre, '\0', sizeof(tmp_nombre));
  int indice = 0;
  int contador = 0;
  while (true) {
    if (Serial.available())
    {
      tmp_nombre[indice] = Serial.read();
      indice++;
      if (indice == 4 ) {
        break;
      }
    }
    contador++;
  }

  memset(&tmp.name_, '\0', sizeof(tmp.name_)); //limpiamos el temporal.
  tmp.name_[0] = tmp_nombre[0];
  tmp.name_[1] = tmp_nombre[1];
  tmp.name_[2] = tmp_nombre[2];
  tmp.name_[3] = tmp_nombre[3];

  //escribimos el struct
  if (rutasguardadas < 2) {
    EEPROM.put(0 + sizeof(int) + sizeof(ObjRuta)*rutasguardadas, tmp);
    rutasguardadas++;
    EEPROM.write(0, rutasguardadas);     //guardo cuantas rutas tengo guardas

    //lectura
    rutasguardadas = EEPROM.read(0);    //verifico cuantas rutas hemops guardado
    Serial.print("Rutas Guardadas: ");
    Serial.print(rutasguardadas);
    Serial.println("#");

  } else {
    Serial.println("Solo se pueden almacenar 2 rutas.#");
  }

  //recorrer ruta:
  Serial.println("Ruta Guardada.#");
  /*for (int k = 0; k < rutasguardadas; k++) {
    RecorrerRuta(k);
    }*/
  estadomanual = false;
  estadonombre = false;
  c_nombre = 0;
  memset(&tmp, 0, sizeof(tmp)); //limpiamos el temporal.
  memset(&tmp_nombre, '\0', sizeof(tmp_nombre));

}

void EstadoRecorrer() {
  bool ruta1 = false;
  bool ruta2 = false;

  char estado[2];
  memset(&estado, '\0', sizeof(estado));
  int indice = 0;
  int contador = 0;
  while (true) {
    if (Serial.available())
    {
      estado[indice] = Serial.read();
      //Serial.print(estado[indice]);
      //Serial.println("#");
      indice++;
      if (indice == 2) {
        break;
      }
    }
    contador++;
  }
  Serial.print(estado[0]);
  Serial.print(estado[1]);
  Serial.println("#");

  if (estado[0] == '1'  && estado[1] == 'x' ) {
    RecorrerRuta(0);
  }
  else if (estado[0] == '0'  && estado[1] == 'x') {
    RecorrerRutaInvertida(0);
  }
  else if (estado[0] == '1'  && estado[1] == '1') {
    RecorrerRuta(1);
  }
  else if (estado[0] == '0'  && estado[1] == '0') {
    RecorrerRutaInvertida(1);
  } else {
    Serial.println("Else chavo#");
  }
}

void RecorrerRuta(int no) {

  ObjRuta customVar; //Variable to store custom object read from EEPROM.
  EEPROM.get(0 + sizeof(int) + (sizeof(ObjRuta)*no) , customVar);

  Serial.print("Nombre Ruta ");
  Serial.print(no + 1);
  Serial.print(" Recuperada: ");
  Serial.print(customVar.name_[0]);
  Serial.print(customVar.name_[1]);
  Serial.print(customVar.name_[2]);
  Serial.print(customVar.name_[3]);
  Serial.println("#");

  for (int i = 0; i < 500; i++) {
    if (customVar.movimientos[i] != NULL) {

      Serial.print("Mov ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(customVar.movimientos[i]);
      Serial.println("#");
      //mandamos a ejecutar movimiento
      EjecutarMovimiento(customVar.movimientos[i]);
      delay(500);
    }
  }
}
void RecorrerRutaInvertida(int no) {

  ObjRuta customVar; //Variable to store custom object read from EEPROM.
  EEPROM.get(0 + sizeof(int) + (sizeof(ObjRuta)*no) , customVar);

  Serial.print("Nombre Ruta ");
  Serial.print(no + 1);
  Serial.print(" Recuperada: ");
  Serial.print(customVar.name_[0]);
  Serial.print(customVar.name_[1]);
  Serial.print(customVar.name_[2]);
  Serial.print(customVar.name_[3]);
  Serial.println("#");

  for (int i = 499; i >= 0; i--) {
    if (customVar.movimientos[i] != NULL) {

      Serial.print("Mov ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(customVar.movimientos[i]);
      Serial.println("#");
      //mandamos a ejecutar movimiento
      //EjecutarMovimiento(customVar.movimientos[i]);
      EjecutarMovimientoInvertido(customVar.movimientos[i]);
      delay(500);
    }
  }
}
void EjecutarMovimientoInvertido(char VarChar) {
  if (VarChar == '2')
  {
    Atras();
    delay(100);
    Apagar();
    delay(50);
  }
  else if (VarChar == '3')
  {
    Adelante();
    delay(100);
    Apagar();
    delay(50);
  }
  else if (VarChar == '4')
  {
    Izquierda();
    delay(100);
    Apagar();
    delay(50);
  }
  else if (VarChar == '5')
  {
    Derecha();
    delay(100);
    Apagar();
    delay(50);
  }
}

void EjecutarMovimiento(char VarChar) {
  if (VarChar == '2')
  {
    Adelante();
    delay(100);
    Apagar();
    delay(50);
  }
  else if (VarChar == '3')
  {
    Atras();
    delay(100);
    Apagar();
    delay(50);
  }
  else if (VarChar == '4')
  {
    Derecha();
    delay(100);
    Apagar();
    delay(50);
  }
  else if (VarChar == '5')
  {
    Izquierda();
    delay(100);
    Apagar();
    delay(50);
  }
}

void LimpiarRutas() {
  //borrar toda la memoria
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void Apagar() {

  digitalWrite(rueda_Izquierda_T, LOW);
  digitalWrite(rueda_Izquierda_C, LOW);
  digitalWrite(rueda_Derecha_T, LOW);
  digitalWrite(rueda_Derecha_C, LOW);
}

void Adelante() {
  digitalWrite(rueda_Izquierda_T, LOW);
  digitalWrite(rueda_Izquierda_C, HIGH);
  digitalWrite(rueda_Derecha_C, HIGH);
  digitalWrite(rueda_Derecha_T, LOW);
}

void Atras() {
  digitalWrite(rueda_Izquierda_T, HIGH);
  digitalWrite(rueda_Izquierda_C, LOW);
  digitalWrite(rueda_Derecha_C, LOW);
  digitalWrite(rueda_Derecha_T, HIGH);
}

void Izquierda() {
  digitalWrite(rueda_Izquierda_T, HIGH);
  digitalWrite(rueda_Izquierda_C, LOW);
  digitalWrite(rueda_Derecha_C, HIGH);
  digitalWrite(rueda_Derecha_T, LOW);
}

void Derecha() {
  digitalWrite(rueda_Izquierda_T, LOW);
  digitalWrite(rueda_Izquierda_C, HIGH);
  digitalWrite(rueda_Derecha_C, LOW);
  digitalWrite(rueda_Derecha_T, HIGH);
}


//modo automatico
void Automatico() {
  do {
    char VarChar = Serial.read();
    if (VarChar == '0')
    {
      Play = false;
      break;
    } else {
      //Serial.println("Error comando no permitido.");
    }

    if (!detener()) {
      Adelante();
      delay(50);
      Apagar();
      delay(150);
    } else {
      Apagar();
      delay(1000);
      //Serial.println(detener());
      int azul, verde, rojo;
      azul = 0;
      verde = 0;
      rojo = 0;
      int contador = 0;
      while (contador < 50)
      {
        int color_tmp = color();
        if (color_tmp == 0) {
          rojo++;
        } else if (color_tmp == 1)
        {
          azul++;
        } else if (color_tmp == 2)
        {
          verde++;
        }
        contador++;
      }
      Serial.print("Rojo: ");
      Serial.print(rojo);
      Serial.println("#");
      Serial.print("Verde: ");
      Serial.print(verde);
      Serial.println("#");
      Serial.print("Azul: ");
      Serial.print(azul);
      Serial.println("#");

      int col = 4;
      if (rojo > verde && rojo > azul)
      {
        col = 0;
      } else if (verde > rojo && verde > azul)
      {
        col = 2;
      } else if (azul > rojo && azul > verde)
      {
        col = 1;
      }

      if (col == 1) {
        for (int i = 0; i < 16; i++) {
          Derecha();
          delay(50);
          Apagar();
          delay(150);
        }
      } else if (col == 2) {
        for (int i = 0; i < 16; i++) {
          Izquierda();
          delay(50);
          Apagar();
          delay(150);
        }
      } else if (col == 0) {
        Apagar(); //apagar
        break;
      } else {
        //no hace nada
        //Serial.print(col);
      }
    }

  } while (Play);
}


//sesonr ultrasonico
boolean detener()
{
  int cm = ping(TriggerPin, EchoPin);
  //Serial.println(cm);
  if (cm <= 4)
  {
    return true;
  }
  else
  {
    return false;
  }
}

int ping(int TriggerPin, int EchoPin) {

  long duration, distanceCm;
  digitalWrite(TriggerPin, LOW);
  delayMicroseconds(4);
  digitalWrite(TriggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TriggerPin, LOW);
  duration = pulseIn(EchoPin, HIGH);  //medimos el tiempo entre pulsos, en microsegundos
  distanceCm = duration * 10 / 292 / 2;  //convertimos a distancia, en cm
  return distanceCm;
}


//sensor de color
int color()
{
  digitalWrite(s2, LOW);
  digitalWrite(s3, LOW);
  //count OUT, pRed, RED
  red = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s3, HIGH);
  //count OUT, pBLUE, BLUE
  blue = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);
  digitalWrite(s2, HIGH);
  //count OUT, pGreen, GREEN
  green = pulseIn(out, digitalRead(out) == HIGH ? LOW : HIGH);

  if (red < blue && red < green && red < 20)
  {
    return 0; //ROJO
  }
  else if (blue < red && blue < green)
  {
    return 1; //AZUL
  }
  else if (green < red && green < blue)
  {
    return 2; //VERDE
  } else {
    return 3; //default
  }
}
