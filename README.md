# nrf52840-DK-project

## Sobre

Este projeto foi desenvolvido para uma Placa de Desenvolvimento nrf52840-DK com Zephyr RTOS 2.7.1. 

### Desafio

"O projeto deve ter dois botões (Botão 1 e Botão 2) em que quando não estão pressionados, nada acontece.   
Entretanto, quando o Botão 1 é pressionado, deve piscar o LED1 a cada 500 us, além de ler um contador I2C (S-35770) conectado na placa com a maior frequência suportada.  
Já quando o Botão 2 é pressionado, deve acender o LED2 imediatamente, e deve permanecer ligado enquanto o Botão 2 é pressionado.  
Caso o Botão 1 e Botão 2 sejam apertados juntos, a aplicação deve esperar a soltura de quem foi pressionado primeiro para realizar a tarefa do segundo.   
Uma exceção vai ser o piscar do LED1 que deve ser executada toda vez.  
Por fim, quando o pino conectado ao LOOP da S-35770 for mudado de sinal lógico, deve resetar o contador.  
A aplicação deve também mostrar todas as suas ações.  
Você deve usar Kconfig symbols para remover todas as operações de contador para um cenário em que o aplicativo seria compilado e testado sem o contador anexado ao DK."  

## Implementação

Os botões usados no código foram os próprios botões da nrf52840-DK, sendo eles: 
- Botão 1 (P0.11)
- Botão 2 (P0.12)

Além disso, os LEDS utilizados também encontram-se na placa: 
- LED 1 (P0.13) 
- LED 2 (P0.14). [^1]

Para encontrar as configurações dos pinos foi usado da API de Devicetree disponibilizado pelo Zephyr no GitHub [^2]. Deste modo, é possível usar esta API para encontrar as portas através dos comandos:
`DT_ALIAS`, `DT_LABEL` e `DT_NODELABEL` [^3]. 


Em seguida foi criado um struct para as GPIOs com as suas localizações na API [^4]. Outro fato importante é saber o endereço do slave S-35770 para fazer uso do I2C. Quando é analisado o datasheet, é possível ver que o endereço é "00110010" ou "0x32" em hexadecimal. 


Depois de construído todas as `struct gpio_dt_spec` [^5] para os botões e leds, foi criado `struct device` [^6] para os pinos de `I2C`, `LOOP` e `RESET`. Por conseguinte, foi configurado as portas como saída (`LEDS` e `RESET`) e entradas (`BOTÕES` e `LOOP`). Para as GPIOs configuradas como entradas, utilizou-se do método de pull up para não ter problema com entrada de dados flutuantes.

Após configurado cada porta lógica, foi criado uma variável que terá o valor do pino de `TOGGLE` e um flag que terá o mesmo valor inicial. Deste modo, haverá uma condição que quando o valor do pino `TOGGLE` for diferente da `flag TOGGLE`, irá setar o pino `RESET` para 0, já que no datasheet da S-35770 é possível ver que seu contador reseta quando há uma entrada lógica sendo BAIXA. Assim é completado: 

- [x] Por fim, quando o pino conectado ao `LOOP` da S-35770 for mudado de sinal lógico, deve resetar o contador.

Há também uma condição que se não houver um device na porta `I2C`, faz com que a função `main` termine. Entretanto, caso exista, a programação continua e manda uma mensagem para o debug através da função `printk` [^7]. 
Sendo esse também um dos requisitos do desafio.

- [x] A aplicação deve também mostrar todas as suas ações.


Quando encontrado o dispositivo `I2C`, ele é configurado com a velocidade `ULTRA` [^8]. Completando.

- [x] Além de ler um contador I2C (S-35770) conectado na placa com a maior frequência suportada.

