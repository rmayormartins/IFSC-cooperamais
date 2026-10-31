// FrioConecta: firmware v0 do nó de monitoramento de câmara fria
// IFSC Câmpus São José, projeto IFSC-cooperfamilia
//
// Status: v0, escrito a partir da estação IoT do PJI3 (2026/1). Ainda não testado
// em câmara fria. Serve de base para a Meta M3 (nó com rádio LoRa).
//
// O que esta versão faz:
//   - lê temperatura e umidade (DHT22) a cada 5 s e calcula a média móvel;
//   - lê o sensor de porta (reed switch) e mede há quanto tempo a porta está aberta;
//   - gera alarmes de temperatura fora da faixa (com tempo de persistência)
//     e de porta aberta por tempo demais;
//   - envia a telemetria em um único JSON para o ThingsBoard a cada 30 s;
//   - guarda as mensagens não enviadas em uma fila e reenvia quando a rede volta.
//
// Próximos passos (roadmap do projeto):
//   - M3: trocar o envio por Wi-Fi por rádio LoRa/LoRaWAN (AU915);
//   - M3: sensores de CO2 (SCD41), energia e pressões do ciclo;
//   - M4: leitura e ajuste de setpoint do controlador da câmara via Modbus RS-485.

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include "DHT.h"

// ═════════════ CONFIGURAÇÃO DE REDE ═════════════
const char* WIFI_SSID = "Nome da Rede";
const char* WIFI_PASSWORD = "Senha da Rede";
const char* TB_TOKEN = "SEU_TOKEN_DO_DISPOSITIVO";  // um único dispositivo no ThingsBoard
const char* TB_HOST = "https://thingsboard.cloud";

// ═════════════ PINOS ═════════════
#define DHTPIN 5          // mesmo pino da estação IoT
#define DHTTYPE DHT22
#define PINO_PORTA 25     // reed switch entre o pino e o GND (fechado = porta fechada)
#define PINO_ALARME 2     // LED da placa ou buzzer

// ═════════════ PARÂMETROS DA CÂMARA ═════════════
const float TEMP_MIN = 0.0;                // °C, faixa de resfriados
const float TEMP_MAX = 8.0;                // °C
const unsigned long PERSISTENCIA_MS = 120000;  // fora da faixa por 2 min gera alarme
const unsigned long PORTA_MAX_MS = 120000;     // porta aberta por mais de 2 min gera alarme

// ═════════════ TEMPOS ═════════════
const unsigned long INTERVALO_LEITURA = 5000;
const unsigned long INTERVALO_ENVIO = 30000;

// ═════════════ ESTADO ═════════════
DHT dht(DHTPIN, DHTTYPE);

const int N_MEDIA = 6;
float historico[N_MEDIA];
int idx_hist = 0;
int n_hist = 0;

float temp_media = NAN;
float umidade = NAN;
bool porta_aberta = false;
unsigned long inicio_porta = 0;
unsigned long inicio_fora_faixa = 0;
bool fora_faixa = false;
int codigo_alarme = 0;  // 0 normal, 1 temperatura, 2 porta, 3 ambos

unsigned long ultima_leitura = 0;
unsigned long ultimo_envio = 0;

// Fila simples de mensagens não enviadas
const int TAM_FILA = 20;
String fila[TAM_FILA];
int ini_fila = 0;
int n_fila = 0;

// ═════════════ REDE ═════════════
void conectar_wifi() {
  if (WiFi.status() == WL_CONNECTED) return;
  Serial.printf("Conectando WiFi: %s\n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 15000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.status() == WL_CONNECTED ? "\nWiFi conectado" : "\nSem WiFi, seguindo offline");
}

bool enviar_json(const String& json) {
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();  // v0: trocar por certificado na versão de campo
  HTTPClient http;
  String url = String(TB_HOST) + "/api/v1/" + TB_TOKEN + "/telemetry";

  if (!http.begin(client, url)) return false;
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000);
  int code = http.POST(json);
  http.end();

  Serial.printf("Envio HTTP %d: %s\n", code, json.c_str());
  return code >= 200 && code < 300;
}

