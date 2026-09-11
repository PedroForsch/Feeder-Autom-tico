# Alimentador Automático para Pets

Projeto acadêmico desenvolvido em equipe com o objetivo de automatizar
a alimentação de animais de estimação utilizando um ESP32.

O sistema combina automação, controle de hardware e uma interface web
para permitir a liberação programada ou manual de ração, além do
monitoramento do nível de ração no reservatório.

## Sobre o projeto

A proposta surgiu a partir da necessidade de alimentar animais mesmo
quando o tutor não está em casa.

Para isso, desenvolvemos um protótipo utilizando um ESP32 como
controlador principal, um servomotor para acionar a comporta de
liberação da ração, um sensor ultrassônico para estimar o nível do
reservatório e uma estrutura produzida por impressão 3D.

O ESP32 se conecta à rede Wi-Fi e hospeda uma interface web que pode
ser acessada por dispositivos conectados à mesma rede.

## Funcionalidades

- Alimentação automática em horários programados
- Liberação manual de ração pela interface web
- Controle da comporta por servomotor
- Sincronização de horário utilizando NTP
- Interface web hospedada diretamente no ESP32
- Visualização do estado do alimentador
- Configuração do ângulo de abertura do servo
- Configuração do tempo de abertura da comporta
- Monitoramento do nível de ração
- Estimativa percentual de ração disponível
- Indicação de nível baixo de ração
- Controle e monitoramento pelo Monitor Serial

## Como funciona

### Alimentação automática

O sistema possui horários de alimentação previamente configurados.
Quando o horário programado é atingido, o ESP32 aciona o servomotor,
que abre a comporta do reservatório durante o período configurado.

No código atualmente publicado, os horários definidos são:

- 08:00
- 14:00
- 20:00

O horário é obtido por meio de um cliente NTP configurado para o
fuso horário UTC-3. 

Para o ESP32 se conectar na internet deve-se mudar os campos 

const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";

### Controle manual

A interface web permite liberar ração manualmente, sem depender de um
horário programado.

Também é possível consultar informações do sistema e acompanhar o
estado atual do alimentador.

### Monitoramento do nível de ração

Um sensor ultrassônico foi instalado na parte superior do reservatório
para medir a distância até a ração.

A partir das medições, o sistema calcula uma estimativa percentual do
nível de ração disponível e identifica quando o nível está baixo.

### Interface web

O servidor web é executado diretamente no ESP32. A interface permite
interagir com o alimentador utilizando um navegador conectado à mesma
rede Wi-Fi.

Entre as informações disponibilizadas estão:

- horário atual;
- estado da alimentação;
- posição do servo;
- tempo de abertura;
- percentual estimado de ração;
- distância medida pelo sensor;
- indicação de nível baixo.

## Hardware

- ESP32
- Servomotor
- Sensor ultrassônico
- Estrutura para o reservatório
- Comporta para liberação da ração
- Componentes eletrônicos auxiliares
- Estrutura produzida por impressão 3D

## Tecnologias e bibliotecas

- C/C++
- ESP32
- Wi-Fi
- HTML/CSS/JavaScript
- NTP
- HTTP
- Arduino
- `WiFi.h`
- `WiFiUdp.h`
- `NTPClient.h`
- `ESP32Servo.h`
- `WebServer.h`

## Desenvolvimento

Durante o desenvolvimento, foram realizados testes e calibrações
principalmente relacionados ao acionamento do servomotor e à medição
do nível de ração.

A abertura da comporta precisou ser ajustada para encontrar uma posição
adequada de funcionamento, evitando problemas na liberação da ração.

Também foram realizados testes com o sensor ultrassônico para
relacionar a distância medida à quantidade aproximada de ração
disponível no reservatório.

## Objetivo acadêmico

Este projeto foi desenvolvido como parte de uma atividade acadêmica,
com o objetivo de aplicar conhecimentos de programação, eletrônica,
automação e desenvolvimento de sistemas embarcados em uma aplicação
prática.

O resultado foi um protótipo funcional de alimentador automático,
integrando hardware, software, comunicação Wi-Fi e uma interface web.

## Arquivo principal

O código do projeto está disponível em:

`feeder.ino`
