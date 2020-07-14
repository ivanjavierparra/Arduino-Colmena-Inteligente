// Sketch del Nodo Servidor.
// ==========================
// Permite la comunicación serial entre Arduino y 3 módulos: el ESP8266, el SIM800L y el RTCDS3231.
//
// Hace lo siguiente: Se reciben paquetes de datos del Nodo Colmena desde el ESP8266, se consulta la fecha y la hora con el RTCDS3231, 
// y se envía dicho paquete por el SIM800L hacia la aplicación Laravel (Servidor web).
//
//  Pins
//  Arduino pin 2 (RX) to ESP8266 TX
//  Arduino pin 3 to voltage divider then to ESP8266 RX
//  Connect GND from the Arduiono to GND on the ESP8266
//  Pull ESP8266 CH_PD HIGH
//
// When a command is entered in to the serial monitor on the computer 
// the Arduino will relay it to the ESP8266
//
 
#include <SoftwareSerial.h>
#include <Wire.h>   // incluye libreria para interfaz I2C
#include <RTClib.h>   // incluye libreria para el manejo del modulo RTC
#include <SPI.h>    // incluye libreria interfaz SPI
#include <SD.h>     // incluye libreria para tarjetas SD
#define SSpin 53    // Slave Select en pin digital 53

SoftwareSerial ESPserial(10,11); // RX | TX  => TX del ESP al pin 2 y RX del ESP al pin 3
SoftwareSerial serialSIM800L(5,4); // TX | RX => TX del SIM al pin 5 y RX del SIM al pin 4
RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231
File archivo;     // objeto archivo del tipo File

String apiario = "abcdef";
String colmena = "abcdef";
String temperatura = "abcdef";
String humedad = "abcdef";

int start = 0;
int end = 0;

char char_array_apiario[2] = "";
char char_array_colmena[2] = "";
char char_array_temperatura[5] = "";
char char_array_humedad[6] = "";
char buf1[20];

void setup() 
{
    Serial.begin(9600);     // communication with the host computer
    //while (!Serial)   { ; }

    if (!SD.begin(SSpin)) {     // inicializacion de tarjeta SD
      Serial.println("fallo en inicializacion del Modulo Micro SD!");// si falla se muestra texto correspondiente y
      return;         // se sale del setup() para finalizar el programa
    }
  
    Serial.println("Inicializacion correcta del Modulo MicroSD");  // texto de inicializacion correcta

    if (! rtc.begin()) {       // si falla la inicializacion del modulo
      Serial.println("Modulo RTC no encontrado !");  // muestra mensaje de error
      while (1);         // bucle infinito que detiene ejecucion del programa
    }

    Serial.println("Inicializacion correcta del Modulo RTC");  // texto de inicializacion correcta

    // funcion que permite establecer fecha y horario
    // al momento de la compilacion.
    rtc.adjust(DateTime(__DATE__, __TIME__));  

    // Start the software serial for communication with the ESP8266 and SIM800L
    serialSIM800L.begin(9600); // Inicio de comunicación Serial    
    ESPserial.begin(9600);  
    
    
    Serial.println("Nodo Servidor activo...");
    Serial.println("");    
}
 
void loop() 
{
    /***
     * El ESP8266 Servidor me va a pasar por comunicación serial un String grande, con el siguiente formato:
     * 
     * 16:42:13.852 -> 11,12,35.55,100.00 
     * El orden de los datos indica Apiario, Colmena, Temperatura, Humedad.
     */
  
    // listen for communication from the ESP8266 and then write it to the serial monitor
    if ( ESPserial.available() > 0 )   {  

      apiario = "";
      colmena = "";
      temperatura = "";
      humedad = "";
      
      String informacion = ESPserial.readString(); // serial comunication: get info from ESP8266.
      Serial.println(informacion);

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

      if( apiario == "" && colmena == "" && temperatura == "" && humedad == "" ) {
        //Serial.println("El Paquete de datos recibido es incompleto.");
      }
      else {
        Serial.println(apiario);
        Serial.println(colmena);
        Serial.println(temperatura);
        Serial.println(humedad);
        validarDatos();
      }

      //delay(5000); // Esto lo sacaría, porque congela el micro de Arduino, lo reemplazaría por millis().
    }
}

/** 
 *  Consulta la hora y busca en el archivo del Nodo Colmena si existe 
 *  un dato de temperatura y humedad para esa hora y día. Si existe, se lo descarta, sino 
 *  lo agrega al archivo y se lo envía por GPRS al Servidor.
 *  
 *  Nota: cada Nodo Colmena tiene su archivo. La primer fila del archivo es el día. Después se interpreta cada fila
 *  restante como la hora del día. Por ejemplo, la fila 2 sería el horario 00:00. La fila 15 sería el horario 14:00.
 */
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

  if( filas <= fila_hora ) { Serial.println("Ya existen datos del Nodo Colmena " + colmena + " para esta hora."); return;} // datos ya cargados

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


