# SmartBioPH

Plataforma portátil, miniaturizada e de baixo custo para determinação
colorimétrica de pH usando o sensor multicanal **AS7341** e o indicador
ácido-base **azul de bromotimol (BTB)**, com leitura em **cubeta** (solução)
e suporte a impressão 3D das peças usadas no protótipo.

Este repositório acompanha o artigo:

> *Application of AS7341 sensor in portable, miniaturized and low-cost
> systems for colorimetric pH determination on paper and solution in
> saliva samples* — A. G. Xavier, C. C. S. Machado, Y. S. N. da Mota,
> J. F. S. Petruci, S. G. da Silva. Instituto de Química, Universidade
> Federal de Uberlândia.

## Estrutura do repositório

```
SmartBioPH/
├── firmware/              firmware PlatformIO (Arduino Uno / ESP32-S3)
│   ├── platformio.ini      ambientes de build + dependências (lib_deps)
│   └── src/
│       ├── main.cpp         setup/loop, leitura da serial
│       ├── core/
│       │   ├── Utils.h        helpers PROGMEM (sem heap/String)
│       │   ├── SensorManager.h  fábrica do sensor ativo (fixo: AS7341)
│       │   ├── Measurement.h/.cpp  amostragem e média dos 8 canais
│       │   └── Protocol.h/.cpp    protocolo JSON via Serial
│       └── sensors/
│           ├── SensorBase.h     interface abstrata de sensor
│           └── AS7341.h/.cpp    driver do AS7341 (modo reflectância)
├── web/                    interface web (Web Serial API)
│   ├── index.html
│   ├── script.js
│   └── style.css
└── hardware/               peças para impressão 3D
    ├── SmartpH_Cuvette.stl        cubeta de baixo volume (40 mm)
    ├── SmartpH_Holder.stl         suporte do sensor AS7341
    ├── Arduino_UNO_Smart_Box.3mf  case do Arduino Uno (projeto completo)
    └── Arduino_UNO_Smart_Box_part_2.stl  segunda peça do case
```

## Arquitetura do firmware

O firmware segue uma separação simples em três camadas:

1. **`sensors/`** — implementa `SensorBase`, a interface mínima que
   qualquer sensor precisa expor (leitura dos canais, ganho, LED). Hoje só
   existe o driver do `AS7341Sensor`, operando **somente em modo
   reflectância** (LED interno ligado, sem os modos absorbância/
   fluorescência nem LEDs externos).
2. **`core/`** — lógica independente de sensor:
   - `SensorManager` devolve a instância ativa do sensor (fixa no AS7341,
     sem seleção por build flag);
   - `Measurement` faz N leituras e tira a média dos 8 canais;
   - `Protocol` é o único arquivo que conhece JSON — trata os comandos
     vindos da serial e monta as respostas.
   - `Utils` traz helpers para strings em `PROGMEM`, evitando o uso da
     classe `String` (fragmentação de heap em placas AVR).
3. **`main.cpp`** — inicializa Serial/I2C/sensor e faz o parsing linha a
   linha da entrada serial, repassando cada linha completa para
   `Protocol::handleCommand`.

A interface web (`web/`) fala com a placa via **Web Serial API**
(Chrome/Edge), usando o protocolo JSON descrito abaixo — não depende do
Arduino IDE nem de nenhum backend.

### Protocolo (JSON por linha, via Serial)

**Browser → Placa**
```json
{"cmd":"get_info"}
{"cmd":"set_gain","idx":4}
{"cmd":"set_led","current":60}
{"cmd":"set_samples","n":1}
{"cmd":"measure"}
```

**Placa → Browser**
```json
{"evt":"info","sensor":"AS7341","channels":[...8...],"gain":{"options":[...],"default":4},"led":{"minMA":4,"maxMA":258,"default":60}}
{"evt":"ack","cmd":"..."}
{"evt":"progress","n":1,"data":{"0":123.4,...}}
{"evt":"result","n":1,"gain":"16X","led_ma":60,"data":{"0":123.4,...}}
{"evt":"error","msg":"..."}
```

## Hardware

- Microcontrolador: **Arduino Uno** (testado, ~91% flash / ~40% RAM) ou
  **ESP32-S3** (USB nativo).
- Sensor: **AS7341** (Adafruit breakout), 8 canais espectrais (415–680 nm).
- Cubeta acrílica de baixo volume (40 mm de altura, 900 µL) — arquivo
  `hardware/SmartpH_Cuvette.stl`.
- Suporte impresso em 3D para fixar a geometria sensor/cubeta —
  `hardware/SmartpH_Holder.stl`.
- Case para o Arduino Uno — `hardware/Arduino_UNO_Smart_Box.3mf`
  (projeto completo, editável) e a peça complementar
  `Arduino_UNO_Smart_Box_part_2.stl`.

Os `.stl` podem ser abertos em qualquer fatiador (Cura, PrusaSlicer etc.);
o `.3mf` preserva o projeto completo (múltiplas peças/posicionamento) e
abre diretamente no seu fatiador ou no CanvasWorkspace/software de origem.

## Como instalar o PlatformIO

O firmware usa **PlatformIO**, que baixa e resolve automaticamente todas
as bibliotecas (`Adafruit_AS7341`, `Adafruit_BusIO`, `ArduinoJson`)
listadas em `firmware/platformio.ini` — não é preciso instalar nada à mão.

**Opção recomendada — VS Code:**
1. Instale o [VS Code](https://code.visualstudio.com/).
2. Instale a extensão **PlatformIO IDE** (aba de extensões, busque
   "PlatformIO IDE").
3. Abra a pasta `firmware/` deste repositório em *File → Open Folder*.
   O PlatformIO detecta o `platformio.ini` e baixa as dependências
   automaticamente na primeira vez que você compilar.

**Opção via linha de comando (CLI):**
```bash
# instala o PlatformIO Core (precisa de Python 3)
pip install -U platformio

# dentro da pasta firmware/
cd firmware
pio run                      # compila os dois ambientes e já baixa as libs
pio run -e uno -t upload     # compila e grava no Arduino Uno
pio run -e as7341_esp32s3 -t upload   # compila e grava no ESP32-S3
```

Não é necessário nenhum passo manual de instalação de bibliotecas: o
PlatformIO lê `lib_deps` em `platformio.ini` e resolve tudo sozinho, tanto
pelo VS Code quanto pela CLI.

## Como usar

1. Conecte o AS7341 à placa via I2C (SDA/SCL) e ligue a placa ao PC por USB.
2. Grave o firmware (`pio run -e uno -t upload` ou `-e as7341_esp32s3`).
3. Abra `web/index.html` em **Chrome ou Edge** (a Web Serial API não
   funciona em Firefox/Safari).
4. Clique em **Connect via USB**, selecione a porta da placa.
5. Ajuste Gain, LED current e Number of samples, clique em **Send setup**.
6. Clique em **Measure** para disparar uma leitura e ver o resultado dos
   8 canais espectrais.

## Licença

Este projeto está sob a licença MIT — veja [LICENSE](LICENSE). Se você
preferir outra licença (ex.: GPL, Apache-2.0) para o repositório do
artigo, é só trocar o arquivo.

## Citação

Se este código ou as peças forem úteis no seu trabalho, cite o artigo
correspondente (dados completos serão atualizados após publicação).