Em seguida, foi criado um `while()` onde há os estados dos botões (se estão em nível lógico ALTO ou BAIXO), setado o pino `RESET` em ALTO e o estado do pino `TOGGLE`. Há diversas condições nesse `while`, uma delas é quando um botão é apertado primeiro faz com que mude a ordem em um vetor. Dessa forma, quando o `Botão 1` é pressionado primeiro, faz com que seja realizado as suas atividades como: blinkar o `LED` e ler o contador `I2C` [^9] e caso o `Botão 2` seja pressionado enquanto o `Botão 1` já está apertado, a condição verifica no vetor quem foi o primeiro a ser apertado e espera ele ser solto para realizar a ação do segundo. O mesmo acontece caso o `Botão 2` seja apertado primeiro e depois o `Botão 1` segundo. Uma exceção é o blink que apenas depende de pressionar o `Botão 1` para realizar essa tarefa.
> Essa parte do código foi testada em um Arduino UNO. 

Com isso, foi concluído:

- [x] Quando o Botão 1 é pressionado, deve piscar o LED1 a cada 500 us, além de ler um contador I2C (S-35770) conectado na placa com a maior frequência suportada. Já quando o Botão 2 é pressionado, deve acender o LED2 imediatamente, e deve permanecer ligado enquanto o Botão 2 é pressionado. Caso o Botão 1 e Botão 2 sejam apertados juntos, a aplicação deve esperar a soltura de quem foi pressionado primeiro para realizar a tarefa do segundo. Uma exceção vai ser o piscar do LED1 que deve ser executada toda vez.  

## Execução do código

### Instalação do Chocolatey

[Instale-o através desse link](https://chocolatey.org/install)

Execute o cmd como administrador e use o seguinte comando :

```bash
choco install cmake --installargs 'ADD_CMAKE_TO_PATH=System'
choco install ninja gperf python git
```

Depois instale o west através de:

```
pip3 install west
```

Após isso, instale [GNU Arm Embedded Toolchain](https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads) no diretório **c:/gnuarmemb**

### Instalação do Zephyr RTOS v2.7.1

No CMD use o seguinte código:

```bash
west init -m https://github.com/zephyrproject-rtos/zephyr.git --mr zephyr-v2.7.1
```

Em seguida, utilize:

```bash
west update
```

Por fim:

```bash
west zephyr-export
```

### Ajustes Finais do Ambiente

Ainda no CMD digite:
```bash
pip3 install -r zephyr/scripts/requirements.txt
```
Depois:

```bash
set ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb
set GNUARMEMB_TOOLCHAIN_PATH=c:\gnuarmemb
```

### Compilação
Para compilar o código, utilize o comando :

```bash
west build -b nrf52840dk_nrf52840 -t guiconfig
```

Deste modo, é aberto o Kconfig interativo [^10] e nele é possível desativar os timers da placa, concluindo a última etapa do desafio.

- [x] Você deve usar Kconfig symbols para remover todas as operações de contador para um cenário em que o aplicativo seria compilado e testado sem o contador anexado ao DK.

### Referências

[^1]:https://docs.zephyrproject.org/2.7.1/boards/arm/nrf52840dk_nrf52840/doc/index.html

[^2]:https://github.com/nrfconnect/sdk-zephyr/blob/main/boards/arm/nrf52840dk_nrf52840/nrf52840dk_nrf52840.dts

[^3]:https://docs.zephyrproject.org/latest/reference/devicetree/api.html

[^4]:https://docs.zephyrproject.org/latest/reference/peripherals/gpio.html

[^5]:https://docs.zephyrproject.org/apidoc/latest/structgpio__dt__spec.html

[^6]:https://docs.zephyrproject.org/apidoc/latest/structdevice.html

[^7]:https://docs.zephyrproject.org/apidoc/latest/printk_8h.html#a768a7dff8592b69f327a08f96b00fa54

[^8]:https://docs.zephyrproject.org/latest/reference/peripherals/i2c.html#c.I2C_SPEED_ULTRA

[^9]:https://docs.zephyrproject.org/latest/reference/peripherals/i2c.html#c.i2c_reg_read_byte

[^10]:https://docs.zephyrproject.org/latest/guides/build/kconfig/menuconfig.html
