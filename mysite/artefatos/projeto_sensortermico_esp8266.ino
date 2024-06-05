#include <ESP8266HTTPClient.h>
#include <ESP8266WiFiMulti.h>
#include <Wire.h>
#include <SSD1306Wire.h>
#include <Adafruit_AMG88xx.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include <fauxmoESP.h>
#include <math.h>


#define PIN D3       // Pino de dados para a barra de LEDs
#define NUMPIXELS 8  // Número de LEDs na barra

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Pinos do NodeMCU para o display
#define OLED_SDA D5
#define OLED_SCL D6

// Pinos do NodeMCU para o sensor térmico
#define SENSOR_SDA D2
#define SENSOR_SCL D1

#define TEMPLIMITE 40

#define APIENDPOINT "http://gravaiot.pythonanywhere.com/api/temperaturas"
#define APIKEY "01234"

WiFiClient client;
HTTPClient http;

#define NWIFIS 5

struct redes {
  const char *ssid;
  const char *senha;
};

char *redeAtual;
int respostaHttp = 0;
bool conectou = false;
float tempAlerta = 0;

struct redes redeWifi[NWIFIS] = {
  {"AP1501", "ARBBBE11"},
  {"iPhone de Arnott", "arbbbe11"},
  {"SENAC-Mesh", "09080706"},
  {"Vivo-Internet-E532", "6EFC366C"},
  {"SENAC-MESH", "09080706"}
};

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

// Inicializa o display OLED
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

// Inicializa o sensor térmico
Adafruit_AMG88xx amg;

// Configuração do servidor web
ESP8266WebServer server(80);

/*
Configuração na Alexa:
Adicionar o dispositivo Fauxmo na Alexa:

Abra o aplicativo Alexa no seu smartphone.
Vá em Dispositivos -> Adicionar Dispositivo -> Outro.
Selecione "Procurar dispositivos".
A Alexa deve detectar automaticamente o dispositivo "sensor aviso".
Criar uma Rotina:

No aplicativo Alexa, vá em Mais -> Rotinas -> Adicionar Rotina.
Dê um nome à sua rotina (por exemplo, "Aviso Sensor").
Em Quando isso acontecer, selecione Dispositivo e escolha o dispositivo "sensor aviso".
Em Adicionar ação, selecione Avisar e escreva a mensagem que deseja que a Alexa diga (por exemplo, "Temperatura alta detectada!").
*/
// Inicializa a FauxmoESP
fauxmoESP fauxmo;


unsigned long previousMillis;
unsigned long currentMillis;

const long interval = 10000;  // Intervalo de 1 minuto em milissegundos

void setup() {
  Serial.begin(115200);
  strip.begin();
  strip.show();  // Inicializa todos os LEDs apagados

  // Inicializa a comunicação I2C para o display OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.display();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(64, 32, "Start...");
  display.display();
  delay(2000);

  // Inicializa a comunicação I2C para o sensor térmico AMG8831
  Wire.begin(SENSOR_SDA, SENSOR_SCL);
  if (!amg.begin()) {
    Serial.println("Falha - AMG88xx sensor!");
    display.clear();
    display.drawString(64, 32, "Sensor AMG88xx Falhou!");
    display.display();
    while (1);
  }

  Serial.println("Sensor initialized");
  display.clear();
  display.drawString(64, 32, "Sensores Ok");
  display.display();
  delay(2000);

  // Conecta à rede Wi-Fi
  display.clear();
  display.drawString(64, 32, "Conectando WiFi...");
  display.display();

  do {
    conectou = conectaWifi();  // tenta conectar em uma das redes disponíveis
    if (!conectou)
      delay(100);

  } while (!conectou);

  Serial.println("");
  Serial.println("WiFi connected");

  display.clear();
  display.drawString(64, 32, "WiFi ok");
  display.display();
  delay(3000);
  display.clear();
  display.drawString(64, 32, WiFi.localIP().toString());
  display.display();
  previousMillis = millis();


  // Configura o dispositivo Fauxmo
  fauxmo.addDevice("Aviso sensor");

  // Configura o callback para o dispositivo Fauxmo
  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
    if (state) {
      Serial.println("Aviso da Alexa: Medida crítica atingida!");
    }
  });

  // Inicializa FauxmoESP
  fauxmo.enable(true);

  delay(5000);
}

void loop() {
  float pixels[64];
  Wire.begin(SENSOR_SDA, SENSOR_SCL);
  amg.readPixels(pixels);
  displayData(pixels);

  // Atualiza FauxmoESP
  fauxmo.handle();

  currentMillis = millis();
  if ((currentMillis - previousMillis) >= interval) {
    Serial.println("Envio de Dados p/ API");
    previousMillis = currentMillis;
    enviarDadosParaAPI(pixels, APIENDPOINT, APIKEY);

    if (tempAlerta > TEMPLIMITE) {  // Ajuste o valor crítico conforme necessário
    // Dispara a função da Alexa
    fauxmo.setState("Aviso sensor", true, 255);
    }

  }
}

