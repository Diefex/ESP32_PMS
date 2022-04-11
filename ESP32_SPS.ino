/* 
 *  ESP32 con Sensores SPS30 y conexion a servidor remoto
 *  Creado por: Diego Velez (dalejandrovelezg@gmail.com)
 *  abr-2022
*/

// Librerias necesarias ----------------
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include<Wire.h>
#include "sps30.h"

// Canales de comunicacion -------------
#define SPS30_COMMS1 Serial1   // UART
//#define SPS30_COMMS2 Serial2   // UART
//#define SPS30_COMMS3 Serial3   // UART
#define SPS30_COMMS4 Wire      // I2C 
//#define SPS30_COMMS5 Wire1     // I2C 

// Pines para serial1 ------------------
int TX_PIN = 32;
int RX_PIN = 33;

// Constructores -----------------------
SPS30 sps30_1;
SPS30 sps30_2;
SPS30 sps30_3;
SPS30 sps30_4;
SPS30 sps30_5;
WiFiMulti wifiMulti;

// Verificacion de inicio de sensores
bool sen1 = false;
bool sen2 = false;
bool sen3 = false;
bool sen4 = false;
bool sen5 = false;

// Credenciales para WiFi --------------
// Introduzca las credenciales de la red WiFi a continuacion
char ssid[20] = "DiefexMobile";   // Nombre de la red wifi
char pass[20] = "ewfa1249";   // Contraseña de la red wifi


// Pines complementarios ---------------
const int led = 2; // Pin del led

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
void enviar(String datos) {
  HTTPClient http;
  Serial.print("\nEnviando datos al servidor...\n");
  http.begin("http://ritaportal.udistrital.edu.co:10394/subirDatos.php");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpCode = http.POST(datos);

  if(httpCode > 0) { // la cabecera HTTP fue enviada y el servidor respondio 
      if(httpCode == HTTP_CODE_OK) { // La respuesta del servidor fue correcta
          // Impresion de la respuesta del servidor
          String payload = http.getString();
          Serial.println("Envio correcto. Respuesta del Servidor:");
          Serial.println(payload);
          inLed(2, false);// Indicacion por led
      } else {// Respuesta incorrecta del servidor
        Serial.println("Envio incorrecto. Respuesta del Servidor incorrecta");
        inLed(3, true);// Indicacion por led
      }
  } else { // Se produjo un error al enviar
      Serial.printf("ERROR HTTP: %s\n", http.errorToString(httpCode).c_str());
      inLed(2,true);// Indicacion por led
  }
  http.end();
}

// Leer un sensor
bool read_all(uint8_t d)
{

  static bool header = true;
  uint8_t ret, error_cnt = 0;
  struct sps_values val;

  // loop to get data
  do {
  if (d == 1) ret = sps30_1.GetValues(&val);
  else if (d == 2) ret = sps30_2.GetValues(&val);
  else if (d == 3) ret = sps30_3.GetValues(&val);
  else if (d == 4) ret = sps30_4.GetValues(&val);
  else if (d == 5) ret = sps30_5.GetValues(&val);
  else {
    Serial.println("Invalid device to read");
    return false;
  }

    // data might not have been ready
    if (ret == ERR_DATALENGTH){

        if (error_cnt++ > 3) {
          Serial.println("los datos no estan listos");
        }
        delay(1000);
    }

    // if other error
    else if(ret != ERR_OK) {
      Serial.println("error en la lectura de datos");
    }

  } while (ret != ERR_OK);
  
  Serial.print(d);
  Serial.print(F("    "));
  
  Serial.print(val.MassPM1);
  Serial.print(F("\t"));
  Serial.print(val.MassPM2);
  Serial.print(F("\t"));
  Serial.print(val.MassPM4);
  Serial.print(F("\t"));
  Serial.print(val.MassPM10);
  Serial.print(F("\t"));
  Serial.print(val.NumPM0);
  Serial.print(F("\t"));
  Serial.print(val.NumPM1);
  Serial.print(F("\t"));
  Serial.print(val.NumPM2);
  Serial.print(F("\t"));
  Serial.print(val.NumPM4);
  Serial.print(F("\t"));
  Serial.print(val.NumPM10);
  Serial.print(F("\t"));
  Serial.print(val.PartSize);
  Serial.print(F("\n"));
  
  // Envio de datos al servidor
  String datos = "";
  datos += "PM25=";
  datos += val.MassPM2;
  datos += "&PM10=";
  datos += val.MassPM10;
  datos += "&Sensor=";
  datos += "1";
  
  enviar(datos);
    
  return(true);
}

