#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

/* Set these to your desired credentials. */
const char *ssid = "AD5C0A";  //ENTER YOUR WIFI SETTINGS    AD5C0A || ESP8266
const char *password = "4515447838"; // 4515447838  || 12345678

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

}

//=======================================================================
//                    Main Program Loop
//=======================================================================
void loop() {
  // Esperamos 5 segundos entre medidas
  delay(5000);
  
  HTTPClient http;    //Declare object of class HTTPClient

  String apiario, colmena, temperatura, humedad, postData;
  apiario = "1";
  colmena = "14";
  temperatura = "22";
  humedad = "35";

  //Post Data
  postData = "apiario=" + apiario + "&colmena=" + colmena + "&temperatura=" + temperatura + "&humedad=" + humedad;

  http.begin("http://192.168.1.68:8000/datos");  //Specify request destination    
  http.addHeader("Content-Type", "text/html"); //Specify content-type header
  
  int httpCode = http.POST(postData);   //Send the request
  String payload = http.getString();    //Get the response payload

  Serial.println(httpCode);   //Print HTTP return code
  Serial.println(payload);    //Print request response payload

  http.end();  //Close connection
  
  delay(5000);  //Post Data at every 5 seconds
}
//=======================================================================
