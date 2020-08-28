/*
 * Este sketch representa a un Nodo Colmena: ESP8266 + DHT11.
 * Lo que hace es leer datos de Temperatura y Humedad cada cierto tiempo y los envía, 
 * junto a su identificación (Número de Apiario y Número de Colmena), 
 * al Nodo Servidor como parámetros de una petición HTTP GET. 
 * 
 * Links que utilice:
 * https://circuits4you.com 
 * https://circuits4you.com/2018/03/10/esp8266-nodemcu-post-request-data-to-website/
 * https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
 * https://www.instructables.com/id/Arduino-Esp8266-Post-Data-to-Website/
 * https://github.com/esp8266/Arduino/issues/1390
 * https://stackoverflow.com/questions/41371156/esp8266-and-post-request
 *  
 * Creado por Iván Parra. 
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

/* WIFI Settings */
const char *ssid = "ESP8266";  // AD5C0A || ESP8266
const char *password = "12345678"; // 4515447838  || 12345678

/* Dirección IP del Nodo Servidor */ 
const char *host = "192.168.4.1";   

/* Definimos el pin digital donde se conectará el ESP8266 al DHT11: GPIO2 */
#define DHTPIN 2
/* Elegimos el tipo de sensor: DHT11 o DHT22 */
#define DHTTYPE DHT11
/* Inicializamos el sensor DHT11 */
DHT dht11(DHTPIN, DHTTYPE);

/* Definimos el BAUD_RATE: */
#define BAUD_RATE 9600 

unsigned long previousMillis = 0;               // Lleva el conteo de los milisegundos de ejecución del sketch.
const long interval = 1000 * 60 * 2;           // Intervalo de 2 minutos para realizar la medición y enviarla al Nodo Servidor.


//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  Serial.begin(BAUD_RATE);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  dht11.begin(); // Inicializamos el sensor DHT11.
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {

  // Activo el Modem Sleep.
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  Serial.println("WiFi is down");
    

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) { 

        // save the last time you blinked the LED
        previousMillis = currentMillis;

        
        // Desactivo el Model Sleep.
        WiFi.forceSleepWake();
        delay(1);
        // Bring up the WiFi connection
        WiFi.mode(WIFI_STA);


        
        // Leemos la humedad relativa
        float h = dht11.readHumidity();
        // Leemos la temperatura en grados centígrados (por defecto)
        float t = dht11.readTemperature();
      
        // Comprobamos si ha habido algún error en la lectura
        if (isnan(h) || isnan(t)) {
          Serial.println("Error obteniendo los datos del sensor DHT11");   
        }
      
        
        //Connect to your WiFi router
        WiFi.begin(ssid, password);    
        Serial.println("");
        Serial.print("Iniciando conexión con el Nodo Servidor");
        
        
        
        // Protocolo de reintentos de conexión al sevidor.
        uint8_t i = 0;
        while (WiFi.status() != WL_CONNECTED && i++ < 20 ) { //wait 10 seconds
          delay(500);
          Serial.print(".");
        }

        
        // Si pasaron más de 10 segundos, considero inválidos los datos.
        if (i == 21) {
          Serial.print("Could not connect to Server Node.");
        }
        else{
           
          //If connection successful show IP address in serial monitor
          Serial.println("");
          Serial.print("Conectado a ");
          Serial.println(ssid);
          Serial.print("Direccion IP: ");
          Serial.println(WiFi.localIP());  //IP address assigned to your ESP
      
    
          //Declare object of class HTTPClient
          HTTPClient http;    
        
          
          String apiario, colmena, temperatura, humedad, postData;
          apiario = "1";
          colmena = "6";
          temperatura = String(t);
          humedad = String(h);
          // Query Parameters a enviar por GET
          postData = "Apiario=" + apiario + "&Colmena=" + colmena + "&Temperatura=" + temperatura + "&Humedad=" + humedad;
              
          
          // Envio datos por GET
          http.begin("http://192.168.4.1:80/datos?" + postData); // Inicio la conexión          
          http.addHeader("Content-Type", "text/html"); // Specify content-type header  
          int httpCode = http.GET();   // Send the request: also we can do http.POST(postData)


          
          // Analizo la respuesta del Servidor.
          if( httpCode > 0 ) {
              
              Serial.println("Peticion GET exitosa.");
              Serial.println(httpCode);
              String payload = http.getString();  // Obtener respuesta
              Serial.println(payload);  // Mostrar respuesta por serial
            
          }
          else {
            
            // Peticion fallida: Activo protocolo de reenvio de datos.
            Serial.println("Peticion GET fallida.");
            for( int i = 0; i < 6; i++ ) {
              delay(1000);
              int httpCode = http.GET();
              if( httpCode > 0 ) break;            
            }
            
          }
            
             
          http.end();  //Close connection HTTP
        
          WiFi.disconnect(true); // Disconnect from Access Point
          
        }
  }
  
  
}
//=======================================================================
