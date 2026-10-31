# Nó FrioConecta v0

Primeira versão do firmware do nó de monitoramento da câmara fria, escrita a partir da estação IoT base. **Ainda não testada em câmara fria.** É a base da Meta M3, em que o envio por Wi-Fi será trocado por rádio LoRa.

## O que faz

- Lê temperatura e umidade (DHT22) a cada 5 s e calcula a média móvel das últimas 6 leituras.
- Lê o sensor de porta e mede há quanto tempo ela está aberta.
- Alarme 1: temperatura fora da faixa por mais de 2 minutos. Alarme 2: porta aberta por mais de 2 minutos. Alarme 3: os dois.
- Envia a cada 30 s um JSON único ao ThingsBoard: `temperatura`, `umidade`, `porta_aberta`, `tempo_porta_s`, `alarme`, `rssi_wifi` e `uptime_s`.
- Guarda até 20 mensagens não enviadas e reenvia quando a rede volta.

## Ligações

| Componente | Pino do ESP32 |
| --- | --- |
| DHT22 (dados, com pull-up de 4,7 kΩ) | GPIO 5 |
| Reed switch (outro terminal no GND) | GPIO 25 |
| LED ou buzzer de alarme | GPIO 2 |

## Configuração

No início do arquivo: rede Wi-Fi, token do dispositivo no ThingsBoard, faixa de temperatura (`TEMP_MIN`, `TEMP_MAX`) e tempos de alarme.

O DHT22 opera de -40 °C a 80 °C, o que atende câmaras de resfriados. Para câmaras de congelados e medições de referência, o projeto prevê sensores DS18B20 e PT100.
