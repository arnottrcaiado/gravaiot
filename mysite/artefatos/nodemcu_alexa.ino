#include <ESP8266WiFi.h>
#include <Espalexa.h>

#define ssid  "****"
#define password "***"

#define dev1Nome  "tomada"
#define dev2Nome  "ar"
#define dev3Nome  "luz"

#define D1 5    // porta de saida para ligar primeiro dispositivo - tomada
#define D2 4    // porta de saida para ligar segundo dispotitivo - ventilador / ar
#define D3 0    // porta de saida para ligar terceiro dispositivo - luz - LED

Espalexa espalexa;

void funcaoLigaTomada( uint8_t param);
void funcaoLigaAr( uint8_t param);
void funcaoLigaLuz( uint8_t param);
void setup() {
  Serial.begin( 115200 );
  WiFi.begin( ssid, password );
  Serial.println( "Conectando");
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print("." );
    delay(100);
  }
  Serial.println("Conectado");

  espalexa.addDevice( dev1Nome, funcaoLigaTomada);
  espalexa.addDevice( dev2Nome, funcaoLigaAr);
  espalexa.addDevice( dev3Nome, funcaoLigaLuz);

  pinMode( D1, OUTPUT );
  pinMode( D2, OUTPUT );
  pinMode( D3, OUTPUT );
  espalexa.begin();
}
void loop() {
  espalexa.loop();
}

// funcao construida para ligar dispositivo TOMADA
void funcaoLigaTomada( uint8_t param ){
  Serial.println( param );
  if ( param == 0 ) {
    digitalWrite ( D1, LOW );
    Serial.println("Tomada: desligada");
  }
  else {
    digitalWrite ( D1, HIGH );
    Serial.println("Tomada: Ligada");
  }
}

// funcao construida paar ligar dispositivo AR
void funcaoLigaAr( uint8_t param ){

  Serial.println( param );
  if ( param == 0 ) {
    digitalWrite ( D2, LOW );
    Serial.println("Ar: desligado");
  }
  else {
    digitalWrite ( D2, HIGH );
    Serial.println("Ar: Ligado");
  }
}


// funcao construida para ligar dispositivo LUZ
void funcaoLigaLuz( uint8_t param ){

  Serial.println( param );
  if ( param == 0 ) {
    digitalWrite ( D3, LOW );
    Serial.println("Luz: desligada");
  }
  else {
    digitalWrite ( D3, HIGH );
    Serial.println("Luz: Ligada");

  }
}