#include <SoftwareSerial.h>
#include <Wire.h>   // incluye libreria para interfaz I2C
#include <RTClib.h>   // incluye libreria para el manejo del modulo RTC

SoftwareSerial ESPserial(2, 3); // RX | TX  => TX del ESP al pin 2 y RX del ESP al pin 3
SoftwareSerial serialSIM800L(5,4); // TX | RX => TX del SIM al pin 5 y RX del SIM al pin 4
RTC_DS3231 rtc;     // crea objeto del tipo RTC_DS3231

char buf1[20];
char apiario[3] = "";
char colmena[3] = "";
char temperatura[7] = "";
char humedad[8] = "";
boolean newData = false;
const byte numChars = 30;
char receivedChars[numChars];   // an array to store the received data
char endMarker = '\n';


    
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
    if ( ESPserial.available() > 10 )   {  
      cleanArrays();
      leerBufferESP();
      parsearDatos();
      if( String(apiario) != "" && String(colmena) != "" && String(temperatura) != "" && String(humedad) != "" ) enviarDatos();
    }
}

void cleanArrays(){
  // Clean arrays
  apiario[0] = '\0';
  colmena[0] = '\0';
  temperatura[0] = '\0';
  humedad[0] = '\0';
  buf1[0] = '\0';
}


void leerBufferESP() {
    static byte ndx = 0;
    
    while (ESPserial.available() > 0 && newData == false) {
        char rc = ESPserial.read();

        if (rc != endMarker) {
            receivedChars[ndx] = rc;
            ndx++;
            if (ndx >= numChars) {
                ndx = numChars - 1;
            }
        }
        else {
            receivedChars[ndx] = '\0'; // terminate the string
            ndx = 0;
            newData = true;
        }
    }
}

void parsearDatos() {
  if (newData == true) {
        Serial.println(receivedChars);
        char * token;
        token = strtok(receivedChars,",");
        strcpy(apiario, token);
        token = strtok(NULL,",");          
        strcpy(colmena, token);    
        token = strtok(NULL,",");          
        strcpy(temperatura, token); 
        token = strtok(NULL,",");          
        strcpy(humedad, token);   
             
        newData = false;
    }
}

/** Envía datos por GPRS al Servidor web **/
void enviarDatos() {

    
    DateTime now = rtc.now();
    sprintf(buf1, "%02d:%02d:%02d %02d,%02d,%02d",  now.hour(), now.minute(), now.second(), now.day(), now.month(), now.year());

    Serial.println(apiario);
    Serial.println(colmena);
    Serial.println(temperatura);
    Serial.println(humedad);
    Serial.println(buf1);

    /* Comprobamos comunicación con comandos AT */
    serialSIM800L.println("AT"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    
    /* Inicializamos comunicación */
    serialSIM800L.println("AT+SAPBR=3,1,\"APN\",\"CMNET\""); // Envío En comando AT //CMNET
    delay(3000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+SAPBR=1,1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/

    /* Nos comunicamos con el Servidor */
    serialSIM800L.println("AT+HTTPINIT"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+HTTPPARA=\"CID\",1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/
    Serial.println("AT+HTTPPARA=\"URL\",\"http://colmenainteligente.ddns.net/abeja2/?apiario_id=" + String(apiario).substring(0,2) + "&colmena_id=" + String(colmena).substring(0,2) + "&temperatura=" + String(temperatura).substring(0,5) +"&humedad=" + String(humedad).substring(0,6) + "&fecha_hora=" + String(buf1).substring(0,20) + "\""); // Envío En comando AT  
    serialSIM800L.println("AT+HTTPPARA=\"URL\",\"http://colmenainteligente.ddns.net/abeja2/?apiario_id=" + String(apiario).substring(0,2) + "&colmena_id=" + String(colmena).substring(0,2) + "&temperatura=" + String(temperatura).substring(0,5) +"&humedad=" + String(humedad).substring(0,6) + "&fecha_hora=" + String(buf1).substring(0,20) + "\""); // Envío En comando AT  
    delay(3000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+HTTPACTION=0"); // Envío En comando AT
    delay(3000); // Espero un momento por la respuesta
    /*********/
    serialSIM800L.println("AT+HTTPREAD"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    /*********/ 
    serialSIM800L.println("AT+HTTPTERM"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
    
    /* Cerramos la comunicación */
    serialSIM800L.println("AT+SAPBR=0,1"); // Envío En comando AT
    delay(1000); // Espero un momento por la respuesta
}
