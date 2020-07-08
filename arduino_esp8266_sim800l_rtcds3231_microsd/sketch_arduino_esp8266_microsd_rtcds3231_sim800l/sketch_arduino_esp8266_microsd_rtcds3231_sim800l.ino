#include <SoftwareSerial.h>

// ======= Librerías MICROSD =========
#include <SPI.h>    // incluye libreria interfaz SPI
#include <SD.h>     // incluye libreria para tarjetas SD

// ======= Librerías RTC DS3231 =========
#include <Wire.h>   // incluye libreria para interfaz I2C
#include <RTClib.h>   // incluye libreria para el manejo del modulo RTC

#define SSpin 10    // Slave Select en pin digital 10

//SIM800L TX is connected to Arduino D7
#define SIM800L_TX_PIN 5
 
//SIM800L RX is connected to Arduino D6
#define SIM800L_RX_PIN 4

File archivo;     // objeto archivo del tipo File
RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231
SoftwareSerial esp8266(3, 2); // [TX del ESP8266] conectado al pin 3 del Arduino [RX del ESP8266] conectado al pin 2 del Arduino. 
SoftwareSerial serialSIM800L(SIM800L_TX_PIN,SIM800L_RX_PIN); //Create software serial object to communicate with SIM808

// Variables
String apiario;
String colmena;
String temperatura;
String humedad;
String fecha;
String hora;
char bigstringfecha[13];  // con sufiente espacio para los dos strings a sumar
char bigstringhora[13];  // con sufiente espacio para los dos strings a sumar

  
void setup() {
  
  if (!SD.begin(SSpin)) {     // inicializacion de tarjeta SD
    Serial.println("fallo en inicializacion del Modulo Micro SD!");// si falla se muestra texto correspondiente y
    return;         // se sale del setup() para finalizar el programa
  }

  Serial.println("inicializacion correcta del Modulo MicroSD");  // texto de inicializacion correcta
  
  
  // Open serial communications and wait for port to open 
  Serial.begin(9600);
  while(!Serial){
    // wait for serial port to connect.
  }


  if (! rtc.begin()) {       // si falla la inicializacion del modulo
    Serial.println("Modulo RTC no encontrado !");  // muestra mensaje de error
    while (1);         // bucle infinito que detiene ejecucion del programa
  }

  // funcion que permite establecer fecha y horario
  // al momento de la compilacion.
  rtc.adjust(DateTime(__DATE__, __TIME__));  


  esp8266.begin(9600);
  serialSIM800L.begin(9600); // Inicio de comunicación Serial1

 
}



void guardarDatos(String apiario, String colmena, String temperatura, String humedad, String fecha, String hora) {
  archivo = SD.open("datos.txt", FILE_WRITE); // apertura para lectura/escritura de archivo datos.txt

  if (archivo) { 
    archivo.print(apiario);       // escribe en tarjeta el numero de apiario
    archivo.print(",");     // escribe en tarjeta una coma
    archivo.print(colmena);       // escribe en tarjeta el numero de colmena
    archivo.print(",");     // escribe en tarjeta una coma
    archivo.print(temperatura);   // escribe en tarjeta el valor de temperatura
    archivo.print(",");     // escribe en tarjeta una coma
    archivo.print(humedad);
    archivo.print(",");
    archivo.print(fecha);
    archivo.print(",");
    archivo.println(hora);
  }

  // Cierro el archivo
  archivo.close();
}


void respuesta(){
  while( serialSIM800L.available() ) // Mientras la comunicación está
  {
    if( serialSIM800L.available() > 0 ) // Si se reciben datos (mayor)
    {
      Serial.write(serialSIM800L.read()); // imprime lo que recibe en 
    }
  }
  Serial.flush();
  serialSIM800L.flush();
}


