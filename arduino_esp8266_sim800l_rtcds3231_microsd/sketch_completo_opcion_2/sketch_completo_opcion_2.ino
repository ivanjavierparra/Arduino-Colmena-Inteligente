#include <SoftwareSerial.h>
#include <Wire.h>   // incluye libreria para interfaz I2C
#include <RTClib.h>   // incluye libreria para el manejo del modulo RTC
#include <SPI.h>    // incluye libreria interfaz SPI
#include <SD.h>     // incluye libreria para tarjetas SD
#define SSpin 10    // Slave Select en pin digital 10

SoftwareSerial ESPserial(2, 3); // RX | TX  => TX del ESP al pin 2 y RX del ESP al pin 3
SoftwareSerial serialSIM800L(5,4); // TX | RX => TX del SIM al pin 5 y RX del SIM al pin 4
RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231
File archivo;     // objeto archivo del tipo File

String apiario = "abcdef";
String colmena = "abcdef";
String temperatura = "abcdef";
String humedad = "abcdef";
int i = 0;
char char_array_apiario[2] = "";
char char_array_colmena[2] = "";
char char_array_temperatura[5] = "";
char char_array_humedad[6] = "";
char buf1[20];
int start = 0;
int end = 0;

void setup() 
{ 
    if (!SD.begin(SSpin)) {     // inicializacion de tarjeta SD
      return;         // se sale del setup() para finalizar el programa
    }
    
    if (! rtc.begin()) {    
      while (1);         
    }
    
    rtc.adjust(DateTime(__DATE__, __TIME__));  
    serialSIM800L.begin(9600); 
    ESPserial.begin(9600);  
}
 
void loop() 
{
    if ( ESPserial.available() > 0 )   {  

      apiario = "";
      colmena = "";
      temperatura = "";
      humedad = "";
          
      String informacion = ESPserial.readString();
      
      if(informacion.indexOf(",") > 0) {
        int i1 = informacion.indexOf(","); // primer coma
        apiario = informacion.substring(0,i1);
        apiario.trim();
        int i2 = informacion.indexOf(",", i1 + 1);
        colmena = informacion.substring(i1 + 1,i2);
        colmena.trim();
        int i3 = informacion.indexOf(",", i2 + 1);
        temperatura = informacion.substring(i2+1, i3);
        temperatura.trim();
        int i4 = informacion.indexOf("\n", i3 + 1);
        humedad = informacion.substring(i3+1, i4);
        humedad.trim();
      } 

      if( apiario != "" && colmena != "" && temperatura != "" && humedad != "" ) validarDatos();
    }
}


void validarDatos() {
  String nombre_archivo = colmena + ".txt";
  int filas = 0;  
  String fecha_archivo = "";

  // Recorremos archivos para contar sus filas y obtener la fecha del archivo.
  archivo = SD.open(nombre_archivo, FILE_WRITE);
  archivo.close(); 
  archivo = SD.open(nombre_archivo);
  if (archivo) { 
    while (archivo.available()) {
      char c = archivo.read();
      if(c == '\n')filas++;
      if( filas == 0 ) fecha_archivo.concat(c);
    }
  }
  archivo.close();

  // Obtenemos dia de hoy.
  DateTime now = rtc.now();
  String fecha_hoy = String(now.day(),DEC);
  fecha_archivo.trim();
  fecha_hoy.trim();

  // Si la fecha del archivo es distinta a la fecha de hoy, elimino el archivo
  // y le agrego la fecha de hoy.
  if( fecha_archivo != fecha_hoy ) {
    
    SD.remove(nombre_archivo);
    archivo = SD.open(nombre_archivo, FILE_WRITE); 
    if (archivo) { 
       archivo.println(fecha_hoy);
    }
    archivo.close();
    filas = 1;
  } 

  // Seteo la fila del archivo donde ira el nuevo dato de tyh recibido.
  int fila_hora = int(now.hour()) + 2;
  
  if( filas == fila_hora ) return; // datos ya cargados

  // Si hay saltos en el archivo, por ej: si el último dato es a
  // las 05:00 y el hora actua es 10:00, entonces completo con
  // espacios en blanco entre las 05:00 y las 10:00.
  int filas_a_cargar = fila_hora - filas - 1;

  archivo = SD.open(nombre_archivo, FILE_WRITE); 
  if( archivo ) {
    for( int i = 0; i< filas_a_cargar; i++ ) {
       archivo.println();  
    }

    archivo.println(temperatura + "," + humedad);
  }
  
  archivo.close();

  enviarDatos();
}


/** Envía datos por GPRS al Servidor web **/
void enviarDatos() {

    // Clean arrays
    char_array_apiario[0] = 0;
    char_array_colmena[0] = 0;
    char_array_temperatura[0] = 0;
    char_array_humedad[0] = 0;   
    buf1[0] = 0; 
    
    DateTime now = rtc.now();
    sprintf(buf1, "%02d:%02d:%02d %02d,%02d,%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());

    /* Pasamos los String recibidos desde el ESP8266 a arreglo de caracteres */
    for( i=0; i < humedad.length(); i++ ) {
      if( i < 6 ) char_array_humedad[i] = humedad.charAt(i);
    }

    for( i=0; i < temperatura.length(); i++ ) {
      if( i < 5 ) char_array_temperatura[i] = temperatura.charAt(i);
    }

    for( i=0; i < colmena.length(); i++ ) {
      if( i < 2 ) char_array_colmena[i] = colmena.charAt(i);
    }

    for( i=0; i < apiario.length(); i++ ) {
      if( i < 2 ) char_array_apiario[i] = apiario.charAt(i);
    }

    /* Comprobamos comunicación con comandos AT */
    serialSIM800L.println("AT"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    
    /* Inicializamos comunicación */
    serialSIM800L.println("AT+SAPBR=3,1,\"APN\",\"CMNET\""); // Envío En comando AT //CMNET
    delay(3000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+SAPBR=1,1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/

    /* Nos comunicamos con el Servidor */
    serialSIM800L.println("AT+HTTPINIT"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+HTTPPARA=\"CID\",1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+HTTPPARA=\"URL\",\"http://colmenainteligente.ddns.net/abeja2/?apiario_id=" + String(char_array_apiario).substring(0,2) + "&colmena_id=" + String(char_array_colmena).substring(0,2) + "&temperatura=" + String(char_array_temperatura).substring(0,5) +"&humedad=" + String(char_array_humedad).substring(0,6) + "&fecha_hora=" + String(buf1).substring(0,20) + "\""); // Envío En comando AT  
    delay(3000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+HTTPACTION=0"); // Envío En comando AT
    delay(3000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+HTTPREAD"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/ 
    serialSIM800L.println("AT+HTTPTERM"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    
    /* Cerramos la comunicación */
    serialSIM800L.println("AT+SAPBR=0,1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
}
