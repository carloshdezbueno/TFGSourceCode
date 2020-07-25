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
static float temperatura;
static float humedad;

float temperaturaTemp = 22;
float humedadTemp = 22;

/*
 * Fotoresistor
 */
static const int pResistor = A0; // Se define al pin analogico 0

int valorFotoresistor; // Almacena los valores del fotoresistor (0-1023)

String hayLuz;

/*
 * Servo motor
 */

Servo myservo; // Creamos el objeto servo para controlar el servo
int pos = 0;   // Variable que almacena la posicion del servo
static const int SERVO_MOTOR_PIN = 13;

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
    myservo.attach(SERVO_MOTOR_PIN); // Une el servo en el pin al objeto servo
    myservo.write(0);
    delay(500);
    pos = myservo.read();

    
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
  * Fotoresistor
  */

    valorFotoresistor = analogRead(pResistor); //Con el valor lo tratamos despues en el servicio que recibe los datos.

    if (valorFotoresistor > 50)
        hayLuz = "true";

    else if (valorFotoresistor < 40)
        hayLuz = "false";

    //Posteriormente leemos los datos que hay en el puerto serie para leer las ordenes que lleguen

    if (Serial.available() > 0)
    {

        //leemos la opcion enviada

        //option = Serial.read();

        String data;
        data = Serial.readString();

        Serial.println(data);
        Serial.flush();

        if (data.equals("enviame"))
        {
            unsigned long tiempoinicial, tiempoIntermedio;
            tiempoinicial = tiempoIntermedio = millis();

            while (tiempoIntermedio - tiempoinicial < 10000ul)
            {
                /*
          * Sensor DHT
          */

                bool medision = measure_environment(&temperatura, &humedad);

                if (medision)
                {
                    temperaturaTemp = temperatura;
                    humedadTemp = humedad;
                    break;
                }
                tiempoIntermedio = millis();
            }

            String json = "{\"temperatura\":" + String(temperatura) + ", \"humedad\":" + String(humedad) + ", \"luz\":" + hayLuz + ", \"movimiento\":" + pirState + "}";
            Serial.println(json);
            Serial.flush();
        }
        else if (data.equals("abrirVentana"))
        {
            //Abre la ventana

            if(pos != 180){
                for (pos = myservo.read(); pos <= 180; pos += 1)
                { // goes from 0 degrees to 180 degrees
                    // in steps of 1 degree
                    myservo.write(pos); // tell servo to go to position in variable 'pos'
                    delay(15);          // waits 15ms for the servo to reach the position
                }

            }

            Serial.println("OK");
        }
        else if (data.equals("cerrarVentana"))
        {
            //Cierra la ventana

            if(pos != 0){

                for (pos = myservo.read(); pos >= 0; pos -= 1)
                {                       // goes from 180 degrees to 0 degrees
                    myservo.write(pos); // tell servo to go to position in variable 'pos'
                    delay(15);          // waits 15ms for the servo to reach the position
                }
            }

            Serial.println("OK");
        }
        else if (data.toInt() != 0)
        {

            long pin = data.toInt();

            if (pin <= 10)
                digitalWrite(8, HIGH);
            if (pin > 10)
                digitalWrite(8, LOW);
            //Conectar Reles

            Serial.println("OK");
        }
        else
        { //Ninguna orden coincide
            Serial.println("No");
        }
    }
}
