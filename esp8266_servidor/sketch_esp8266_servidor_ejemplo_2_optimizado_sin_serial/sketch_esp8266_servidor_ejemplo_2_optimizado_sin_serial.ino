#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

const char *ssid = "ESP8266";
const char *password = "12345678";
ESP8266WebServer server(80); // Listening in http://192.168.4.1

void response(){
  
  if(server.hasArg("Apiario") && server.hasArg("Colmena") && server.hasArg("Temperatura") && server.hasArg("Humedad")){ 

    if( server.arg("Apiario") == ""  ) return handleNotFound();
    if( server.arg("Colmena") == "" ) return handleNotFound();
    if( server.arg("Temperatura") == "" ) return handleNotFound();
    if( server.arg("Humedad") == "" ) return handleNotFound();
    
    // Con esta línea estamos enviando la información a Arduino a través de Software Serial.
    Serial.println(server.arg("Apiario") + "," + server.arg("Colmena") + "," + server.arg("Temperatura") + "," + server.arg("Humedad"));
       
    server.send(200, "text/html", "<html><body><h1>Successful</h1><a href='/'>Home</a><br>Apiario: " +  server.arg("Apiario") + "<br> Colmena: " + server.arg("Colmena") + "<br> Temperatura: " +server.arg("Temperatura") + "<br> Humedad: "  + server.arg("Humedad") + "</body></html>");
    
  } else {
    
    server.send(400, "text/html", "<html><body><h1>HTTP Error 400</h1><p>Bad request. Please enter a value.</p>" + String(server.args()) + "</body></html>");
  }
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found");       // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}

void setup() {
  Serial.begin(9600);
  server.begin();                         //inicializamos el servidor
  WiFi.mode(WIFI_AP);
  //WiFi.softAP(ssid, password);            //Red con clave, en el canal 1 y visible
  WiFi.softAP(ssid, password, 1, 0, 1);     //  Nombre, clave, canal, is_nombre_visible, cantidad maxima de conexiones simultaneas (maximo 8). 
  
  server.on("/datos",HTTP_GET, response);
  server.on("/datos",HTTP_POST,response);
  server.onNotFound(handleNotFound);
  
}

void loop() {
  server.handleClient();
}
