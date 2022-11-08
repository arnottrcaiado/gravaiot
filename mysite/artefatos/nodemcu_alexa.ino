#include <ESP8266WiFi.h>
#include <Espalexa.h>

#define ssid  "****"
#define password "***"

#define dev1Nome  "tomada"
#define dev2Nome  "ar"
#define dev3Nome  "luz"

#define D1 5    // porta de saida para ligar primeiro dispositivo - fonte
#define D2 4    // porta de saida para ligar segundo dispotitivo - ventilador
#define D3 0    // porta de saida para ligar terceiro dispositivo - luzes LED

Espalexa espalexa;

void funcaoLigafonte( uint8_t param);
void funcaoLigavento( uint8_t param);
void funcaoLigaled( uint8_t param);
void setup() {
  Serial.begin( 115200 );
  WiFi.begin( ssid, password );
  Serial.println( "Conectando");
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print("." );
    delay(100);
  }
  Serial.println("Conectado");
  
  espalexa.addDevice( dev1Nome, funcaoLigafonte);
  espalexa.addDevice( dev2Nome, funcaoLigavento);
  espalexa.addDevice( dev3Nome, funcaoLigaled);

  pinMode( D1, OUTPUT );
  pinMode( D2, OUTPUT );
  pinMode( D3, OUTPUT );
  espalexa.begin();
}
void loop() {
  espalexa.loop();
}

// funcao para ligar peimriro dispositivo - Fonte 
void funcaoLigafonte( uint8_t param ){
  Serial.println( param );
  if ( param == 0 ) {
    digitalWrite ( D1, LOW );
  }
  else {
    digitalWrite ( D1, HIGH );
  }
}

// funcao para ligar segundo dispositivo - ventilador
void funcaoLigavento( uint8_t param ){

  Serial.println( param );
  if ( param == 0 ) {
    digitalWrite ( D2, LOW );
  }
  else {
    digitalWrite ( D2, HIGH );
  }
}

// funcao para ligar terceiro dispositivo - luz leds
void funcaoLigaled( uint8_t param ){

  Serial.println( param );
  if ( param == 0 ) {
    digitalWrite ( D3, LOW );
  }
  else {
    digitalWrite ( D3, HIGH );
  }
}