/**
 * Valida comandos AT.
 * 
 */
void respuesta(){
  while( serialSIM800L.available() ) // Mientras la comunicación está
  {
    if( serialSIM800L.available() > 0 ) // Si se reciben datos 
    {
      //Serial.write(serialSIM800L.read()); // imprime lo que recibe  
      //Serial.println(serialSIM800L.read());
      Serial.write(char (serialSIM800L.read()));
    }
  }
  Serial.flush();
  serialSIM800L.flush();
}




/**
 * Envía datos por GPRS al Servidor web.
 * 
 */
void enviarDatos() {

    limpiar_arreglos();
    
    obtener_fecha_hora();

    convertir_string_a_chart();

    Serial.println("Enviando paquete..."); 
   
    /* Comprobamos comunicación con comandos AT */
    serialSIM800L.println("AT"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();

    // serialSIM800L.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\""); /* Connection type GPRS */
    
    /* Inicializamos comunicación */
    serialSIM800L.println("AT+SAPBR=3,1,\"APN\",\"CMNET\""); // Envío comando AT // igprs.claro.com.ar
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    serialSIM800L.println("AT+SAPBR=1,1"); // Envío comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/

    /* Nos comunicamos con el Servidor */
    serialSIM800L.println("AT+HTTPINIT"); // Envío comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    serialSIM800L.println("AT+HTTPPARA=\"CID\",1"); // Envío comando AT
    delay(1000); // Espero un momento por la respuesta
    respuesta();
    /*********/
    Serial.println("AT+HTTPPARA=\"URL\",\"http://colmenainteligente.ddns.net/abeja2/?apiario_id=" + String(char_array_apiario).substring(0,2) + "&colmena_id=" + String(char_array_colmena).substring(0,2) + "&temperatura=" + String(char_array_temperatura).substring(0,5) +"&humedad=" + String(char_array_humedad).substring(0,6) + "&fecha_hora=" + String(buf1).substring(0,20) + "\""); // Envío En comando AT  
    serialSIM800L.println("AT+HTTPPARA=\"URL\",\"http://colmenainteligente.ddns.net/abeja2/?apiario_id=" + String(char_array_apiario).substring(0,2) + "&colmena_id=" + String(char_array_colmena).substring(0,2) + "&temperatura=" + String(char_array_temperatura).substring(0,5) +"&humedad=" + String(char_array_humedad).substring(0,6) + "&fecha_hora=" + String(buf1).substring(0,20) + "\""); // Envío En comando AT  
    delay(3000); // Espero un momento por la respuesta
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
    
    Serial.println("Paquete enviado.");
    
}

/**
 * Obtiene la fecha y hora actual a partir del RTC DS3231
 */
void obtener_fecha_hora() {
   DateTime now = rtc.now();
   sprintf(buf1, "%02d:%02d:%02d %02d,%02d,%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());
   Serial.println(buf1);
}


/**
 * Convierte la información recibida por comunicación serial con el ESP8266, que está
 * en String, a un arreglo de caracteres. Esto es necesario porque en Arduino hay memoria
 * limitada, y hay que definir con anterioridad la longitud del arreglo de charts para que 
 * puedan ser enviados por el SIM800L. 
 */
void convertir_string_a_chart() {
    /* Pasamos los String recibidos desde el ESP8266 a arreglo de caracteres */
    int i;
    for( i =0; i < humedad.length(); i++ ) {
      if( i < 6 ) char_array_humedad[i] = humedad.charAt(i);
      else break;
    }

    for( i =0; i < temperatura.length(); i++ ) {
      if( i < 5 ) char_array_temperatura[i] = temperatura.charAt(i);
      else break;
    }

    for( i =0; i < colmena.length(); i++ ) {
      if( i < 2 ) char_array_colmena[i] = colmena.charAt(i);
      else break;
    }

    for( i =0; i < apiario.length(); i++ ) {
      if( i < 2 ) char_array_apiario[i] = apiario.charAt(i);
      else break;
    }
}


/**
 * Setea cada valor del arreglo de caracteres a vacio.
 */
void limpiar_arreglos() {
  memset(char_array_apiario, 0, sizeof(char_array_apiario));
  memset(char_array_colmena, 0, sizeof(char_array_colmena));
  memset(char_array_temperatura, 0, sizeof(char_array_temperatura));
  memset(char_array_humedad, 0, sizeof(char_array_humedad));   
  memset(buf1, 0, sizeof(buf1));   
}
