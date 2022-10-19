/**
    Internet das Coisas
    Baseado em exemplo da IDE Arduino
    BasicHTTPClient.ino
    Created on: 24.05.2015
    Setembro de 2022
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>


#define REPETE 0xFFFFFFFF
#define BT_UM 0x20DF8877
#define BT_DOIS 0x20DF48B7
#define BT_TRES 0x20DFC837
#define BT_QUATRO 0x20DF28D7

#define API_RONALDO "http://iotron.pythonanywhere.com/envia"
#define API_CATARINA "http://catarinasenac.pythonanywhere.com/postJson"
#define API_ARNOTT_IOT "http://apiot.pythonanywhere.com/postJson" 
#define API_ARNOTT_GRAVAIOT "http://gravaiot.pythonanywhere.com/postJson" 

#include <WiFiClient.h>
#include <IRremote.h>                       // Biblioteca IRemote

#define HEADER_KEY "eFgHjukoli12Reatyghmaly76"
#define RECV_PIN 12

/* Variaveis Globais */

ESP8266WiFiMulti WiFiMulti;
WiFiClient client;
HTTPClient http;
DynamicJsonDocument doc(1024);
IRrecv irrecv(RECV_PIN);                    // criando a instância
decode_results results;
int val_sens=0;

char ssid[] = "wifi";
char senha[]="*******";

// setup da placa
void setup() {
  Serial.begin(115200);
  irrecv.enableIRIn();                      // Inicializa a recepção de códigos

  Serial.println();
  Serial.println();
  Serial.println("Conectando Wifi");
  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP( ssid , senha );
  Serial.print("Conectado. SSID:");
  Serial.println( ssid );
}

// repeticao - envia dados se sensor identificar leitura - IR
void loop() {
  if (leSensorIr() != 0){
    envia( API_ARNOTT_GRAVAIOT );
  }
}

// funcao para enviar dados
int envia( char *api ){
  static char dados[1024];
  static char sensor [10], valor[12];
  static String payload;
  
  // wait for WiFi connection
  if (WiFiMulti.run() == WL_CONNECTED) {
    Serial.print("[HTTP] begin...\n");
    Serial.println( api );
    if ( http.begin( client, api)) {
      Serial.print("[HTTP] POST...\n");
      // start connection and send HTTP header
      http.addHeader("Content-Type","application/json");
      http.addHeader("Authorization-Token", HEADER_KEY );

      sprintf( sensor, "%02d", 1 );
      sprintf( valor, "%08X", results.value );

      doc["sensor"] = sensor;
      doc["valor"] = valor;
  
      serializeJson(doc, dados);
      Serial.println( dados );
      int httpCode = http.POST( dados );
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] POST... code: %d\n", httpCode);
        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          payload = http.getString();
          Serial.println(payload);
        }
      } else 
          Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      http.end();
    } else 
        Serial.printf("[HTTP} Unable to connect\n");
  }
  return 0;
}

// funcao oara ler sensor InfraVermelho - controle remoto
int leSensorIr(){
  if (irrecv.decode(&results))              // se algum código for recebido
  {
    if ( results.value != REPETE ){
      Serial.println(results.value, HEX);
      irrecv.resume();
      delay(10);
      return(1);
    }
  }
  irrecv.resume();
  delay(10);
  return(0);
}
