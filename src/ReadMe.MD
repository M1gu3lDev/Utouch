# utouch: Monitoramento Biomecânico para Reabilitação de AVC

O **utouch** é um ecossistema *wearable* de baixo custo projetado para quantificar e validar a evolução motora de pacientes em reabilitação pós-Acidente Vascular Cerebral (AVC). O dispositivo transforma dados inerciais brutos em métricas clínicas objetivas, permitindo que fisioterapeutas acompanhem o progresso do paciente com dados científicos.

---

## 🛠️ Arquitetura Técnica

### Hardware
* **Microcontrolador:** ESP32 DevKit V1 (Processamento Dual-Core e Bluetooth Serial nativo).
* **Unidade de Medição Inercial (IMU):** MPU-9250 (Acelerômetro, Giroscópio e Magnetômetro - 9 eixos).
* **Sensor de Pressão:** BMP280 (Barômetro/Altímetro integrado no módulo 10 DOF para medição de elevação vertical).

### Firmware e Bibliotecas
* **Linguagem:** C++ utilizando o framework Arduino.
* **Ambiente:** PlatformIO (VS Code).
* [cite_start]**Biblioteca Base:** I2Cdev / MPU9250 (Baseada na coleção de Jeff Rowberg)[cite: 9, 178].
* [cite_start]**Configuração de Sensibilidade:** * Acelerômetro: $\pm 2g$ ($16384$ LSB/mg) [cite: 190, 614-617].
    * [cite_start]Giroscópio: $\pm 250^\circ/s$ ($131$ LSB/deg/s)[cite: 190, 641].

---

## 🧠 Conceitos Biomecânicos Aplicados

O sistema utiliza fusão de dados para extrair ângulos de Euler e métricas de controle motor:

### 1. Amplitude de Movimento (ROM)
Mensura o deslocamento angular máximo de uma articulação durante um exercício específico (como a flexão de ombro). No utouch, focamos no eixo de **Pitch**.
$$ROM = | \text{Pitch}_{max} - \text{Pitch}_{min} |$$

### 2. Suavidade do Movimento (Jerk)
Calcula a "limpeza" do movimento. Pacientes em reabilitação costumam apresentar micro-paradas ou tremores, o que aumenta o valor do Jerk (derivada da aceleração).
$$J = \frac{da}{dt}$$

