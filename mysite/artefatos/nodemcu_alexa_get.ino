/**
    IOT
    Exemplo base para requisitcao http GET
    out-2022
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

#define HEADER_KEY "eFgHjukoli12Reatyghmaly76"

DynamicJsonDocument doc(1024);
ESP8266WiFiMulti WiFiMulti;

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println();
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("iPhone de Arnott", "arbbbe11");
}

void loop() {
  static char valor[]="ON";
  char dados[255];

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {
    WiFiClient client;
    HTTPClient http;
    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://gravaiot.pythonanywhere.com/statusmodeloum")){
      Serial.print("[HTTP] GET...\n");
      // start connection and send HTTP header
      http.addHeader("Content-Type","application/json");
      http.addHeader("Authorization-Token", HEADER_KEY );

 //     doc["valor"] = valor;
  
 //     serializeJson(doc, dados);
      
 //     int httpCode = http.POST( dados);
      int httpCode = http.GET( );

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else 
          Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      http.end();
    } else 
        Serial.printf("[HTTP} Unable to connect\n");
  }
  delay(100);
}