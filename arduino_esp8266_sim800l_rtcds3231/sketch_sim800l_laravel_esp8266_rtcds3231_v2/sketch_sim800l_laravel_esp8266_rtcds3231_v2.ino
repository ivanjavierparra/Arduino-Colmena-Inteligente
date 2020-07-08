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

SoftwareSerial ESPserial(2, 3); // RX | TX  => TX del ESP al pin 2 y RX del ESP al pin 3
SoftwareSerial serialSIM800L(5,4); // TX | RX => TX del SIM al pin 5 y RX del SIM al pin 4
RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231

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

    if (! rtc.begin()) {       // si falla la inicializacion del modulo
      Serial.println("Modulo RTC no encontrado !");  // muestra mensaje de error
      while (1);         // bucle infinito que detiene ejecucion del programa
    }

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
     * 16:42:13.852 -> El cliente 192.168.4.2 ha enviado la siguiente informacion: 
       16:42:13.926 -> Apiario: 11
       16:42:13.926 -> Colmena: 12
       16:42:13.966 -> Temperatura: 35.55
       16:42:13.966 -> Humedad: 80.00
     */
  
    // listen for communication from the ESP8266 and then write it to the serial monitor
    if ( ESPserial.available() > 0 )   {  

      apiario = "";
      colmena = "";
      temperatura = "";
      humedad = "";
      
      String informacion = ESPserial.readString(); // serial comunication: get info from ESP8266.
      Serial.println(informacion);
      
      if(informacion.indexOf("Apiario") > 0) 
      {
        Serial.println("Procesando Apiario...");        
        start = informacion.indexOf("Apiario");
        end = informacion.indexOf("\n",start);
        apiario = informacion.substring(start,end); // De todo el String, me quedo con "Apiario: 12"
        start = apiario.indexOf(": ") + 2;
        apiario = apiario.substring(start,end); // Me quedo solo con "12"
        apiario.trim(); // Elimino espacios en blanco
      }
      if(informacion.indexOf("Colmena") > 0) 
      {
        Serial.println("Procesando Colmena...");
        start = informacion.indexOf("Colmena");
        end = informacion.indexOf("\n",start);
        colmena = informacion.substring(start,end); // De todo el String, me quedo con "Colmena: 13"
        start = colmena.indexOf(": ") + 2;
        colmena = colmena.substring(start,end); // colmena = 13
        colmena.trim();
      }
      if(informacion.indexOf("Temperatura") > 0) 
      {
        Serial.println("Procesando Temperatura..."); 
        start = informacion.indexOf("Temperatura");
        end = informacion.indexOf("\n",start);
        temperatura = informacion.substring(start,end); // De todo el String, me quedo con "Temperatura: 35.55"
        start = temperatura.indexOf(": ") + 2;
        temperatura = temperatura.substring(start,end); // temperatura = "35.55"
        temperatura.trim();
      }
      if(informacion.indexOf("Humedad") > 0) 
      {
        Serial.println("Procesando Humedad...");
        start = informacion.indexOf("Humedad");
        end = informacion.indexOf("\n",start);
        humedad = informacion.substring(start,end); // De todo el String, me quedo con "Humedad: 80.00"
        start = humedad.indexOf(": ") + 2;
        humedad = humedad.substring(start,end); // humedad = "80.00"
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
        enviarDatos(); // Envio datos por GPRS  
      }

      //delay(5000); // Esto lo sacaría, porque congela el micro de Arduino, lo reemplazaría por millis().
    }
}



/**
 * Valida comandos AT.
 * 
 */
void respuesta(){
  while( serialSIM800L.available() ) // Mientras la comunicación está
  {
    if( serialSIM800L.available() > 0 ) // Si se reciben datos (mayor)
    {
      //Serial.write(serialSIM800L.read()); // imprime lo que recibe en 
      Serial.println(serialSIM800L.read());
    }
  }
  Serial.flush();
  serialSIM800L.flush();
}




/**
 * Envía datos por GPRS al Servidor web
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
