#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh                     ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S4. Script: stats.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## Este script obtém informações sobre o sistema, afixando resultados diferentes no STDOUT consoante os argumentos passados na sua invocação. A sintaxe resumida é: ./stats.sh <passageiros>|<top <nr>>
## S4.1. Validações:
## S4.1.1. Valida os argumentos recebidos e, conforme os mesmos, o número e tipo de argumentos recebidos. Se não respeitarem a especificação, dá so_error e termina. Caso contrário, dá so_success.
r=$(grep "$1" passageiros.txt | wc -l)
if [ "$1" == "passageiros" ] || [[ $2 =~ ^[1-9]+$ ]]; then
    so_success S4.1.1
else
    so_error S4.1.1
fi

## S4.2. Invocação do script:
## S4.2.1. Se receber o argumento passageiros, (i.e., ./stats.sh passageiros) cria um ficheiro stats.txt onde lista o nome de todos os utilizadores que fizeram reservas, por ordem decrescente de número de reservas efetuadas, e mostrando o seu valor total de compras. Em caso de erro (por exemplo, se não conseguir ler algum ficheiro necessário), dá so_error e termina. Caso contrário, dá so_success e cria o ficheiro. Em caso de empate no número de reservas, lista o primeiro do ficheiro. Preste atenção ao tratamento do singular e plural quando se escreve “reserva” no ficheiro). Um exemplo do ficheiro stats.txt será:
rm -f stats.txt
if [ "$1" == "passageiros" ]; then
    touch stats.txt
    awk -F: '{vpas[$6]++; vpre[$6]+=$5} END {for (pas in vpas) print vpas[pas], vpre[pas], pas}' relatorio_reservas.txt | sort -rn | while IFS=" " read -r cont gasto pas; do
        nomep=$(grep "$pas" passageiros.txt | cut -d ':' -f3)
        if [[ $cont -gt 1 ]]; then
            echo "$nomep: $cont reservas; $gasto€" >> stats.txt
        else 
            echo "$nomep: $cont reserva; $gasto€" >> stats.txt
        fi
    done
    if [ $? -eq 0 ]; then
      so_success S4.2.1
    else
       so_error S4.2.1
       exit
    fi
fi

## S4.2.2. Se receber o argumento top <nr:number>, (e.g., ./stats.sh top 4), cria um ficheiro stats.txt onde lista os <nr> (no exemplo, os 4) voos mais rentáveis (que tiveram melhores receitas de vendas), por ordem decrescente. Em caso de erro (por exemplo, se não conseguir ler algum ficheiro necessário), dá so_error e termina. Caso contrário, dá so_success e cria o ficheiro. Em caso de empate, lista o primeiro do ficheiro; o ficheiro stats.txt ficará então:
if [[ $2 =~ ^[1-9]+$ ]]; then
    rm -f stats.txt
    touch stats.txt
    i=1
    awk -F: '{vvoo[$2]++; vprev[$2]+=$5} END {for (voo in vvoo) print vvoo[voo], vprev[voo], voo}' relatorio_reservas.txt | sort -rn | while IFS=" " read -r con lucro voo && [ $i -le $2 ]; do
        nomev=$(grep "$voo" relatorio_reservas.txt | cut -d ':' -f2 | sort | uniq)
        echo "$nomev: $lucro€" >> stats.txt
        i=$((i + 1))
    done
    t=$(awk -F': ' '{print $2, $1}' stats.txt | sort -nr | awk '{print $2 ": " $1}' | tr ' ' '\n')
    echo $t | tr ' ' '\n' | while read -r num; do
        read -r val;
        echo "$num $val"
    done > stats.txt
    
    if [ $? -eq 0 ]; then
      so_success S4.2.2
    else
       so_error S4.2.2
       exit
    fi
fi