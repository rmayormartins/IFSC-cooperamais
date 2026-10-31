// Estação Meteorológica IoT (ESP32 + DHT22 + BMP280 + sensor de chuva)
// Projeto Integrador 3 (PJI3), Engenharia de Telecomunicações, IFSC Câmpus São José, 2026/1
// Autores: Beatriz Papst Abreu, Gian Gabriel Fagundes Santos, Jean Motta Caitano dos Reis,
//          Joana da Silva e Wagner Flores dos Santos
// Código transcrito do relatório técnico de 23/06/2026. Os tokens do ThingsBoard foram
// substituídos por marcadores: preencha com os tokens dos seus dispositivos.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "DHT.h"

// ═════════════ WIFI ═════════════
const char* WIFI_SSID = "Nome da Rede";
const char* WIFI_PASSWORD = "Senha da Rede";

// ═════════════ TOKENS THINGSBOARD ═════════════
const char* TOKEN_TEMP = "SEU_TOKEN_TEMPERATURA";
const char* TOKEN_PRESSAO = "SEU_TOKEN_PRESSAO";
const char* TOKEN_UMIDADE = "SEU_TOKEN_UMIDADE";
const char* TOKEN_CHUVA = "SEU_TOKEN_CHUVA";

// ═════════════ BMP280 ═════════════
#define BMP280_I2C_ADDRESS 0x76
Adafruit_BMP280 bmp;

// ═════════════ DHT22 ═════════════
#define DHTPIN 5
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// ═════════════ SENSOR CHUVA ═════════════
#define PINO_ANALOGICO 34
#define PINO_DIGITAL 25

// ═════════════ CONTROLE ═════════════
#define INTERVALO_ENVIO 30000

unsigned long ultimo_envio = 0;

// ═════════════ WIFI ═════════════
void conectar_wifi() {
  Serial.printf("Conectando WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi conectado!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}

// ═════════════ ENVIO HTTPS ═════════════
void enviar_dado(String token, String json) {
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;

  String url = "https://thingsboard.cloud/api/v1/" + token + "/telemetry";

  Serial.println("\nEnviando:");
  Serial.println(url);
  Serial.println(json);

  http.useHTTP10(true);
  http.setTimeout(10000);

  if (http.begin(client, url)) {
    http.addHeader("Content-Type", "application/json");

    int code = http.POST(json);

    Serial.print("HTTP Code: ");
    Serial.println(code);

    if (code > 0) {
      String resposta = http.getString();
      Serial.println("Resposta:");
      Serial.println(resposta);
    } else {
      Serial.print("Erro HTTP: ");
      Serial.println(http.errorToString(code));
    }

    http.end();
  } else {
    Serial.println("Falha begin HTTPS");
  }
}

// ═════════════ SETUP ═════════════
void setup() {
  Serial.begin(115200);
  Serial.println("=== Estação IoT ===");

  // I2C
  Wire.begin(21, 22);

  // BMP280
  if (!bmp.begin(BMP280_I2C_ADDRESS)) {
    Serial.println("Erro BMP280!");
    while (1);
  }

  // DHT22
  dht.begin();

  // Sensor chuva
  pinMode(PINO_DIGITAL, INPUT);

  // WiFi
  conectar_wifi();
}

// ═════════════ LOOP ═════════════
void loop() {
  // Reconecta WiFi
  if (WiFi.status() != WL_CONNECTED) {
    conectar_wifi();
  }

  unsigned long agora = millis();

  if (agora - ultimo_envio >= INTERVALO_ENVIO) {

    // ===== BMP280 =====
    float temp_bmp = bmp.readTemperature();
    float pressao = bmp.readPressure() / 100.0;

    // ===== DHT22 =====
    float umidade = dht.readHumidity();
    float temp_dht = dht.readTemperature();

    // ===== SENSOR CHUVA =====
    int chuva_analogico = analogRead(PINO_ANALOGICO);
    int chuva_digital = digitalRead(PINO_DIGITAL);
    bool status_chuva = (chuva_digital == 0);

    // ===== VALIDAÇÃO =====
    if (isnan(umidade) || isnan(temp_dht)) {
      Serial.println("Erro DHT22!");
      umidade = -1;
      temp_dht = -1;
    }

    // ===== JSON =====
    String json_temp = "{\"temperature\":" + String(temp_bmp, 2) + "}";
    String json_pressao = "{\"Pressao\":" + String(pressao, 2) + "}";
    String json_umidade = "{\"Umidade\":" + String(umidade, 2) + "}";
    String json_chuva = "{\"QuantidadeDeChuva\":" +
                        String(status_chuva ? "Verdadeiro" : "Falso") + "}";

    // ===== ENVIO =====
    enviar_dado(TOKEN_TEMP, json_temp);
    delay(1000);
    enviar_dado(TOKEN_PRESSAO, json_pressao);
    delay(1000);
    enviar_dado(TOKEN_UMIDADE, json_umidade);
    delay(1000);
    enviar_dado(TOKEN_CHUVA, json_chuva);

    // ===== SERIAL =====
    Serial.println("\n===== DADOS =====");
    Serial.print("Temp BMP: ");
    Serial.println(temp_bmp);
    Serial.print("Pressao: ");
    Serial.println(pressao);
    Serial.print("Umidade: ");
    Serial.println(umidade);
    Serial.print("Chuva: ");
    Serial.println(status_chuva ? "chovendo" : "seco");
    Serial.print("Analogico: ");
    Serial.println(chuva_analogico);

    ultimo_envio = agora;
  }
}
