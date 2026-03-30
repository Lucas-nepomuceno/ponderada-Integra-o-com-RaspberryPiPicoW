# Ponderada - Integração com Raspberry Pi Pico W

&emsp; Este repositório contém o firmware responsável por integrar a Raspberry Pi Pico W ao [backend da atividade anterior](https://github.com/Lucas-nepomuceno/ponderada-em-sala-Fila). O desenvolvimento foi realizado com as seguintes especificações:

**Framework/Toolchain utilizado:** Arduino Framework  

**Sensores integrados:**

- **Sensor de presença**  
  - Tipo: Sensor de Presença  
  - Pino: 11  
  - Intervalo de valores esperados: 0 ou 1  

- **Sensor de temperatura**  
  - Tipo: Sensor de Temperatura  
  - Pino: GP28_A2  
  - Intervalo de valores esperados: 0 a 100  

---

## Instruções para compilação e gravação

&emsp; Para compilar o código e gravá-lo na Pico W, siga os passos abaixo:

1. Instale a biblioteca Earle Philhower acessando *Preferences >> Additional Boards Manager URLs* e adicionando o link:  
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json  

2. Acesse *Tools > Board > Boards Manager*, busque por **"Raspberry Pi RP2040"** e instale o pacote.

3. Conecte a Pico W ao computador mantendo pressionado o botão **BOOTSEL** enquanto conecta via USB.

4. Selecione a placa **Raspberry Pi Pico W** em *Tools > Board*.

5. Selecione a porta correta em *Tools > Port*.

6. Abra o arquivo `.ino` presente neste repositório.

7. Clique em **Upload**. O Arduino IDE irá compilar e gravar automaticamente na Pico W.

---

## Instruções para configuração de rede e endpoint

&emsp; A configuração da rede com a biblioteca `WiFi.h` é simples: basta alterar, no código, os parâmetros da função `WiFi.begin()` com o nome da sua rede e a respectiva senha.

&emsp; Para configurar o endpoint, é necessário primeiro [clonar o repositório anterior](https://github.com/Lucas-nepomuceno/ponderada-em-sala-Fila) e executar o Docker Compose conforme as instruções disponíveis nele. Em seguida, altere a constante `SERVER_URL_POST`, substituindo o IP padrão `192.168.15.38` pelo IP da sua máquina.

&emsp; Para descobrir o seu IP, utilize o comando `ipconfig` no terminal e procure pelo endereço **IPv4**.

---

## Evidência de Funcionamento

&emsp; Abaixo, apresenta-se um vídeo demonstrativo do sistema:

<p>Vídeo 1 - Demonstração do Funcionamento do Sitema</p>

[![Assista ao vídeo](https://img.youtube.com/vi/cwPcd_0YDRc/0.jpg)](https://www.youtube.com/watch?v=cwPcd_0YDRc)

<p>Fonte: o autor (2026)</p>


&emsp; No vídeo, é possível observar a interação entre o firmware e o backend. No terminal da Arduino IDE, o identificador da requisição (ID do dispositivo) é exibido juntamente com a mensagem padrão do servidor ("Sua requisição será processada em breve"). Paralelamente, no backend, é possível verificar o recebimento das requisições POST e, após breve processamento, sua persistência no banco de dados.

---

## Funcionamento da integração

&emsp; A integração ocorre conforme o fluxo abaixo:

1. A Raspberry Pi Pico W coleta os dados do sensor de presença no pino 11;  
2. Aplica-se um algoritmo de *debouncing* para reduzir ruídos;  
3. O valor processado é armazenado na variável `presence_state`;  
4. Esse valor é enviado ao backend, juntamente com outras informações (como timestamp), por meio de uma requisição HTTP POST em formato JSON.

&emsp; O backend espera receber um JSON no seguinte formato:

```json
POST /dados-sensores HTTP/1.1
Host: 192.168.15.38:8080
Content-Type: application/json

{
  "id": "1000006",
  "timestamp": "2026-03-23T23:10:45-03:00",
  "tipo-sensor": "Termistor",
  "tipo-leitura": "analogico",
  "valor": 27.45
}
```

&emsp; Essa formatação é realizada pela função send_sensor_data(), que também é utilizada para o envio dos dados do sensor de temperatura. Nesse caso, porém, o ruído é tratado por meio de uma média móvel, garantindo maior estabilidade nas leituras.