void enfileirar(const String& json) {
  int pos = (ini_fila + n_fila) % TAM_FILA;
  fila[pos] = json;
  if (n_fila < TAM_FILA) {
    n_fila++;
  } else {
    ini_fila = (ini_fila + 1) % TAM_FILA;  // fila cheia: descarta a mais antiga
  }
}

void esvaziar_fila() {
  while (n_fila > 0) {
    if (!enviar_json(fila[ini_fila])) return;
    ini_fila = (ini_fila + 1) % TAM_FILA;
    n_fila--;
  }
}

// ═════════════ LEITURAS E ALARMES ═════════════
void ler_sensores() {
  float t = dht.readTemperature();
  float u = dht.readHumidity();

  if (!isnan(t)) {
    historico[idx_hist] = t;
    idx_hist = (idx_hist + 1) % N_MEDIA;
    if (n_hist < N_MEDIA) n_hist++;
    float soma = 0;
    for (int i = 0; i < n_hist; i++) soma += historico[i];
    temp_media = soma / n_hist;
  } else {
    Serial.println("Erro DHT22");
  }
  if (!isnan(u)) umidade = u;

  bool aberta = digitalRead(PINO_PORTA) == HIGH;  // com pull-up, aberta = HIGH
  if (aberta && !porta_aberta) inicio_porta = millis();
  porta_aberta = aberta;
}

void avaliar_alarmes() {
  unsigned long agora = millis();

  bool fora = !isnan(temp_media) && (temp_media < TEMP_MIN || temp_media > TEMP_MAX);
  if (fora && !fora_faixa) inicio_fora_faixa = agora;
  fora_faixa = fora;

  bool alarme_temp = fora_faixa && (agora - inicio_fora_faixa >= PERSISTENCIA_MS);
  bool alarme_porta = porta_aberta && (agora - inicio_porta >= PORTA_MAX_MS);

  codigo_alarme = (alarme_temp ? 1 : 0) + (alarme_porta ? 2 : 0);
  digitalWrite(PINO_ALARME, codigo_alarme ? HIGH : LOW);
}

String montar_json() {
  unsigned long tempo_porta_s = porta_aberta ? (millis() - inicio_porta) / 1000 : 0;
  String json = "{";
  json += "\"temperatura\":" + (isnan(temp_media) ? String("null") : String(temp_media, 2));
  json += ",\"umidade\":" + (isnan(umidade) ? String("null") : String(umidade, 1));
  json += ",\"porta_aberta\":" + String(porta_aberta ? "true" : "false");
  json += ",\"tempo_porta_s\":" + String(tempo_porta_s);
  json += ",\"alarme\":" + String(codigo_alarme);
  json += ",\"rssi_wifi\":" + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0);
  json += ",\"uptime_s\":" + String(millis() / 1000);
  json += "}";
  return json;
}

// ═════════════ SETUP E LOOP ═════════════
void setup() {
  Serial.begin(115200);
  Serial.println("=== FrioConecta: no v0 ===");

  dht.begin();
  pinMode(PINO_PORTA, INPUT_PULLUP);
  pinMode(PINO_ALARME, OUTPUT);
  digitalWrite(PINO_ALARME, LOW);

  conectar_wifi();
}

void loop() {
  unsigned long agora = millis();

  if (agora - ultima_leitura >= INTERVALO_LEITURA) {
    ultima_leitura = agora;
    ler_sensores();
    avaliar_alarmes();
  }

  if (agora - ultimo_envio >= INTERVALO_ENVIO) {
    ultimo_envio = agora;
    conectar_wifi();
    enfileirar(montar_json());
    esvaziar_fila();
    if (n_fila > 0) Serial.printf("Mensagens na fila: %d\n", n_fila);
  }
}
