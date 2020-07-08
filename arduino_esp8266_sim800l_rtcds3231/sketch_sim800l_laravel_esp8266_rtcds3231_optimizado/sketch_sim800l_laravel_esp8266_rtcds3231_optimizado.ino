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
    Serial.begin(9600);
    
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
      
      
      if(informacion.indexOf("Apiario") > 0)
      {
        start = informacion.indexOf("Apiario");
        end = informacion.indexOf("\n",start);
        apiario = informacion.substring(start,end);
        start = apiario.indexOf(": ") + 2;
        apiario = apiario.substring(start,end);
        apiario.trim();
      }
      if(informacion.indexOf("Colmena") > 0)
      {
        start = informacion.indexOf("Colmena");
        end = informacion.indexOf("\n",start);
        colmena = informacion.substring(start,end);
        start = colmena.indexOf(": ") + 2;
        colmena = colmena.substring(start,end);
        colmena.trim();
      }
      if(informacion.indexOf("Temperatura") > 0)
      {
        start = informacion.indexOf("Temperatura");
        end = informacion.indexOf("\n",start);
        temperatura = informacion.substring(start,end);
        start = temperatura.indexOf(": ") + 2;
        temperatura = temperatura.substring(start,end);
        temperatura.trim();
      }
      if(informacion.indexOf("Humedad") > 0)
      {
        start = informacion.indexOf("Humedad");
        end = informacion.indexOf("\n",start);
        humedad = informacion.substring(start,end);
        start = humedad.indexOf(": ") + 2;
        humedad = humedad.substring(start,end);
        humedad.trim();
      }
      
      Serial.println(apiario);

      if( apiario != "" && colmena != "" && temperatura != "" && humedad != "" ) enviarDatos();
      delay(5000);
    }
}



/** Valida comandos AT **/
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


/** Envía datos por GPRS al Servidor web **/
void enviarDatos() {

    

    memset(char_array_apiario, 0, sizeof(char_array_apiario));
    memset(char_array_colmena, 0, sizeof(char_array_colmena));
    memset(char_array_temperatura, 0, sizeof(char_array_temperatura));
    memset(char_array_humedad, 0, sizeof(char_array_humedad));   
    memset(buf1, 0, sizeof(buf1)); 
    
    DateTime now = rtc.now();
    sprintf(buf1, "%02d:%02d:%02d %02d/%02d/%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());

    /* Pasamos los String recibidos desde el ESP8266 a arreglo de caracteres */
    for( i = 0; i < humedad.length(); i++ ) {
      if( i < 6 ) char_array_humedad[i] = humedad.charAt(i);
    }

    for( i =0; i < temperatura.length(); i++ ) {
      if( i < 5 ) char_array_temperatura[i] = temperatura.charAt(i);
    }

    for( i =0; i < colmena.length(); i++ ) {
      if( i < 2 ) char_array_colmena[i] = colmena.charAt(i);
    }

    for( i =0; i < apiario.length(); i++ ) {
      if( i < 2 ) char_array_apiario[i] = apiario.charAt(i);
    }

   
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
    //serialSIM800L.println("AT+HTTPPARA=\"URL\",\"http://colmenainteligente.ddns.net/abeja2/?apiario_id=" + String(char_array_apiario).substring(0,2) + "&colmena_id=" + String(char_array_colmena).substring(0,2) + "&temperatura=" + String(char_array_temperatura).substring(0,5) +"&humedad=" + String(char_array_humedad).substring(0,5) + "\""); // Envío En comando AT  
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

    Serial.println("Enviado.");
}
