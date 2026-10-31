# Estação IoT base

Código da estação meteorológica IoT desenvolvida no Projeto Integrador 3 (PJI3) da Engenharia de Telecomunicações do IFSC Câmpus São José, em 2026/1. É o ponto de partida (TRL 3) do FrioConecta.

- Microcontrolador: ESP32
- Sensores: DHT22 (temperatura e umidade, GPIO 5), BMP280 (temperatura e pressão, I2C nos GPIO 21 e 22) e sensor de chuva (GPIO 34 analógico e GPIO 25 digital)
- Comunicação: Wi-Fi, HTTPS, envio a cada 30 s para o ThingsBoard

O código foi transcrito do relatório técnico de 23/06/2026 (em `docs/`). Os tokens dos dispositivos do ThingsBoard foram trocados por marcadores; preencha com os seus antes de gravar no ESP32.

Bibliotecas (Arduino IDE): Adafruit BMP280 Library, Adafruit Unified Sensor e DHT sensor library.

Autores: Beatriz Papst Abreu, Gian Gabriel Fagundes Santos, Jean Motta Caitano dos Reis, Joana da Silva e Wagner Flores dos Santos.
