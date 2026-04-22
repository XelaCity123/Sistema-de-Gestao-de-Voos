#!/bin/bash
# SO_HIDE_DEBUG=1                   ## Uncomment this line to hide all @DEBUG statements
# SO_HIDE_COLOURS=1                 ## Uncomment this line to disable all escape colouring
. ./so_utils.sh                     ## This is required to activate the macros so_success, so_error, and so_debug

###############################################################################
## ISCTE-IUL: Trabalho prático de Sistemas Operativos 2023/2024
##
## Aluno: Nº:       Nome:
## Nome do Módulo: S1. Script: regista_passageiro.sh
## Descrição/Explicação do Módulo:
##
##
###############################################################################

## S1.1. Valida os argumentos passados e os seus formatos:
## S1.1.1. Valida os argumentos passados, avaliando se são em número suficiente (mínimo 3, máximo 4). Em caso de erro, dá so_error S1.1.1 e termina. Caso contrário, dá so_success S1.1.1.
if [ $# -eq 3 ] || [ $# -eq 4 ];then
    so_success S1.1.1
else
    so_error S1.1.1
    exit
fi
## S1.1.2. Valida se o argumento <Nome> corresponde ao nome de um utilizador do servidor Tigre. Se não corresponder ao nome de nenhum utilizador do Tigre, dá so_error S1.1.2 e termina. Senão, dá so_success S1.1.2.
Nome=$1
n=$(grep "$1" /etc/passwd | wc -l)
a=$(echo "$1"",,,")
j=$(cat /etc/passwd | grep "$1" | cut -f5 -d":")
if [ "$a" = "$j" ]; then
    so_success S1.1.2
else
    so_error S1.1.2
    exit
fi

## S1.1.3. Valida se o argumento <Saldo a adicionar> tem formato “number” (inteiro positivo ou 0). Se não tiver, dá so_error S1.1.3 e termina. Caso contrário, dá so_success S1.1.3.
if [[ $3 =~ ^[0-9]+$ ]]; then
    so_success S1.1.3
else
    so_error S1.1.3
    exit
fi
## S1.1.4. Valida se o argumento opcional <NIF> (só no caso de ser passado, i.e., se tiver valor) tem formato “number” com 9 (nove) dígitos. Se não for, dá so_error S1.1.4 e termina. Caso contrário, dá so_success S1.1.4.
if ! [ -z $4 ]; then
    if [[ $4 =~ ^[0-9]{9}$ ]]; then
        so_success S1.1.4
    else
        so_error S1.1.4
        exit
    fi
fi
## S1.2. Associa os dados passados com a base de dados dos passageiros registados:
## S1.2.1. Verifica se o ficheiro passageiros.txt existe. Se o ficheiro existir, dá so_success S1.2.1 e continua no passo S1.2.3. Se não existir, dá so_error S1.2.1, e continua.
## S1.2.2. Cria o ficheiro passageiros.txt. Se der erro, dá so_error S1.2.2 e termina. Senão, dá so_success S1.2.2.
if [ -e "passageiros.txt" ]; then
    so_success S1.2.1
else
    so_error S1.2.1
    if touch "passageiros.txt"; then
        so_success S1.2.2
    else
        so_error S1.2.2
        exit
    fi
fi
## S1.2.3. Caso o passageiro <Nome> passado já exista no ficheiro passageiros.txt, dá so_success S1.2.3, e continua no passo S1.3. Senão, dá so_error S1.2.3, e continua.
n2=$(cat passageiros.txt | cut -f3 -d":" passageiros.txt | grep "$1" passageiros.txt | wc -l)
if ! [ $n2 -eq 0 ];then
    so_success S1.2.3
else
    so_error S1.2.3
    ## S1.2.4. Como o passageiro <Nome> não existe no ficheiro, terá de o registar. Para isso, valida se <NIF> (campo opcional) foi mesmo passado. Se não foi, dá so_error S1.2.4 e termina. Senão, dá so_success S1.2.4.
    if ! [[ $4 =~ ^[0-9]{9}$ ]]; then
        so_error S1.2.4
        exit
    else
        so_success S1.2.4
    fi

    ## S1.2.5. Define o campo <ID_passageiro>, como sendo o UserId Linux associado ao utilizador de nome <Nome> no servidor Tigre. Em caso de haver algum erro na operação, dá so_error S1.2.5 e termina. Caso contrário, dá so_success S1.2.5 <ID_passageiro> (substituindo pelo campo definido).
    ID_passageiro=$(cat /etc/passwd | grep "$1" /etc/passwd | cut -f1 -d":")
    n3=$(cat /etc/passwd | grep "$ID_passageiro" /etc/passwd | wc -l)
    if [ $n3 -eq 0 ]; then
        so_error S1.2.5
        exit
    else
        so_success S1.2.5
    fi

    ## S1.2.6. Define o campo <Email>, gerado a partir do <Nome> introduzido pelo utilizador, usando apenas o primeiro e o último nome, convertendo-os para minúsculas apenas, colocando um ponto entre os dois nomes, e domínio iscteflight.pt. Assim sendo, um exemplo seria “david.gabriel@iscteflight.pt”. Se houver algum erro na operação (e.g., o utilizador “root” tem menos de 2 nomes), dá so_error S1.2.6 e termina. Caso contrário, dá so_success S1.2.6 <Email> (substituindo pelo campo gerado). Ao registar um novo passageiro no sistema, o número inicial de <Saldo> tem o valor 0 (zero).
    e=$(echo $1 | tr ' ' '.')
    primeiro=$(echo "$e" | awk -F'.' '{print $1}')
    ultimo=$(echo "$e" | awk -F'.' '{print $NF}')
    E=$primeiro.$ultimo@iscteflight.pt
    Email=${E,,}
    no=$(echo $1 | wc -w)
    if [ $no -lt 2 ]; then
        so_error S1.2.6
        exit
    else
        so_success S1.2.6 $Email
    fi

    ## S1.2.7. Regista o utilizador numa nova linha no final do ficheiro passageiros.txt, seguindo a sintaxe: <ID_passageiro>:<NIF>:<Nome>:<Email>:<Senha>:<Saldo>. Em caso de haver algum erro na operação (e.g., erro na escrita do ficheiro), dá so_error S1.2.7 e termina. Caso contrário, dá so_success S1.2.7 <linha> (substituindo pela linha completa escrita no ficheiro).
    Saldo=0
    NIF=$4
    Senha=$2
    l=$ID_passageiro:$NIF:$Nome:$Email:$Senha:$Saldo
    echo "$l" >> passageiros.txt
    if [ $? -ne 0 ]; then
        so_error S1.2.7
        exit
    else
        so_success S1.2.7
    fi
fi

## S1.3. Adiciona créditos na conta de um passageiro que existe no ficheiro passageiros.txt:
## S1.3.1. Tendo já encontrado um “match” passageiro com o Nome <Nome> no ficheiro, valida se o campo <Senha> passado corresponde à senha registada no ficheiro. Se não corresponder, dá so_error S1.3.1 e termina. Caso contrário, dá so_success S1.3.1.
p=$(cat passageiros.txt | grep "$1" | cut -f5 -d":")
Senha=$2
if [ $p = $Senha ];then
    so_success S1.3.1
else
    so_error S1.3.1
    exit
fi

## S1.3.2. Mesmo que tenha sido passado um campo <NIF> (opcional), ignora-o. Adiciona o valor passado do campo <Saldo a adicionar> ao valor do <Saldo> registado no ficheiro passageiros.txt para o passageiro em questão, atualizando esse valor no ficheiro passageiros.txt. Se houver algum erro na operação (e.g., erro na escrita do ficheiro), dá so_error S1.3.2 e termina. Caso tudo tenha corrido bem, dá o resultado so_success S1.3.2 <Saldo> (substituindo pelo valor saldo atualizado no ficheiro passageiros.txt).
Saldo=$(grep "$1" passageiros.txt | cut -f6 -d":")
sa=$3
ss=$((sa + Saldo))
k=$(grep -n "$1" passageiros.txt | cut -f1 -d":")
sed -i ${k}s/$Saldo/$ss/ passageiros.txt
if [ "$?" -eq 0 ]; then
    so_success S1.3.2 $ss
else
    so_error S1.3.2
    exit
fi
## S1.4. Lista todos os passageiros registados, mas ordenados por saldo:
## S1.4.1. O script deve criar um ficheiro chamado passageiros-saldos-ordenados.txt igual ao que está no ficheiro passageiros.txt, com a mesma formatação, mas com os registos ordenados por ordem decrescente do campo <Saldo> dos passageiros. Se houver algum erro (e.g., erro na leitura ou escrita do ficheiro), dá so_error S1.4.1, e termina. Caso contrário, dá so_success S1.4.1.
if ! [ -e passageiros-saldos-ordenados.txt ]; then
    touch passageiros-saldos-ordenados.txt
fi

if [ -e passageiros-saldos-ordenados.txt ]; then

    sort -t ":" -k6nr passageiros.txt > passageiros-saldos-ordenados.txt
    if [ $? -ne 0 ]; then
       so_error S1.4.1
       exit
    else
        so_success S1.4.1
    fi
fi