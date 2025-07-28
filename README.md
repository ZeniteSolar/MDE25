# Visão Geral do Sistema

O sistema de direção elétrica do barco solar é composto por duas placas interligadas, uma de controle e outra de potência. O propósito do sistema é ser capaz de controlar o leme do barco conforme comandos recebidos pela rede CAN do barco enviados pelo Módulo Interface de Controle (MIC).

O barco contém atualmente a revisão [MDE22](https://github.com/ZeniteSolar/MDE22). Como pode ser visto, atualmente o barco contém uma bateria auxiliar de 12V dedicada para alimentar o sistema de direção elétrica, algo que não é tão proveitoso, sendo que o barco possui um banco de baterias principal que é mais que capacitado para alimentar o sistema de direção elétrica. Assim sendo, a bateria auxiliar implica em peso e eletrônica adicional, sendo que requer um Módulo de Carregamento de Bateria Auxiliar.

Tendo em vista a fundamentação acima, a revisão 2025 (MDE25) tem por objetivo:

* Remover a bateria auxiliar de 12V
* Utilizar o banco de baterias principal para alimentar o sistema de direção elétrica
* Melhorar aspectos gerais do sistema, velocidade de resposta, confiabilidade, etc.

Assim sendo, os requisitos abaixo foram definidos, vale notar que vários dos valores já são conhecidos de revisões anteriores, por isso não serão abordados aqui, mas podem ser encontrados no GitHub da [Zenite Solar](https://github.com/ZeniteSolar).

## Requisitos

* Tensão de entrada: 10-60V
* Tensão de saída: 10-60V
* Corrente média nominal: 20-30A
* Corrente média máxima: 42A
* Corrente de pico máxima: 80A
* Full Bridge (Reversão Completa)

## Estrutura do Projeto

Para o desenvolvimento do sistema, tendo em vista que a fabricação das placas foi realizada localmente, ou seja, não se atinge qualidade de empresas especializadas, foi decidido que o sistema seria desenvolvido em duas placas, uma de controle e outra de potência. Permitindo tolerância a erros de fabricação maiores e facilitando a fabricação e manutenção em geral.

---

### 1. Placa de Controle

A placa de controle é responsável por realizar a parte lógica do sistema, bem como gerar as tensões baixas, $V_{DDs}$ em geral e $V_{drive}$. Assim sendo, é constituída majoritariamente por um microcontrolador, um transceptor CAN, e um conjunto de conversores Step-Down / LDOs responsáveis por gerar as tensões necessárias para o sistema.

#### Componentes Principais

* **Microcontrolador:** STM32L431
* **Comunicação:** Rede CAN a 500 kbit/s baseado no transceptor MCP2561.
* **Interfaces:**
  * Conector CAN (também fornece 18 V para alimentação)
  * UART / ST-LINK para programação e depuração
  * Conector para sensor de posição do leme (potenciômetro)
  * Conectores para encoder e fim de curso (redundância)

---

### 2. Placa de Potência

A placa de potência é um sistema extremamente simples, sendo suas únicas funções controlar o motor utilizando uma Ponte H conforme sinais de PWM recebidos da placa de controle, prover sinais de medição, ou seja (Tensão de entrada, Tensão de saída e Corrente de entrada), sendo esses já tratados para serem convertidos pelo ADC do microcontrolador da placa de controle.

## Componentes Principais

* **Topologia:** Ponte H (full-bridge) com dois drivers UCC27211DDAR e quatro MOSFETs BSC093N15NS5.
* **Sensoriamento:**
  * Corrente: shunt de 1 mΩ + amplificador de instrumentação (INA283)
* Tensão: divisor resistivo com filtro passa-baixa de 40 Hz, objetivo de reduzir ruído de chaveamento/ringing.
* **Conectores:**
  * Alimentação principal via KRE (baterias chumbo-ácido 30-42 V ou lítio até 60 V)
  * Saída para motor modelo CIM (modelo FR801‑001)
* **Dissipação térmica:**
  * 5 vias de cobre (Ø 0,7 mm) por MOSFET
  * Thermal pad de 1 mm de espessura e 1 cm² por MOSFET
  * Dissipador montado entre as placas

---

# Fluxo de Sinal e Potência

1. **Entrada de Comando:**
   A placa MIC mede a posição do volante e transmite via CAN o setpoint da posição do leme através da mensagem `CAN_MSG_MIC19_MDE_ID`.

2. **Leitura e Controle:**
   O STM32L431 recebe o comando via CAN, valida o ID e seta o setpoint no controlador PID (utilizando apenas ganho proporcional devido a testes anteriores revelarem conflito entre piloto e ganho integral), que por consequência ajusta o PWM dos drivers conforme o erro entre o setpoint e a posição do leme.

3. **Ponte H:**
   Os drivers UCC27211 acionam os MOSFETs BSC093N15NS5 conforme o PWM gerado. Sendo que pode ser controlada tanto a direção quanto a velocidade do motor através da direção da corrente estabelecida pelo chaveamento e o duty cycle do PWM.

4. **Motor e Feedback:**
   O motor movimenta o leme, cuja posição é retroalimentada via um potenciômetro de 10 kΩ acoplado ao eixo e lido pelo ADC do STM32L431, que por sua vez atualiza a posição atual no controlador PID.

5. **Proteções e Limites:**
   * Monitoramento de corrente via shunt
   * Faixa de tensão de entrada: 10 V a 60 V
   * Faixa de tensão de saída: 0 V a 12 V (Motor projetado para 12 V)
   * Corrente média operacional: 20–30 A (No uso normal do barco)
   * Corrente contínua máxima: 42 A (com eixo travado até a queima) com picos de 80 A.
   * Potência média máxima: 425 W (Teste realizado até a queima da ponte H por incapacidade de dissipação e consequente dessolda dos MOSFETs)
   * Dissipador alcançou 60 C após 2 minutos de teste com reversão contínua, ou seja, ir de um extremo ao outro do leme (condição não usual)

---

# Etapas da Placa de Potência

## Entrada

Função de receber a tensão da bateria principal e alimentar a ponte H. Possui um fusível de 20 A para proteção e um banco de capacitores para estabilização da tensão que será utilizada pela ponte H.

![Etapa de Entrada Placa de Potência](./assets/etapa_entrada.png)

* Conector KRE de alimentação (bateria principal)
* Fusível de 20 A
* Shunt de 1 mΩ para medição de corrente
* Filtros:
  * 2 × 470 µF eletrolíticos
  * 4 × 1 µF cerâmicos para redução de ripple e compensação de indutâncias parasitas

---

## Drivers

Os drivers são responsáveis por converter o sinal de PWM (Baixa tensão 3.3V e baixa capacidade de corrente) para um sinal de gate apropriado para os MOSFETs, além de que, por se tratar de uma ponte H completa, é necessário um driver com capacidade de chavear MOSFETs high-side e low-side. Assim sendo, foram utilizados dois drivers UCC27211DDAR, um para cada braço da ponte H. Mais especificações podem ser encontradas no datasheet do driver [UCC27211DDAR](https://www.ti.com/lit/ds/symlink/ucc27211.pdf) e no texto abaixo.

![Drivers](./assets/drivers.png)

* 2 × UCC27211DDAR (meia ponte, tecnologia bootstrap)
* Acionamento complementar com limitação de corrente de gate por resistores de 62 Ω, ajustado em laboratório conforme notas de aplicação [External Gate Resistor Design Guide for Gate Drivers](https://www.ti.com/lit/ab/slla385a/slla385a.pdf)
* Resistores de 10 kΩ entre gate e source para evitar operação na região linear quando em alta impedância.
* Proteções adicionais conforme [Bootstrap Circuitry Selection for Half-Bridge
Configurations](https://www.ti.com/lit/an/slua887a/slua887a.pdf):
  * 2 diodos para proteção
  * Resistor de 2 Ω para limitar corrente de bootstrap

---

## Ponte H (MOSFETs + Snubber)

A ponte H é composta por quatro MOSFETs, dois para cada braço da ponte H, sendo que cada braço é composto por um MOSFET high-side e um MOSFET low-side, permitindo a reversão completa do motor e controle de velocidade através do duty cycle do PWM.

![Ponte H](./assets/mosfets.png)

* 4 × BSC093N15NS5 (150 V, 87 A, R<sub>DS(on)</sub> = 9,3 mΩ)
* Proteção por snubber com TVS de 111 V (breakdown), 160 V (clamping)
* Estratégia de chaveamento:

  * Um braço da ponte permanece fixo (conduzindo), o outro chaveia.
  * O braço fixo alterna de acordo com o sentido do motor.
  * Para evitar sobrecarga no bootstrap, é garantido no controlador que o MOSFET acionado no braço fixo seja sempre o MOSFET low-side, assim sendo, o bootstrap não será completamente descarregado, pois o MOSFET high-side não conduzirá, evitando entrada na região linear dos MOSFETs.

---

## Sensor de Corrente (Entrada)

![Sensor de Corrente](./assets/sensor_de_corrente.png)

* Shunt de 1 mΩ + amplificador INA283 (ganho de 50 V/V)
* Saturação a 3,3 V → Corrente máxima medida ≈ 66 A

---

## Sensor de Tensão (Entrada e Saída)

![Sensor de Tensão](./assets/sensor_de_tensao.png)

* Divisor resistivo para adequar até 60 V para 2,6 V
* Filtro passa-baixa de 100 nF a 40 Hz
* Margem contra saturação do ADC e interferência do zener de proteção (3,3 V)
> Para medição da tensão de saída deve ser utilizado modo diferencial do ADC, assim sendo possível a medição da tensão tanto positiva quanto negativa. Isso ocorre pois, dependendo do sentido da corrente, a tensão de saída pode ser positiva ou negativa.

---

# Placa de Controle

A placa de controle, como já mencionado, é responsável por realizar as seguintes funções:

* Gerar níveis de tensão baixos (15V, 5V, 3.3V), note-se que o conector RJ45 da rede CAN já fornece 18V.
* Receber o setpoint desejado pelo piloto através da rede CAN.
* Monitorar a posição do leme através do potenciômetro.
* Gerar uma ação de controle para o motor através do PWM.

![Placa de Controle](./assets/placa_de_controle.png)

* Alimentação lógica e dos drivers por 18 V da rede CAN
* Comunicação CAN conforme padrão do barco solar
* MCU STM32L431:
  * Leitura de posição do leme (potenciômetro via ADC)
  * Implementação do controle proporcional
  * Geração do PWM de controle para os drivers

# Implementação do Projeto

Dado que praticamente todos os componentes já eram impostos (Já existiam no inventário da Zenite Solar e não podiam ser adquiridos novos atualmente), o projeto foi simplesmente a realização dos requisitos. Assim sendo, foi utilizado o software KiCad para a elaboração do projeto.

[Esquemático da Placa de Controle](./hardware/control/outputs/control.pdf)

[Esquemático da Placa de Potência](./hardware/power/outputs/power.pdf)

Com o projeto em mãos, foi realizada a fabricação das placas, ambas placas foram fabricadas localmente utilizando-se a técnica de fotolitografia com tinta UV e uma impressora de resina Elegoo Mars 2 PRO para a exposição UV. Os químicos utilizados foram os padrões para esse tipo de processo (Carbonato de Sódio 1%, Percloreto de Ferro e Acetona). Após a manufatura, ambas placas foram estanhadas, tendo em vista que o ambiente de trabalho é propenso à corrosão e o estanhamento melhora a vida útil das placas.

## Placa de Controle
![Placa de Controle Topo](./assets/mde25/control_board_top.jpeg)
![Placa de Controle Base](./assets/mde25/control_board_bottom.jpeg)

## Placa de Potência
![Placa de Potência Topo](./assets/mde25/power_board_top.jpeg)
![Placa de Potência Base](./assets/mde25/power_board_bottom.jpeg)

## Placa de controle montada

![Placa de Controle Montada Topo](./assets/mde25/control_board_complete_top.jpeg)
![Placa de Controle Montada Base](./assets/mde25/control_board_complete_bottom.jpeg)

## Placa de potência montada

![Placa de Potência Montada Topo](./assets/mde25/power_board_complete_top.jpeg)
![Placa de Potência Montada Base](./assets/mde25/power_board_complete_bottom.jpeg)

## Sistema de dissipação térmica

![Dissipador 1](./assets/mde25/heatsink_top.jpeg)

![Dissipador 2](./assets/mde25/heatsink_bottom.jpeg)

![Dissipador 3](./assets/mde25/heatsink_assembly_partial.jpeg)

![Dissipador 4](./assets/mde25/heatsink_assembly_final.jpeg)

## MDE25 - Montagem Final

![MDE25 - Montagem Final Frente](./assets/mde25/complete_front.jpeg)

![MDE25 - Montagem Final Trás](./assets/mde25/complete_back.jpeg)

![MDE25 - Montagem Final Topo](./assets/mde25/complete_top.jpeg)

![MDE25 - Montagem Final Base](./assets/mde25/complete_bottom.jpeg)

---

# Testes

Para monitoramento/ajustes, um pequeno aplicativo Python foi desenvolvido, pode-se encontrar [Aqui](./firmware/Extra/TelemetryPanel/panel.py). Esse aplicativo foi utilizado para verificar comportamento do sistema e ajustar o controlador PID.

## Simples exemplo de um teste em hardware real (Direção + Leme) aproximando uma resposta ao degrau

![Resposta ao degrau](./assets/step_response.png)

## Simples exemplo de um teste em hardware real (Direção + Leme) aproximando um setpoint seguindo uma onda triangular

![Setpoint triangular](./assets/triangle_response.png)

Como pode ser visto, o sistema operou corretamente conforme o esperado, abaixo mais detalhes sobre os testes são apresentados.

## Limite a limite velocidade constante

Foi realizado um teste com o volante indo de limite a limite a velocidade constante:

![Teste de Limite a Limite Velocidade Constante](./assets/teste_limites.jpg)

* O motor leva cerca de 1s para atingir o extremo oposto (Ótimo tempo de resposta)
* Na reversão, consome cerca de 40 A por 500 ms
* Corrente estabiliza em torno de 11 A após aceleração
* Temperatura do dissipador atingiu 60 C após 2 min de teste contínuo (Esse comportamento de teste contínuo não é um caso esperado que o piloto realize em operação normal)

## Teste de queima

Após a validação do sistema, desejou-se testar a capacidade de sobrecarga do MDE25, para assim entender os limites e como o sistema vem a falhar. Para isso, o eixo do leme foi travado e o piloto comandou a direção de um extremo ao outro do leme.

As métricas do painel de monitoramento foram as seguintes:

![Teste de queima](./assets/delta_death.png)

Foi necessário cerca de 30 segundos para que o sistema entrasse em sobrecarga e viesse a queima. Como pode ser visto pelo painel, no momento em que entra em sobrecarga, a tensão na bateria principal caiu cerca de 4V. Indo de aproximadamente 38V para 34V, isso demonstra que um estresse muito grande foi aplicado ao sistema, tendo em vista que se trata de um banco de baterias de chumbo com alta capacidade de corrente. Após a queima, o sistema foi desmontado e averigou-se a causa da queima.

## Detalhes da queima e notas sobre o sistema

Dado que os MOSFETs utilizados realizam a sua troca de calor com o dissipador por um pad térmico, ou seja, a transferência não é realizada pelo case do componente, mas sim pelo seu pad de dreno, é necessário que a placa seja capaz de conduzir esse calor de forma efetiva para o dissipador, assim foram feitas vias de cobre (5 vias de 0.7mm para cada MOSFET). Segue abaixo a foto das vias térmicas.

![Vias Térmicas](./assets/mde25/vias.png)

Como pode ser observado, um dos conjuntos de vias ficou apenas com 3 vias efetivamente transferindo calor, pois 2 acabaram sendo lixadas pelo processo de fabricação, isso resultou com que o MOSFET associado com essas vias viesse a sobreaquecer e dessoldar, não sendo o calor diretamente a causa da queima, mas sim a dessolda do MOSFET que resultou em uma gota de solda dando curto no braço da ponte H e após isso gerando a queima. Segue foto do MOSFET queimado.

<div style="text-align: center; max-width: 20%; margin: 0 auto;">
  <img src="./assets/mde25/dead_nmos.jpeg" alt="MOSFET Queimado" style="width: 100%;">
</div>

# Notas extras sobre estimativa da Indutância do Motor

Para estimar a indutância do motor, utilizamos o ripple de corrente observado em regime permanente com o motor em velocidade constante.

## Parâmetros do teste:

* Corrente média: **11 A**
* Ripple de corrente: **10%** → $\Delta I = 1{,}1\,\text{A}_{pp}$
* Tensão de alimentação: **36 V**
* Duty cycle: **40%**
* Frequência de PWM: **20 kHz** (→ período $T_s = 50\,\mu\text{s}$)
* Topologia: **Ponte H full-bridge com modulação bipolar**

## Modelo de cálculo:

Assumindo modulação bipolar, a variação de corrente durante o ciclo de comutação é dada por:

$$
\Delta I = \frac{2 \cdot V \cdot D \cdot T_s}{L}
\Rightarrow
L = \frac{2 \cdot V \cdot D \cdot T_s}{\Delta I}
$$

Substituindo os valores:

$$
L = \frac{2 \cdot 36\,\text{V} \cdot 0{,}4 \cdot 50 \times 10^{-6}\,\text{s}}{1{,}1\,\text{A}}
= \frac{1{,}44 \cdot 10^{-3}}{1{,}1}
\approx 1{,}31\,\text{mH}
$$

## Resultado:

A indutância estimada do motor é:

$$
\boxed{L \approx 1{,}31\;\text{mH}}
$$

# Sobre o Firmware

## Arquitetura de Software

O firmware está organizado em módulos funcionais especializados:
- **Core/control**: Implementação do controlador PID
- **Core/pwm**: Geração de sinais PWM para ponte H
- **Core/sense**: Aquisição de dados via ADC
- **Core/CAN**: Servidor de comunicação CAN
- **Core/uart_server**: Interface de debug e configuração
- **Core/telemetry**: Sistema de monitoramento
- **Core/status**: Gerenciamento de estado e feedback sonoro

## Configuração do Sistema

O firmware possui diversos parâmetros que podem ser ajustados para otimizar o sistema para as necessidades do usuário, esses podem ser ajustados no arquivo de configuração [config.h](./firmware/Core/Inc/config.h).

## Módulos de Firmware

### 1. Módulo de Controle (Core/control)

#### Funcionalidades Principais
O módulo implementa um controlador PID digital completo (control.c) com as seguintes características técnicas:

**Parâmetros de Controle:**
- Ganho Proporcional (Kp): 2.8 (inicial, configurável)
- Ganho Integral (Ki): 0.0 (inicial, configurável)
- Ganho Derivativo (Kd): 0.0 (inicial, configurável)
- Janela de média móvel: 15 amostras para filtragem de setpoint para evitar mudança abrupta de setpoint.

**Algoritmo PID:**
```
error = setpoint - feedback
integral += error * dt
integral = clamp(integral, -0.1, 0.1)  // Anti-windup
derivative = (error - error_previous) / dt
output = Kp * error + Ki * integral + Kd * derivative
```

**Características de Implementação:**
- **Filtragem de Setpoint** (control.c): Média móvel de 15 amostras para suavizar comandos de entrada e reduzir oscilações
- **Saturação de Saída**: Limitação entre -1.0 e +1.0 com zona morta configurável
- **Anti-windup**: Limitação da integral entre -0.1 e 0.1 para evitar saturação
- **Período Dinâmico**: Baseado na frequência real de amostragem do ADC
- **Lock de Setpoint**: Capacidade de travamento para operação manual

### 2. Módulo PWM (Core/pwm)

#### Geração de Sinais PWM
O módulo controla a ponte H através de dois canais PWM complementares (pwm.c):

**Especificações Técnicas:**
- **Timer**: TIM1 (Advanced Timer) utilizado para gerar os sinais PWM.
- **Frequência Base**: 24 kHz (configurável acima de 1.3 kHz)
- **Resolução**: 16 bits
- **Canais**: CH1 e CH2 com saídas complementares (CHxN)

**Compensação de Tensão** (pwm.c):
O sistema implementa compensação automática baseada na tensão de entrada:
```
max_duty = PWM_MAX_OUT_VOLTAGE / clamped_input_voltage;
effective_duty = map(input_duty, -1.0, 1.0, -max_duty, max_duty);
```

**Características de Segurança:**
- **Zona Morta**: Configurável (-0.06 a +0.06) para evitar vibrações e perda de energia sem realização de movimento útil.

#### Configurações
- **Tensão Máxima de Saída (Ajustável)**: 12V
- **Faixa de Tensão de Entrada**: 24V a 60V
- **Dead Time**: Configurado para proteção da ponte H, otimizado em laboratório para melhor tempo sem gerar curto no braço.

### 3. Módulo de Sensoriamento (Core/sense)

#### Aquisição de Dados ADC
Sistema de conversão analógico-digital de 4 canais simultâneos (sense.c:10-16):

**Canais de Conversão:**
```
typedef enum {
  ADC_RANK_CURRENT_CONTROL_POINT = 0U,  // Posição do leme
  ADC_RANK_INPUT_VOLTAGE = 1U,           // Tensão de entrada
  ADC_RANK_OUTPUT_VOLTAGE = 2U,          // Tensão de saída
  ADC_RANK_INPUT_CURRENT = 3U,           // Corrente de entrada
} adc_ranks_t;
```

**Características Técnicas:**
- **Resolução**: 16 bits (65535 níveis) (Com Oversample, dado cru 12 bits)
- **Modo**: Conversão contínua via DMA
- **Trigger**: Timer 2 (frequência configurável) atrelado também a atualização da malha de controle.
- **Buffer**: Circular DMA para aquisição contínua

**Calibração e Linearização:**
Cada canal possui calibração específica (sense.c), calibrações do ADC podem ser feitas utilizando o seguinte [script](./firmware/Extra/ADC_Linearization/linearization.py):
- **Tensão de Entrada**: `V = 0.001150 * ADC - 0.088064`
- **Tensão de Saída**: `V = 0.002327 * ADC - 73.731254`
- **Posição**: `P = (ADC * 2.0 / 65535.0) - 1.0` (normalizada ±1.0)
- **Corrente**: `I = ADC * 3.3 / 65535.0` (referência 3.3V)

### 4. Servidor CAN (Core/CAN)

#### Protocolo de Comunicação
Implementa cliente CAN para recepção de comandos do MIC19 (can_server.c):

**Filtro CAN** (can_server.c):
- **Modo**: ID Mask para filtragem seletiva
- **Banco**: 0 (FIFO0)
- **Máscara**: 0x7FF (11 bits de ID padrão)

**Especificações do Protocolo:**
- **ID da Mensagem**: 31 (CAN_MSG_MIC19_MDE_ID)
- **Comprimento**: 4 bytes
- **Frequência**: 50 Hz
- **Assinatura**: 240 (CAN_SIGNATURE_MIC19)

**Estrutura da Mensagem:**
```
Byte 0: Assinatura (240)
Byte 1: Position_L (8 bits baixos)
Byte 2: Position_H (8 bits altos)
Byte 3: Reservado
```

**Processamento de Dados** (can_server.c):

Normalização do setpoint -1.0 a 1.0:

```
uint16_t position = rxData[1] | (rxData[2] << 8);
float steering_angle = (position / 1024.0f) * 2.0f - 1.0f;
```

#### Sistema de Watchdog
**Timeout de Comunicação** (can_server.c):
- **Timer**: TIM7 para detecção de perda de comunicação
- **Reset Automático**: Contador zerado a cada mensagem válida
- **Ação de Timeout**: Setpoint zerado e status de desconexão. Gera sinal sonoro de sequência específica de BEEP no motor. Reconexão também irá resultar em sinal sonoro de sequência específica de BEEP no motor.

### 5. Servidor UART (Core/uart_server)

#### Interface de Debug e Configuração
Sistema de comunicação serial para debug e ajuste de parâmetros (uart_server.c):

**Protocolo de Comando:**
- **Formato**: `<comando:valor>`
- **Delimitadores**: `<` (início) e `>` (fim)
- **Parser**: Máquina de estados para robustez
- **Buffer**: 128 bytes com buffer circular

**Comandos Disponíveis:**
```
'D': Set PWM duty cycle (-1.0 a 1.0)
'F': Set PWM frequency (Hz)
'p': Set PID proportional gain
'i': Set PID integral gain
'd': Set PID derivative gain
's': Set ADC sampling frequency
'L': Lock/unlock PWM (1/0)
'R': System reset
```

#### Implementação do Parser
**Máquina de Estados** (uart_server.c):
```
UART_SERVER_STATE_WAIT_START      -> Aguarda '<'
UART_SERVER_STATE_WAIT_TERMINATOR -> Coleta dados até '>'
UART_SERVER_STATE_NEW_MESSAGE     -> Mensagem completa
UART_SERVER_STATE_ERROR           -> Overflow ou erro
```
### 6. Sistema de Telemetria (Core/telemetry)

#### Estrutura de Dados
Coleta e transmite métricas do sistema (telemetry.h):

```
typedef struct {
  uint16_t preamble;           // 0x5A54
  float sense_input_voltage;   // Tensão de entrada
  float sense_input_current;   // Corrente de entrada
  float sense_output_voltage;  // Tensão de saída
  float sense_control_point;   // Posição atual
  float control_setpoint;      // Setpoint desejado
  float control_error;         // Erro de controle
  float pwm_duty;             // Duty cycle nominal
  float pwm_effective_duty;   // Duty cycle efetivo
  uint8_t status_connected;   // Status de conexão
} telemetry_t;
```

**Características de Transmissão:**
- **Frequência**: 40 Hz (intervalo de 25ms)
- **Interface**: UART a 115200 bps
- **Formato**: Printf formatado para debug
- **Habilitação**: Condicional via DEBUG_TELEMETRY_ENABLE

#### Saída de Debug
Formato de telemetria (telemetry.c):
```
"VI:%f, II:%f, VO:%f, CP:%f, SP:%f, ER:%f, D:%f, ED:%f, ST:%hu\n"
```

### 7. Módulo de Status (Core/status)

#### Sistema de Feedback Sonoro
Gerencia indicação sonora de estado do sistema (status.c):

**Sequências de Status:**
- **Conexão**: 3 tons crescentes (Baixo → Médio → Alto)
- **Desconexão**: 3 tons decrescentes (Alto → Médio → Baixo)
- **Desconectado**: Tom único alto

**Parâmetros Sonoros:**
```
#define STATUS_TONE_FREQUENCY_LOW_HZ    2093.0f  // C7
#define STATUS_TONE_FREQUENCY_MEDIUM_HZ 2794.0f  // F7
#define STATUS_TONE_FREQUENCY_HIGH_HZ   4186.0f  // C8
#define STATUS_TONE_DURATION_S          0.16f
#define STATUS_TONE_PWM_VOLUME          0.70f
```

#### Implementação Técnica
**Geração de Tom** (status.c):
- **Timer**: TIM6 para base de tempo
- **Modulação**: PWM alternado entre +70% e -70%
- **Cálculo de Frequência**: `repetitions = duration / (1/frequency)`
- **Callback**: Sistema de callbacks encadeados para sequências

**Controle de Estado:**
- **Variável Global**: `device_connected` para rastreamento
- **Transições**: Apenas em mudanças de estado
- **Proteção**: Evita múltiplas execuções da mesma sequência

### 8. Utilitários Matemáticos (utils.h)

```
float clampf(float value, float min, float max);
// Limita valor entre mínimo e máximo

float passf(float value, float replace, float cut_min, float cut_max);
// Zona morta - substitui valores dentro de faixa

float mapf(float value, float min_in, float max_in, float min_out, float max_out);
// Mapeia valor de uma faixa para outra
```

### Configuração de Periféricos
**Timers Utilizados:**
- **TIM1**: PWM principal (24 kHz, centro-alinhado)
- **TIM2**: Trigger para ADC (frequência configurável)
- **TIM6**: Base de tempo para status sonoro
- **TIM7**: Watchdog de comunicação CAN

**DMA Channels:**
- **ADC1**: Conversão contínua 4 canais
- **UART1**: Recepção com idle detection
