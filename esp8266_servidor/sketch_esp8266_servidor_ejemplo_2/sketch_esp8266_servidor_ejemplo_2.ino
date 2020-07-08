/*
 * https://forum.arduino.cc/index.php?topic=421214.0
 * https://techtutorialsx.com/2016/10/22/esp8266-webserver-getting-query-parameters/
 * Como obtener el request: https://techtutorialsx.com/2018/02/17/esp32-arduino-web-server-getting-client-ip/
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/* Set these to your desired credentials. */
const char *ssid = "ESP8266";
const char *password = "12345678";

String    inString      ;         // string for incoming serial data
int     stringPos = 0     ;         // string index counter                // what do you use this for?
boolean startRead = false ;         // is reading?

ESP8266WebServer server(80);
WiFiClient client;
/* Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */

void webpage() {
  server.send(200, "text/html", "<html><body><form  name='frm'  method='post'><input type='text' name='x'   ><input type='submit' value='Submit'>   </form></body></html>");
}
void response(){
  //if(server.hasArg("Temperatura") && (server.arg("Temperatura").length()>0)){ // TODO check that it's not longer than 31 characters
  if(server.hasArg("Temperatura")){ // TODO check that it's not longer than 31 characters
    Serial.println("El cliente " + server.client().remoteIP().toString() + " ha enviado la siguiente informacion: ");
    Serial.println("Apiario: " + server.arg("Apiario"));
    //Serial.write("Apiario: " + server.arg("Apiario"));
    Serial.println("Colmena: " + server.arg("Colmena"));
    Serial.println("Temperatura: " + server.arg("Temperatura"));
    Serial.println("Humedad: " + server.arg("Humedad"));
    inString = server.arg("Temperatura");
    server.send(200, "text/html", "<html><body><h1>Successful</h1><a href='/'>Home</a><br>Apiario: " +  server.arg("Apiario") + "<br> Colmena: " + server.arg("Colmena") + "<br> Temperatura: " +server.arg("Temperatura") + "<br> Humedad: "  + server.arg("Humedad") + "</body></html>");
  } else {
    server.send(400, "text/html", "<html><body><h1>HTTP Error 400</h1><p>Bad request. Please enter a value.</p>" + String(server.args()) + "</body></html>");
    Serial.println(server.args());
  }
}

void setup() {
  Serial.begin(9600);
  server.begin();                         //inicializamos el servidor
  WiFi.mode(WIFI_AP);
  //WiFi.softAP(ssid, password);            //Red con clave, en el canal 1 y visible
  WiFi.softAP(ssid, password, 1, 0, 8);     //  Nombre, clave, canal, is_nombre_visible, cantidad maxima de conexiones simultaneas (maximo 8). 
  Serial.println();
  Serial.print("Direccion IP Access Point - por defecto: ");      //Imprime la dirección IP
  Serial.println(WiFi.softAPIP()); 
  Serial.print("Direccion MAC Access Point: ");                   //Imprime la dirección MAC
  Serial.println(WiFi.softAPmacAddress()); 
  //server.on("/datos",HTTP_GET, webpage);
  server.on("/datos",HTTP_GET, response);
  server.on("/datos",HTTP_POST,response);
  Serial.println();
  Serial.println("HTTP server started");
  // Probando rama develop
}

void loop() {
  server.handleClient();
}
