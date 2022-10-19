# STM32F446-Oscilloscope
Osciloscópio usando o seu ARM (STM32F446)
Versão 1.2

DESCRIÇÃO

Programa simples para a utilização de um STM32F446 como osciloscópio de até 3 canais, não requerindo qualquer programa extra para sua utilização. O gráfico da forma de onda coletada é exibido em formato bitmap no computador, sendo de facil manipulação e visualização. Os dados coletados pelo microcontrolador, tais como período, frequência, tensão de pico a pico e os valores de volts/div e seg/div também são exibidos em um arquivo de texto (.txt), de fácil leitura.


COMO UTILIZAR

Basta passar o arquivo "Osc_v1-2.bin" para o seu microcontrolador através do diretório criado quando ele é conectado ao computador ('D:/', 'F:/', ou similar). Em seguida, basta executar o arquivo "Osc v1.2.exe" e seguir as instruções.


SOBRE O USO DO ARM

NUNCA UTILIZE TENSÕES MAIORES QUE 3,3v OU MENORES QUE 0v.Para medir tensões acima de 3,3v ou menores que 0v, deve-se condicionar o sinal de forma discreta ao microcontrolador


SOBRE OS CANAIS

Canal 0
Cor: Vermelho
Pino ARM: PA0

Canal 1
Cor: Verde
Pino ARM: PA1

Canal 2
Cor: Azul
Pino ARM: PA4


SOBRE A TAXA DE AMOSTRAGEM

São disponibilizadas 8 taxas de amostragem para a aquisição de diferentes sinais.
As taxas de 1kHz, 5kHz, 10kHz, 50kHz, 100kHz e 500kHz estão disponíveis para todos os canais. A taxa de amostragem de 1MHz só está disponível para a aquisição em 1 ou 2 canais e a taxa de 2MHz só pode ser usada para a aquisição em apenas 1 canal


Recomendações para a taxa de amostragem a ser utilizada (para máxima resolução):

Sinais de 0,2Hz a 1Hz: 1kHz
Sinais de 1Hz a 5Hz: 5kHz
Sinais de 2Hz a 10Hz: 10kHz
Sinais de 10Hz a 50Hz: 50kHz
Sinais de 20Hz a 100Hz: 100kHz
Sinais acima de 100Hz: 500kHz
Sinais acima de 200Hz: 1MHz


SOBRE O NÚMERO DE AMOSTRAS:
O programa disponibiliza a opção do usuário escolher o número de amostras que serão exibidas no gráfico.Recomenda-se, no geral, que o número de amostras seja a escolha automática (inserir valor 0 no número de amostras), dessa forma o gráfico será dimensionado automaticamente para o periodo do sinal
