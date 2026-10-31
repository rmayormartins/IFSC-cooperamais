# Componentes

## Estação IoT base (em uso)

| Componente | Função | Quantidade |
| --- | --- | --- |
| ESP32 DevKit | Microcontrolador com Wi-Fi | 1 |
| DHT22 | Temperatura e umidade | 1 |
| BMP280 | Temperatura e pressão | 1 |
| Sensor de chuva | Detecção de precipitação | 1 |
| Resistor 4,7 kΩ | Pull-up do DHT22 | 1 |
| Protoboard, jumpers e fonte USB | Montagem | 1 conjunto |

## Nó FrioConecta v0 (a montar)

| Componente | Função | Pino |
| --- | --- | --- |
| ESP32 DevKit | Microcontrolador | |
| DHT22 | Temperatura e umidade | GPIO 5 |
| Reed switch e ímã | Sensor de porta | GPIO 25 (pull-up interno) |
| LED ou buzzer | Alarme local | GPIO 2 |

## Previstos no projeto

| Componente | Função | Meta |
| --- | --- | --- |
| ESP32 com rádio LoRa na mesma placa (SX1262) | Nó com LoRa | M3 |
| Gateway LoRaWAN AU915 | Rede no câmpus e no piloto | M3 |
| Sensirion SCD41 ou sensor NDIR equivalente | CO2 | M3 |
| Sensirion SHT45 | Umidade de referência | M3 |
| DS18B20 e PT100 com MAX31865 | Temperatura em vários pontos e referência | M3 |
| Medidor de energia (PZEM-004T ou equivalente) | Consumo do compressor | M3 |
| Transdutores de pressão (sucção e descarga) | Diagnóstico do ciclo de refrigeração | M5 |
| Acelerômetro (ADXL345 ou MPU6050) | Vibração do compressor | M6 |
| Conversor RS-485 (MAX485) | Modbus com o controlador da câmara | M4 |

Antes da compra, conferir no datasheet a faixa de temperatura de operação de cada sensor.
