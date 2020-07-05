

#include <dht_nonblocking.h>
#include <Servo.h>

/*
 * Defines
 */
#define DHT_SENSOR_TYPE DHT_TYPE_11

/*
 * Sensor PIR
 */
static const int ledPin = 52;   // Elegimos el pin para el led
static const int inputPin = 53; // Elegimos el pin de entrada para el sensor PIR
String pirState = "false";      // Empezamos asumiendo que el PIR no ha detectado a nadie
int val = 0;                    // Variable del estado de lectura del sensor

/*
 * Sensor DHT
 */
static const int DHT_SENSOR_PIN = 3;
DHT_nonblocking dht_sensor(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

//Valor de las mediciones de temperatura y humedad, se inicializan en valores normales para evitar problemas en los primeros ciclos
float temperatura = 20;
float humedad = 50;

/*
 * Fotoresistor
 */
static const int pResistor = A0; // Se define al pin analogico 0

int valorFotoresistor; // Almacena los valores del fotoresistor (0-1023)

String hayLuz;

/*
 * Servo motor
 */
/* Servo myservo; // Creamos el objeto servo para controlar el servo
int pos = 0;   // Variable que almacena la posicion del servo
static const int SERVO_MOTOR_PIN = 4; */

/*
 * Relés
 */
// Definimos los pines de los relés
/* static const int rele_termostato = 10;
static const int rele_luz1 = 11;
static const int rele_luz2 = 12;
static const int rele_persiana1 = 13;
static const int rele_persiana2 = 14; */

/*
* Control del json
*/

String jsonAntiguo = "";

/*
* Lectura puerto serie
*/
int option;

void setup()
{
  // Definimos el puerto serie
  Serial.begin(9600);

  /*
 * PIR sensor
 */
  pinMode(ledPin, OUTPUT);  // Declaramos el pin del led como salida
  pinMode(inputPin, INPUT); // Declaramos el pin del sensor como entrada
 pinMode(8, OUTPUT);
  /*
 * DHT sensor
 */

  /*
 * Servo motor
 */
  //myservo.attach(SERVO_MOTOR_PIN); // Une el servo en el pin al objeto servo

  /*
 * Relés
 */
  /* pinMode(rele_termostato, OUTPUT);
  pinMode(rele_luz1, OUTPUT);
  pinMode(rele_luz2, OUTPUT);
  pinMode(rele_persiana1, OUTPUT);
  pinMode(rele_persiana2, OUTPUT); */

  /*
 * Fotoresistor
 */
  int calibracionFotoresistor;

  calibracionFotoresistor = analogRead(pResistor);

  if (calibracionFotoresistor > 50)
  {
    hayLuz = "true";
  }
  else if (calibracionFotoresistor <= 50)
  {
    hayLuz = "false";
  }
}

/*
 * Definicion de funciones
 */
static bool measure_environment(float *temperature, float *humidity)
{
  static unsigned long measurement_timestamp = millis();

  /* Mide cada segundo. */
  if (millis() - measurement_timestamp > 1000ul)
  {
    if (dht_sensor.measure(temperature, humidity) == true)
    {
      measurement_timestamp = millis();
      return (true);
    }
  }

  return (false);
}

void loop()
{

  // Primero leemos los datos de los sensores y los escribimos en el puerto serie

  /*
 * PIR sensor
 */

  val = digitalRead(inputPin); // Lee el input
  if (val == HIGH)
  {
    // Si el input esta en HIGH significa que hemos detectado movimiento

    digitalWrite(ledPin, HIGH); // Encendemos el led
                                // Se considera que ha habido movimiento
    pirState = "true";
  }
  else
  {
    // Si el input esta en LOW significa que NO hemos detectado movimiento
    digitalWrite(ledPin, LOW); // Apagamos el led
    // Se considera que no ha habido movimiento
    pirState = "false";
  }

  /*
 * Sensor DHT
 */

  // Devuelve verdadero cuando la medicion esta disponible
  measure_environment(&temperatura, &humedad);

  /*
 * Fotoresistor
 */

  valorFotoresistor = analogRead(pResistor); //Con el valor lo tratamos despues en el servicio que recibe los datos.

  if (valorFotoresistor > 50)
    hayLuz = "true";

  else if (valorFotoresistor < 40)
    hayLuz = "false";

  //Enviamos el json por el puerto serie
  String json = "{\"temperatura\":" + String(temperatura, 1) + ", \"humedad\":" + String(humedad, 1) + ", \"luz\":" + hayLuz + ", \"movimiento\":" + pirState + "}";

  if (json != jsonAntiguo){
    Serial.println(json);
  }

  jsonAntiguo = json;

  //Posteriormente leemos los datos que hay en el puerto serie para leer las ordenes que lleguen

  if (Serial.available() > 0)
  {
    //leemos la opcion enviada
    option = Serial.read();

    
    if (option == 'v')
    {
      //Conectar servomotor
      digitalWrite(8, HIGH);


      Serial.println("Abrir ventana");
    }else if ((option-'0') >= 1 && (option-'0') <= 9)
    {
      int puertoRele = option-'0';

      digitalWrite(8, LOW);
      //Conectar Reles

      Serial.print("Activar/Descativar Rele ");
      Serial.println(puertoRele);
    }
  }
}
