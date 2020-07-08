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
 * Enviar datos por GET o POST:
 * https://www.youtube.com/watch?v=uLkpILpQKuY&t=592s
 * https://github.com/ioticos/esp32_esp8266_post_request/blob/master/src/main.cpp
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


//=======================================================================
//                    Power on setup
//=======================================================================

void setup() {
  delay(1000);
  Serial.begin(BAUD_RATE);
  WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
  delay(1000);
  WiFi.mode(WIFI_STA);        //This line hides the viewing of ESP as wifi hotspot
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  Serial.print("Connecting");
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP

  // Comenzamos el sensor DHT
  dht11.begin();
}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  // Esperamos 5 segundos entre medidas
  delay(5000);

  // Leemos la humedad relativa
  float h = dht11.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht11.readTemperature();

  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");   
  }
  
  HTTPClient http;    //Declare object of class HTTPClient

  String apiario, colmena, temperatura, humedad, postData;
  apiario = "1";
  colmena = "14";
  temperatura = String(t);
  humedad = String(h);

  // Query Parameters a enviar por GET
  postData = "Apiario=" + apiario + "&Colmena=" + colmena + "&Temperatura=" + temperatura + "&Humedad=" + humedad;
       
  http.begin("http://192.168.4.1:80/datos"); // Specify request destination and the query parameters 
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Specify content-type header

  int httpCode = http.POST(postData);   // Send the request: also we can do http.POST(postData)
  String payload = http.getString();    // Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection
  
  delay(5000);  //Post Data at every 5 seconds
}
//=======================================================================