void enviarDatos(String apiario, String colmena, String temperatura, String humedad, String fecha, String hora) {
    /* Comprobamos comunicación con comandos AT */
    serialSIM800L.println("AT"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    
    /* Inicializamos comunicación */
    serialSIM800L.println("AT+SAPBR=3,1,\"APN\",\"CMNET\""); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    serialSIM800L.println("AT+SAPBR=1,1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/

    /* Nos comunicamos con el Servidor */
    serialSIM800L.println("AT+HTTPINIT"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    serialSIM800L.println("AT+HTTPPARA=\"CID\",1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    serialSIM800L.println("AT+HTTPPARA=\"URL\",\"http://colmenainteligente.ddns.net/abeja2/?apiario_id=" + apiario + "&colmena_id=" + colmena + "&temperatura=" + temperatura +"&humedad=" + humedad + "&fecha=" + fecha + "&hora=" + hora + "\""); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    serialSIM800L.println("AT+HTTPACTION=0"); // Envío En comando AT
    delay(3000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    serialSIM800L.println("AT+HTTPREAD"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/ 
    serialSIM800L.println("AT+HTTPTERM"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    

    /* Cerramos la comunicación */
    serialSIM800L.println("AT+SAPBR=0,1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    
}

void loop(){
  
  
  if( esp8266.available() ){
      String informacion = esp8266.readString();
      if(informacion.indexOf("Apiario") > 0)
      {
        Serial.println("Apiario encontrado");
        Serial.println(informacion.indexOf("Apiario"));
        // Apiario= 1
        int start = informacion.indexOf("Apiario");
        int end = informacion.indexOf("\n",start);
        apiario = informacion.substring(start,end);
        Serial.println(informacion.substring(start,end));
      }
      if(informacion.indexOf("Colmena") > 0)
      {
        Serial.println("Colmena encontrada");
        Serial.println(informacion.indexOf("Colmena"));
        // Colmena= 13
        int start = informacion.indexOf("Colmena");
        int end = informacion.indexOf("\n",start);
        colmena = informacion.substring(start,end);
        Serial.println(informacion.substring(start,end));
      }
      if(informacion.indexOf("Temperatura") > 0)
      {
        Serial.println("Temperatura encontrada");
        Serial.println(informacion.indexOf("Temperatura"));
        // Temperatura= 25.00
        int start = informacion.indexOf("Temperatura");
        //int end = start + 18; 
        int end = informacion.indexOf("\n",start);
        temperatura = informacion.substring(start,end);
        Serial.println(informacion.substring(start,end));
      }
      if(informacion.indexOf("Humedad") > 0)
      {
        Serial.println("Humedad encontrada");
        Serial.println(informacion.indexOf("Humedad"));
        // Humedad= 31.00
        int start = informacion.indexOf("Humedad");
        int end = informacion.indexOf("\n",start);
        humedad = informacion.substring(start,end);
        Serial.println(informacion.substring(start,end));
      }

      // Obtengo fecha y hora actual
      DateTime fecha_hora = rtc.now();
      // fecha = fecha_hora.year() + "/" + fecha_hora.month() + "/" + fecha_hora.day();
      // hora = fecha_hora.hour() + ":" + fecha_hora.minute();
      bigstringfecha[0] = 0;
      strcat(bigstringfecha, fecha_hora.year()); // COMO SE CONCATE UN STRING EN ARDUINO???
      strcat(bigstringfecha, "/");
      strcat(bigstringfecha, fecha_hora.month());
      strcat(bigstringfecha, "/");
      strcat(bigstringfecha, fecha_hora.day());
      bigstringhora[0] = 0;
      strcat(bigstringhora, fecha_hora.hour());
      strcat(bigstringhora, ":");
      strcat(bigstringhora, fecha_hora.minute());      
      
      // Guardo en un archivo
      guardarDatos(apiario, colmena, temperatura, humedad, bigstringfecha, bigstringhora);
      
      // Envio datos por GPRS
      enviarDatos(apiario, colmena, temperatura, humedad, bigstringfecha, bigstringhora);
       
      // Demora de 1 segundo
      delay(1000);
  }
  
  
}