// iniciar un sensor
void setupSPS30(uint8_t d)
{
  bool ret;

#ifdef SPS30_COMMS1
  if (d == 1)
  {
    // start channel
    SPS30_COMMS1.begin(115200, SERIAL_8N1, TX_PIN, RX_PIN);    // ESP32 in case of Serial1
    
    // Initialize SPS30 library
    if (!sps30_1.begin(&SPS30_COMMS1)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 1"));
    } else
    // check for SPS30 connection
    if (! sps30_1.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 1"));
      } else {
      Serial.println(F("\nDetected SPS30 - 1"));
    
      // reset SPS30 connection
      if (! sps30_1.reset()){
        Serial.println(F("could not reset."));
      } else
      if (! sps30_1.start()) {
        Serial.println(F("\nCould NOT start measurement SPS30"));
      } else {
        Serial.println(F("Measurement started SPS30 - 1"));
        sen1 = true;
      }
    }
  }
#endif

#ifdef SPS30_COMMS2
  if (d == 2)
  {
    // start channel
    SPS30_COMMS2.begin(115200);

    // Initialize SPS30 library
    if (!sps30_2.begin(&SPS30_COMMS2)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 2"));
    } else
    // check for SPS30 connection
    if (! sps30_2.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 2"));
    } else {
      Serial.println(F("\nDetected SPS30 - 2"));
    
      // reset SPS30 connection
      if (! sps30_2.reset()){
        Serial.println(F("could not reset."));
      } else 
      if (! sps30_2.start()) {
        Serial.println(F("\nCould NOT start measurement SPS30"));
      } else {
    
        Serial.println(F("Measurement started SPS30 - 2"));
        sen2 = true;
      }
    }
  }
#endif

#ifdef SPS30_COMMS3
  if (d == 3)
  {
    // start channel
    SPS30_COMMS3.begin(115200);

    // Initialize SPS30 library
    if (!sps30_3.begin(&SPS30_COMMS3)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 3"));
    } else
    // check for SPS30 connection
    if (! sps30_3.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 3"));
    } else {
      Serial.println(F("\nDetected SPS30 - 3"));
    
      // reset SPS30 connection
      if (! sps30_3.reset()){
        Serial.println(F("could not reset."));
      } else 
      if (! sps30_3.start()) {
        Serial.println(F("\nCould NOT start measurement SPS30"));
      } else {
        Serial.println(F("Measurement started SPS30 - 3"));
        sen3 = true;
      }
    }
  }
#endif

#ifdef SPS30_COMMS4 
  if (d == 4)
  {
    // start channel
    SPS30_COMMS4.begin();

    // Initialize SPS30 library
    if (!sps30_4.begin(&SPS30_COMMS4)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 4"));
    } else
    // check for SPS30 connection
    if (! sps30_4.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 4"));
    } else {
      Serial.println(F("\nDetected SPS30 - 4"));
    
      // reset SPS30 connection
      if (! sps30_4.reset()){
        Serial.println(F("could not reset."));
      } else 
      if (! sps30_4.start()) {
        Serial.println(F("\nCould NOT start measurement SPS30"));
      } else {
        Serial.println(F("Measurement started SPS30 - 4"));
        sen4 = true;
        if (sps30_4.I2C_expect() == 4)
            Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
      }
    }
  }
#endif

#ifdef SPS30_COMMS5
  if (d == 5)
  {
    // start channel
    SPS30_COMMS5.begin();
    //SPS30_COMMS5.begin(23,18,100000); // SDA pin 23, SCL pin 18, 100kHz frequency // Wire1 on ESP32
    
    // Initialize SPS30 library
    if (!sps30_5.begin(&SPS30_COMMS5)) {
      Serial.println(F("\nCould not set communication channel for SPS30 - 5"));
    } else
    // check for SPS30 connection
    if (! sps30_5.probe()) {
      Serial.println(F("\nCould not probe / connect with SPS30 - 5"));
    } else {
      Serial.println(F("\nDetected SPS30 - 5"));
    
      // reset SPS30 connection
      if (! sps30_5.reset()){
        Serial.println(F("could not reset."));
      } else 
      if (! sps30_5.start()) {
        Serial.println(F("\nCould NOT start measurement SPS30"));
      } else {
        Serial.println(F("Measurement started SPS30 - 5"));
        sen5 = true;
        if (sps30_5.I2C_expect() == 4)
            Serial.println(F(" !!! Due to I2C buffersize only the SPS30 MASS concentration is available !!! \n"));
      }
    }
  }
#endif
}

// Setup y Loop --------------------------
void setup() {

  // iniciar canales I2C
  Wire.begin(25, 26);
  //Wire1.begin(32, 33);

  pinMode(led, OUTPUT);
  inLed(5, false); // Indicacion de Inicio

  // Iniciar conexion WiFi
  wifiMulti.addAP(ssid, pass);
  
  Serial.begin(115200);
  
  // setup SPS30 - 1 UART 
  #ifdef SPS30_COMMS1
    setupSPS30(1);
  #endif
  // setup SPS30 - 2 UART 
  #ifdef SPS30_COMMS2
    setupSPS30(2);
  #endif
  // setup SPS30 - 3 UART 
  #ifdef SPS30_COMMS3
    setupSPS30(3);
  #endif
  // setup SPS30 - 4  I2C 
  #ifdef SPS30_COMMS4
    setupSPS30(4);
  #endif
  // setup SPS30 - 5  I2C
  #ifdef SPS30_COMMS5
    setupSPS30(5);
  #endif
  
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

  // lectura de sensores y envio de datos
  inLed(4, false);// Indicacion por led
  Serial.println(F("---------------------------------"));

  struct sps_values val;
  uint8_t ret;
    
  #ifdef SPS30_COMMS1
    if(sen1) read_all(1);
  #endif
  
  #ifdef SPS30_COMMS2
    if(sen2) read_all(2);
  #endif
  
  #ifdef SPS30_COMMS3
    if(sen3) read_all(3);
  #endif
  
  #ifdef SPS30_COMMS4
    if(sen4) read_all(4);
  #endif
  
  #ifdef SPS30_COMMS5
    if(sen5) read_all(5);
  #endif
    
  Serial.println(F("---------------------------------"));
  
  delay(5000); // Espera de 20 seg. entre lecturas
}
