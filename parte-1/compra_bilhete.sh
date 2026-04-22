#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh                     ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S2. Script: compra_bilhete.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## Este script não recebe nenhum argumento, e permite que o passageiro compre um bilhete para um voo da lista de voos disponíveis. Para realizar a compra, o passageiro deve fazer login para confirmar sua identidade e saldo disponível. Os voos disponíveis estão listados no ficheiro voos.txt.
## S2.1. Validações e Pedido de informações interativo:
## S2.1.1. O script valida se os ficheiros voos.txt e passageiros.txt existem. Se algum não existir, dá so_error e termina. Caso contrário, dá so_success.
if [ ! -f voos.txt ] || [ ! -f passageiros.txt ]; then
    so_error S2.1.1
    exit
else
    so_success S2.1.1
fi

## S2.1.2. Na plataforma é possível consultar os voos pela sua <Origem> ou <Destino>. O utilizador insere a cidade Origem ou Destino do voo (o interesse é que pesquise nos 2 campos). Caso o utilizador tenha introduzido uma cidade que não exista no ficheiro voos.txt, ou se não existirem voos com lugares disponíveis com origem ou destino nessa cidade, dá so_error e termina. Caso contrário, dá so_success <Cidade>.
read -p "Insira a cidade de origem ou destino do voo:" cidade
v=$(grep $cidade voos.txt | wc -l)
l=$(grep $cidade voos.txt | cut -f8 -d":" | sort | head -n 1)
if [ $v -eq 0 ] || [ $l -eq 0 ]; then
    so_error S2.1.2
    exit
else
    so_success S2.1.2 $cidade
fi

## S2.1.3. O programa pede ao utilizador para inserir uma opção de voo, listando os voos que existem de acordo com a origem/destino inserida anteriormente, da seguinte forma: O utilizador insere a opção do voo (neste exemplo, números de 1 a 3 ou 0). Se o utilizador escolheu um número de entre as opções de voos apresentadas (neste caso, entre 1 e 3), dá so_success <opção>. Caso contrário, dá so_error e termina.
grep $cidade voos.txt | nl -w 6 -s"." | awk -F: '{printf "%d.%s para %s, %s, Partida:%s, Preço: %d, Disponíveis:%d lugares\n", $1, $2, $3, $4, $5, $6, $8}'
echo "0.Sair"    
vl=$(grep $cidade voos.txt | cut -f8 -d":" | wc -l)
read -p "Insira o voo que pretende reservar:" voo
if [ $voo -ge 1 ] && [ $voo -le $vl ]; then
    so_success $voo
else
    so_error S2.1.3
    exit
fi

## S2.1.4. O programa pede ao utilizador o seu <ID_passageiro>: O utilizador insere o respetivo ID de passageiro (dica: UserId Linux). Se esse ID não estiver registado no ficheiro passageiros.txt, dá so_error e termina. Caso contrário, reporta so_success <ID_passageiro>.
read -p "Insira o ID do seu utilizador:" ID
g=$(grep $ID passageiros.txt | wc -l)
if ! [ $g -eq 1 ]; then
    so_error S2.1.4
    exit
else
    so_success $ID
fi

## S2.1.5. O programa pede ao utilizador a sua <Senha>: O utilizador insere a respetiva senha. Caso o script veja que essa senha não é a registada para esse passageiro no ficheiro passageiros.txt, dá so_error e termina. Caso contrário, reporta so_success.
read -p "Insira a senha do seu utilizador:" se
p=$(grep $ID passageiros.txt | cut -f5 -d":")
if [ $p = $se ]; then
    so_success S2.1.5
else
    so_error S2.1.5
    exit
fi
## S2.2. Processamento da resposta:
## S2.2.1. Valida se o passageiro possui <Saldo>, definido no ficheiro passageiros.txt, para comprar o bilhete selecionado no passo S2.1.3. Se a compra não é possível por falta de saldo, dá so_error <preço voo> <Saldo> e termina. Caso contrário, dá so_success <preço voo> <Saldo>.
sp=$(grep $ID passageiros.txt | cut -f6 -d":")
pv=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f6 -d":")
if [ $sp -ge $pv ]; then
    so_success S2.2.1 $pv $sp
else
    so_error S2.2.1 $pv $sp
    exit
fi

## S2.2.2. Subtrai o valor do <preço voo> no <Saldo> do passageiro, e atualiza o ficheiro passageiros.txt. Em caso de erro (e.g., na escrita do ficheiro), dá so_error e termina. Senão, dá so_success <Saldo Atual>.
sa=$((sp-pv))
k=$(grep -n "$ID" passageiros.txt | cut -f1 -d":")
sed -i ${k}s/$sp/$sa/ passageiros.txt
if [ $? -ne 0 ]; then
    so_error S2.2.2
    exit
else
    so_success S2.2.2 $sa
fi

## S2.2.3. Decrementa uma unidade aos lugares disponíveis do voo escolhidos no passo S2.1.3, e atualiza o ficheiro voos.txt. Em caso de erro (por exemplo, na escrita do ficheiro), dá so_error e termina. Senão, dá so_success.
la=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f8 -d":")
lf=$((la-1))
b=$(grep -n $cidade voos.txt | cut -f1 -d":" | sed -n "${voo}p")
cut -f8 -d":" voos.txt | sed -i ${b}s/$la$/$lf/ voos.txt
if [ $? -ne 0 ]; then
    so_error S2.2.3
    exit
else
    so_success S2.2.3
fi

## S2.2.4. Regista a compra no ficheiro relatorio_reservas.txt, inserido uma nova linha no final deste ficheiro. Em caso de erro (por exemplo, na escrita do ficheiro), dá so_error e termina. Caso contrário, dá so_success.
if ! [ -e relatorio_reservas.txt ]; then
    touch relatorio_reservas.txt
fi
Origem=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f2 -d":")
NrVoo=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f1 -d":")
Destino=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f3 -d":")
pre=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f6 -d":")
Data=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f4 -d":")
Hora=$(grep $cidade voos.txt | sed -n "${voo}p" | cut -f5 -d":")
ID_res=$(tail -n 1 relatorio_reservas.txt | cut -f1 -d":")
ID_reserva=$((ID_res+1))
u=$ID_reserva:$NrVoo:$Origem:$Destino:$pre:$ID:$Data:$Hora
echo "$u" >> relatorio_reservas.txt
if [ $? -ne 0 ]; then
    so_error S2.2.4
    exit
else
    so_success S2.2.4
fi