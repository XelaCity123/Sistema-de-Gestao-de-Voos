#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh                     ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S3. Script: estado_voos.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## Este script não recebe nenhum argumento, e é responsável pelo relatório do estado dos voos que pertencem à plataforma IscteFlight.
## S3.1. Validações:
## S3.1.1. O script valida se o ficheiro voos.txt existe. Se não existir, dá so_error e termina. Senão, dá so_success.
if [ -e voos.txt ]; then
    so_success S3.1.1
else
    so_error S3.1.1
fi

## S3.1.2. O script valida se os formatos de todos os campos de cada linha do ficheiro voos.txt correspondem à especificação indicada em S2, nomeadamente se respeitam os formatos de data e de hora. Se alguma linha não respeitar, dá so_error <conteúdo da linha> e termina. Caso contrário, dá so_success.
while IFS= read -r l; do
    echo $l
    if [[ $l =~ ^[A-Z0-9]+:[A-Za-z\s]+:[A-Za-z\s] ]];then
        so_success S3.1.2
    else
        so_error "$l"
    fi
done < voos.txt

## S3.2. Processamento do script:
## S3.2.1. O script cria uma página em formato HTML, chamada voos_disponiveis.html, onde lista os voos com lugares disponíveis, indicando nº, origem, destino, data, hora, o número de lugares disponíveis em cada classe, e o total de lugares disponíveis (para isso deve calcular a soma de todos os lugares disponíveis). Em caso de erro (por exemplo, se não conseguir escrever no ficheiro), dá so_error e termina. Caso contrário, dá so_success.


## S3.3. Invocação do script estado_voos.sh:

## S3.3.1. Altere o ficheiro cron.def fornecido, por forma a configurar o seu sistema para que o script seja executado de hora em hora, diariamente. Nos comentários no início do ficheiro cron.def, explique a configuração realizada, e indique qual o comando que deveria utilizar para despoletar essa configuração. O ficheiro cron.def alterado deverá ser submetido para avaliação juntamente com os outros Shell scripts.
