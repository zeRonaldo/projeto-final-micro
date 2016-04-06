
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>


ESP8266WiFiMulti WiFiMulti;

WebSocketsServer webSocket = WebSocketsServer(81);
long lastTimeRefresh = 0;
float Vin=5.0;     // [V]        
float Raux=10000;  // [ohm]      Resistor auxiliar
float R0=10000;    // [ohm]      valor nominal do RTC a 25ºC

float T0=298.15;   // [K] (25ºC)

float Vout=0.0;    // [V]        Tensão de saída do divisor
float Rout=0.0;    // [ohm]      Valor de resistência do NTC

//Ver Datasheet ou usar um multimetro e medir a resistencia do NTC nas duas temperaturas
float T1=273;      // [K]        Temperatura de calibração 1
float T2=373;      // [K]        Temperatura de calibração 2
float RT1=35563;   // [ohms]     Resistência medida na temperatura de calibração 1
float RT2=549;    // [ohms]     Resistência medida na temperatura de calibração 2
float TempK=0.0;   // [K]        Temperatura de saída em Kelvin
float TempC=0.0;   // [ºC]       Temperatura de saída em °C


double temp()
{
  double val=analogRead(0);
  Serial.println(val);
  double fenya=(val/1023)*5;
  // Ohm Law r/100=fenya/(3.3-fenya)
  double r=fenya/(3.3-fenya)*100;
  double res =( 1/(  log(r/10) /3000 + 1/298.15   )-273.15);
  Serial.println(res);
  return res;

}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println(" Disconnected!");
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        
        // send message to client
        webSocket.sendTXT(num, "ok");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            long lastTimeRefresh = millis();
            delay(50);
            String text = String((char *) &payload[0]);
            if (text == "PING") {
            
            webSocket.sendTXT(num, "PONG");
            
            }else if(text == "okpct"){
              double te = temp();
              String msg = "newpct";
              
              msg+= te;
              msg+= ":";
              msg+= lastTimeRefresh;
              Serial.println(te);
              
              webSocket.sendTXT(num, msg);
             }

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
    }

}


void setup() {
        // Serial..begin(921600);
    Serial.begin(115200);
    pinMode(0, INPUT);
    //Serial.setDebugOutput(true);
    Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

    WiFiMulti.addAP("dulceCasa", "00456Lol");

    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(100);
        Serial.println("Ainda nada");
    }
    Serial.println("Conectado");

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
}

void loop() {
    webSocket.loop();
}