void displayData(float pixels[64]) {
  // Calcula a média, desvio padrão, valor máximo e valor mínimo
  float sum = 0.0;
  float sumOfSquares = 0.0;
  float maxVal = pixels[0];
  float minVal = pixels[0];

  for (int i = 0; i < 64; i++) {
    sum += pixels[i];
    sumOfSquares += pixels[i] * pixels[i];
    if (pixels[i] > maxVal) maxVal = pixels[i];
    if (pixels[i] < minVal) minVal = pixels[i];
  }

  float mean = sum / 64;
  float variance = (sumOfSquares / 64) - (mean * mean);
  float stdDev = sqrt(variance);

  Serial.print("Media: ");
  Serial.println(mean);
  Serial.print("Max: ");
  Serial.println(maxVal);
  Serial.print("Min: ");
  Serial.println(minVal);

  // Exibe a média, desvio padrão, valor máximo, valor mínimo no display
  Wire.begin(OLED_SDA, OLED_SCL);
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "" + String(mean, 0) + "C d:" + String(stdDev, 1) + "C M:" + String(maxVal, 0) + "C m:" + String(minVal, 0) + "C");
  display.drawString(0, 20, "Temperatura: " + String(mean, 0));
  display.drawString(0, 30, "Maximo: " + String(maxVal, 0));
  display.display();
  mostrarTemperatura(maxVal);
}

// Função para mostrar a temperatura na barra de LEDs RGB
void mostrarTemperatura(float temp) {
  uint32_t color;

  tempAlerta = temp;

  if (temp < 10) {
    // Tons de branco
    int brightness = map(temp, 0, 10, 64, 50);  // Metade do brilho
    color = strip.Color(brightness, brightness, brightness);
  } else if (temp >= 10 && temp < 15) {
    // Azul
    int blue = map(temp, 10, 15, 50, 64);  // Metade do brilho
    color = strip.Color(0, 0, blue);
  } else if (temp >= 15 && temp < 22) {
    // Verde
    int green = map(temp, 15, 22, 50, 64);  // Metade do brilho
    color = strip.Color(0, green, 0);
  } else if (temp >= 22 && temp < 26) {
    int green = map(temp, 22, 26, 64, 64);  // Metade do brilho
    int red = map(temp, 22, 26, 50, 25);    // Metade do brilho
    color = strip.Color(red, green, 0);
  } else if (temp >= 26 && temp < 35) {
    int red = map(temp, 26, 35, 64, 64);    // Metade do brilho
    int green = map(temp, 26, 35, 41, 25);  // Metade do brilho
    color = strip.Color(red, green, 0);
  } else if (temp >= 35 && temp <= 50) {
    int red = map(temp, 35, 50, 64, 64);   // Metade do brilho
    int green = map(temp, 35, 50, 25, 0);  // Metade do brilho
    color = strip.Color(red, green, 0);
  } else {
    // Vermelho mais forte
    color = strip.Color(64, 0, 0);  // Metade do brilho
  }

  // Acende todos os LEDs com a cor calculada
  for (int i = 0; i < NUMPIXELS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

// Função para enviar os dados para a API
void enviarDadosParaAPI(float *pixels, const char *apiEndpoint, const char *apiKey) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(client, apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-api-key", apiKey);  // Adiciona a chave da API no header

    StaticJsonDocument<512> jsonDoc;
    JsonArray dataArray = jsonDoc.createNestedArray("valor");

    for (int i = 0; i < 64; i++) {
      dataArray.add((int) round(pixels[i]));
    }

    String jsonString;
    serializeJson(jsonDoc, jsonString);

    int httpResponseCode = http.POST(jsonString);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println(httpResponseCode);
      Serial.println(response);
    } else {
      Serial.println("Erro - envio API");
    }

    http.end();
  } else {
    Serial.println("WiFi desconectado");
  }
}


//-------------------------------------------------------------------------
bool conectaWifi() {
  bool conectou = false;
  int i = 0;
  int qtd = 0;

  while (i < NWIFIS) {  // tenta cada uma das redes disponíveis
    Serial.println("Conectando: wifi");
    Serial.println(redeWifi[i].ssid);
    WiFi.begin(redeWifi[i].ssid, redeWifi[i].senha);

    qtd = 0;

    while (WiFi.status() != WL_CONNECTED) {
      delay(400);
      Serial.print(".");
      qtd++;
      if (qtd > 30) {
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Conectado com sucesso. IP: ");
      Serial.println(WiFi.localIP());
      Serial.println(redeWifi[i].ssid);
      delay(2000);
      conectou = true;
      redeAtual = (char *)redeWifi[i].ssid;

      return conectou;
    }
    i++;
  }
  return conectou;
}
