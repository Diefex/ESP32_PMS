/* 
 *  ESP32 con Sensores PMS y conexion a servidor remoto
 *  Creado por: Diego Velez (dalejandrovelezg@gmail.com)
 *  jul-2021
*/

// Librerias necesarias ----------------
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h> 
#include <PMserial.h>
#include "ThingSpeak.h"

WiFiMulti wifiMulti;


// Credenciales para WiFi --------------
// Introduzca las credenciales de la red WiFi a continuacion
char ssid[20] = "suNombreDeRed";   // Nombre de la red wifi
char pass[20] = "suContraseñaDeRed";   // Contraseña de la red wifi


// Pines complementarios ---------------
const int led = 2; // Pin del led

// Credenciales para ThingSpeak
unsigned long canalID = 1742751;
const char * wApiKey = "AXB5B209GAQ6Y21D";

// Declaracion de sensores PMS ---------
// Para agregar sensores PMS declarelos a continuacion:
//                                          Pins
//                                PMSx003, RX, TX
SerialPM sensores[] = {//SerialPM(PMSx003, 13, 12), 
                       //SerialPM(PMSx003, 14, 27),
                       SerialPM(PMSx003, 26, 25),
                       //SerialPM(PMSx003, 33, 32),
                       SerialPM(PMSx003, 35, 34),
                       //SerialPM(PMSx003, 15, 2),
                       //SerialPM(PMSx003, 5, 18),
                       //SerialPM(PMSx003, 19, 21),
                       //SerialPM(PMSx003, 22, 23)
                       };


// Funciones ----------------------------
// Parpadear el led x veces
void inLed(int x, bool err){
  if(err){ // para indicar un error
    digitalWrite(led, HIGH);
    delay(1000);
    digitalWrite(led, LOW);
    delay(100);
  }
  for (int i=0; i<x; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }
  delay(400);
}

// Enviar datos de un sensor
void enviar(int n, int pm10, int pm25) {
  
  ThingSpeak.setField(n, pm10);
  ThingSpeak.setField(n+1, pm25);

  int x = ThingSpeak.writeFields(canalID, wApiKey);
  if(x == 200){
    Serial.println("Datos subidos correctamente.");
  }
  else{
    Serial.println("Error al subir datos. HTTP error code " + String(x));
  }
}

// Leer un sensor
void leerSensor(int i){
  sensores[i].init();
  sensores[i].read();
  Serial.print(F("SENSOR "));Serial.println(i+1);
  if (sensores[i]){ // Lectura correcta
    
    // impresion de resultados
    Serial.println(F("Concentracion de Particulas:"));
    Serial.print(F("PM1.0= "));
    Serial.print(sensores[i].pm01);
    Serial.print(F(", "));
    Serial.print(F("PM2.5= "));
    Serial.print(sensores[i].pm25);
    Serial.print(F(", "));
    Serial.print(F("PM10= "));
    Serial.print(sensores[i].pm10);
    Serial.println(F(" [ug/m3]"));
    inLed(1, false);// Indicacion por led

    // Envio a ThingSpeak
    enviar((i*2)+1, sensores[i].pm10, sensores[i].pm25);
    
  }else{ // Lectura incorrecta
    Serial.println("Fallo al leer el sensor: ");
    switch (sensores[i].status){ // Impresion de Errores
      case sensores[i].OK:
        break;
      case sensores[i].ERROR_TIMEOUT:
        Serial.println(F(PMS_ERROR_TIMEOUT));
        break;
      case sensores[i].ERROR_MSG_UNKNOWN:
        Serial.println(F(PMS_ERROR_MSG_UNKNOWN));
        break;
      case sensores[i].ERROR_MSG_HEADER:
        Serial.println(F(PMS_ERROR_MSG_HEADER));
        break;
      case sensores[i].ERROR_MSG_BODY:
        Serial.println(F(PMS_ERROR_MSG_BODY));
        break;
      case sensores[i].ERROR_MSG_START:
        Serial.println(F(PMS_ERROR_MSG_START));
        break;
      case sensores[i].ERROR_MSG_LENGTH:
        Serial.println(F(PMS_ERROR_MSG_LENGTH));
        break;
      case sensores[i].ERROR_MSG_CKSUM:
        Serial.println(F(PMS_ERROR_MSG_CKSUM));
        break;
      case sensores[i].ERROR_PMS_TYPE:
        Serial.println(F(PMS_ERROR_PMS_TYPE));
        break;
    }
    inLed(1, true);// Indicacion por led
  }
}


// Setup y Loop --------------------------
void setup() {

  pinMode(led, OUTPUT);

  inLed(5, false); // Indicacion de Inicio

  // Iniciar conexion WiFi
  wifiMulti.addAP(ssid, pass);
  
  Serial.begin(115200);  
  
}

void loop() {
  
  // Conectar o reconectar a WiFi
  if((wifiMulti.run() != WL_CONNECTED)){
    Serial.print("Intentando conectarse a la Red: ");
    Serial.println(ssid);
    while((wifiMulti.run() != WL_CONNECTED)){
      Serial.print(".");
      // Indicacion por led
      digitalWrite(led, HIGH);
      delay(500);
      digitalWrite(led, LOW);
      delay(500);
    }
    Serial.println("\nConectado a WiFi.");
  }

  // Inicio de ciclo de lectura de sensores y envio de datos
  inLed(4, false);// Indicacion por led
  Serial.println(F("---------------------------------"));
  int n = sizeof sensores/sizeof sensores[0];
  for(int i=0; i<n; i++){
    leerSensor(i);
    Serial.println(F("---------------------------------"));
    delay(1000); // Espera de 1 seg. entre sensores
  }
    
  delay(20000); // Espera de 20 seg. entre lecturas
}
