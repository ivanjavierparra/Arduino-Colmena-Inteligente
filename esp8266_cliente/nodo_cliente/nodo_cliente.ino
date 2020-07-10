/*
 * Este sketch representa a un Nodo Xliente: ESP8266 + DHT11.
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
const char *host = "192.168.4.1";   //https://circuits4you.com website or IP address of server: 192.168.1.68 || 192.168.4.1

/* Definimos el pin digital donde se conectará el ESP8266 al DHT11: GPIO2 */
#define DHTPIN 2
/* Elegimos el tipo de sensor: DHT11 o DHT22 */
#define DHTTYPE DHT11
/* Inicializamos el sensor DHT11 */
DHT dht11(DHTPIN, DHTTYPE);

/* Definimos el BAUD_RATE: */
#define BAUD_RATE 9600

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = 1000 * 60 * 2;           // interval at which to blink (milliseconds)


//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  Serial.begin(BAUD_RATE);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  // Comenzamos el sensor DHT
  dht11.begin();
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {


  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) { 

        // save the last time you blinked the LED
        previousMillis = currentMillis;
        

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
      
        Serial.print("Iniciado conexión con el Nodo Servidor");
        // Wait for connection
        while (WiFi.status() != WL_CONNECTED) {
          delay(500);
          Serial.print(".");
        }
      
        //If connection successful show IP address in serial monitor
        Serial.println("");
        Serial.print("Conectado a ");
        Serial.println(ssid);
        Serial.print("Direccion IP: ");
        Serial.println(WiFi.localIP());  //IP address assigned to your ESP
      
      
      
        
        
        HTTPClient http;    //Declare object of class HTTPClient
      
        String apiario, colmena, temperatura, humedad, postData;
        apiario = "1";
        colmena = "6";
        temperatura = String(t);
        humedad = String(h);
      
        // Query Parameters a enviar por GET
        postData = "Apiario=" + apiario + "&Colmena=" + colmena + "&Temperatura=" + temperatura + "&Humedad=" + humedad;
            
        http.begin("http://192.168.4.1:80/datos?" + postData); // Inicio la conexión          
        http.addHeader("Content-Type", "text/html"); // Specify content-type header  
      
        int httpCode = http.GET();   // Send the request: also we can do http.POST(postData)
      
        if( httpCode > 0 ) {
          Serial.println("Peticion GET exitosa.");
          //if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
            Serial.println(httpCode);
            String payload = http.getString();  // Obtener respuesta
            Serial.println(payload);  // Mostrar respuesta por serial
          //}
          
        }
        else {
          Serial.println("Peticion GET fallida.");
        }
            
             
        
        http.end();  //Close connection
      
        WiFi.disconnect(true);

  }
  
  
}
//=======================================================================
