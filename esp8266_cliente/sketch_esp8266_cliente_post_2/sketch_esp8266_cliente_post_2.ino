// Incluimos librería
#include <ESP8266WiFi.h>
#include <DHT.h>

const char* ssid = "ESP8266"; // Mi ssid
const char* password = "12345678"; // Mi Password
const char* host = "192.168.4.1";

// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht11(DHTPIN, DHTTYPE);

#define BAUD_RATE 9600


void setup() {
  // Inicializamos comunicación serie
  Serial.begin(BAUD_RATE);
  delay(10);
  Serial.println();

  // Nos conectamos a la red WIFI
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.println();
  Serial.print("Conectandos a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("conectados a la WiFi");
  
  
  // Comenzamos el sensor DHT
  dht11.begin();
}

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

  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String apiario, colmena, temperatura, humedad, postData;
  apiario = "1";
  colmena = "14";
  temperatura = String(t);
  humedad = String(h);

  // Ejemplo: https://stackoverflow.com/questions/41371156/esp8266-and-post-request
  //Post Data
  //postData = "Apiario=" + apiario + "&Colmena=" + colmena + "&Temperatura=" + temperatura + "&Humedad=" + humedad;
  postData = "Temperatura=" + temperatura;
  Serial.println(postData);
  client.println("POST /datos HTTP/1.1");
  client.println("Host: http://192.168.4.1:80");
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.print("Content-Length: ");
  client.println(postData.length());
  client.println();
  client.println(postData);
  
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");

  delay(500);

}