### 3. Erro de Trajetória (RMSE)
Para exercícios de desenho espacial (como o círculo com a mão), medimos o erro quadrático médio entre a trajetória ideal e a realizada pelo paciente.
$$\text{RMSE} = \sqrt{\frac{1}{n} \sum_{i=1}^{n} (x_i - x'_i)^2}$$



---

## 📊 Plano de Validação do Protótipo

| Tipo | Métrica | Método de Validação | Critério de Sucesso |
| :--- | :--- | :--- | :--- |
| **Analítica** | Precisão Angular | Comparação simultânea com goniômetro manual mecânico. | Erro médio $< 3^\circ$. |
| **Analítica** | Estabilidade | Verificação de ruído estático após Filtro Complementar. | Variância residual $< 0.1^\circ$. |
| **Experimental** | Drop Point | Medição do ângulo onde o paciente perde o controle de velocidade. | Repetibilidade do gatilho de aceleração. |
| **Experimental** | Usabilidade | Teste assistido com Especialista em Reabilitação de AVC. | Validação qualitativa das métricas. |

---

## 🚀 Roteiro de Desenvolvimento (Roadmap)

### Fase 1: Estabilização de Firmware (Concluída ✅)
* [cite_start]Integração I2C entre ESP32 e MPU9250 [cite: 600-602].
* Correção de Baud Rate para **115200** no `platformio.ini`.
* [cite_start]Verificação de ID de hardware via registrador `WHO_AM_I` ($0x75$)[cite: 23, 192].

### Fase 2: Módulo de Captura de Dados (Em andamento 🔄)
* Implementação de rotinas de calibração automática (`calibrateAccelGyro`) para eliminar o *drift* inercial.
* Criação de gatilhos Bluetooth para Início (`'s'`) e Fim (`'f'`) de testes de ROM.

### Fase 3: Validação Clínica e RMSE
* Coleta de trajetórias padrão com indivíduos saudáveis para criação de base de comparação (Baseline).
* Implementação do cálculo de RMSE no aplicativo móvel para feedback visual.

---

## 📂 Como Executar

1.  **Instalação:** Clone este repositório e abra-o no VS Code com a extensão PlatformIO instalada.
2.  **Configuração:** Certifique-se de que o arquivo `platformio.ini` contém `monitor_speed = 115200`.
3.  **Conexão:**
    * **SDA:** GPIO 21
    * **SCL:** GPIO 22
    * **VCC:** 3.3V
4.  **Uso:** Abra o Monitor Serial e utilize o comando `'s'` para iniciar uma gravação de Amplitude de Movimento (ROM).

---

## 👥 Equipe e Papéis

* **CTO & Lead Embedded Engineer:** Arquitetura de hardware, desenvolvimento de firmware e processamento digital de sinais (DSP).
* **Especialista em Reabilitação:** Validação de protocolos clínicos e supervisão de exercícios terapêuticos.
* **Desenvolvedor App/UI:** Interface de biofeedback visual e processamento de trajetória.

---

> [cite_start]**Aviso de Isenção Técnica:** A implementação atual baseia-se em uma versão parcial da classe de dispositivo MPU9250, com funcionalidades de magnetômetro em estágio de desenvolvimento ativo[cite: 10, 11, 181].

# Entendendo o Pitch (Arfagem) no Sistema utouch

O **Pitch** é uma das métricas fundamentais capturadas pelo sensor MPU-9250 para monitorar a reabilitação de membros superiores. Ele descreve a orientação espacial do dispositivo em relação ao plano horizontal.

## 1. O que é o Pitch?

[cite_start]O Pitch representa a rotação do sensor em torno do seu **eixo lateral (Eixo Y)** [cite: 644-645]. 

* **Referência Física:** Imagine o bico de um avião subindo ou descendo.
* **Aplicação no utouch:** Quando o sensor é fixado ao braço do paciente, o Pitch mede a inclinação do membro em relação ao horizonte.
    * **Braço em repouso (ao lado do corpo):** Pitch próximo de $0^\circ$.
    * **Braço levantado (em direção à cabeça):** O valor aumenta progressivamente até atingir aproximadamente $90^\circ$ quando apontado para o teto.

## 2. Como o Pitch é calculado?

[cite_start]O cálculo do Pitch é resultado de uma **fusão de dados** processada pela biblioteca para mitigar as limitações individuais de cada sensor interno [cite: 114-115]:

### A. Através do Acelerômetro (Dados Estáticos)
[cite_start]O acelerômetro detecta a aceleração da gravidade terrestre nos eixos X, Y e Z [cite: 604-606]. A fórmula trigonométrica para converter as leituras de força-G em graus de inclinação é:

$$Pitch = \arctan2(A_x, \sqrt{A_y^2 + A_z^2}) \times \left(\frac{180}{\pi}\right)$$

* **Vantagem:** Oferece uma referência absoluta de "chão" que não sofre desvios (*drift*).
* **Limitação:** Sensível a vibrações e ruídos de movimento.

### B. Através do Giroscópio (Dados Dinâmicos)
[cite_start]O giroscópio mede a velocidade angular em torno do eixo Y [cite: 641-644]. Para obter o ângulo, realiza-se a integração da velocidade em relação ao tempo ($\Delta t$):

$$\theta_{novo} = \theta_{anterior} + (\omega_{gyro} \times \Delta t)$$

Onde $\omega_{gyro}$ é a velocidade angular em graus por segundo.
* **Vantagem:** Imune a vibrações externas e reage instantaneamente.
* **Limitação:** Acúmulo de erro sistemático (*drift*).

### C. Fusão e Cálculo de Amplitude (ROM)
Para a validação do progresso do paciente, o sistema utiliza a diferença entre os picos registrados durante o exercício:

$$ROM = | Pitch_{max} - Pitch_{min} |$$

## 3. Configurações de Precisão

[cite_start]Para garantir a validade clínica, o firmware inicializa o sensor com as seguintes sensibilidades [cite: 189-190]:
* [cite_start]**Giroscópio:** $\pm 250^\circ/s$ ($131$ LSB por grau/segundo)[cite: 641].
* [cite_start]**Acelerômetro:** $\pm 2g$ ($16384$ LSB/g) [cite: 614-617].

Estas configurações garantem a resolução necessária para detectar pequenas melhorias na reabilitação motora.