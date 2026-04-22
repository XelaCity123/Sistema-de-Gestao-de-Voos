/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº:       Nome: 
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/**
 * @brief Processamento do processo Cliente
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // C1
    checkExistsFifoServidor_C1(FILE_REQUESTS);
    // C2
    triggerSignals_C2();
    // C3 + C4
    CheckIn clientRequest = getDadosPedidoUtilizador_C3_C4();
    // C5
    writeRequest_C5(clientRequest, FILE_REQUESTS);
    // C6
    configureTimer_C6(MAX_ESPERA);
    // C7
    waitForEvents_C7();
    so_exit_on_error(-1, "ERRO: O cliente nunca devia chegar a este ponto");
    return 0;
}

/**
 *  "O módulo Cliente é responsável pela interação com o utilizador.
 *   Após o login do utilizador, este poderá realizar atividades durante o tempo da sessão.
 *   Assim, definem-se as seguintes tarefas a desenvolver:"
 */

/**
 * @brief C1       Ler a descrição da tarefa C1 no enunciado
 * @param nameFifo Nome do FIFO servidor (i.e., FILE_REQUESTS)
 */
void checkExistsFifoServidor_C1 (char *nameFifo) {
    so_debug("< [@param nameFifo:%s]", nameFifo);

    struct stat st;

    // ver se temos acesso ao ficheiro
    if(access(nameFifo, F_OK) == -1){
        so_error("C1", "Erro no acesso ao ficheiro");
        exit(1);
    }else{
        // ver se o ficheiro é um fifo
        if( stat( nameFifo, &st ) < 0) {
            exit(1);
        }

        if(S_ISFIFO(st.st_mode)){
            so_success("C1","é um fifo");
        }else{
            so_error("C1","nao é um fifo");
            exit(1);
        }
        // ver se o ficheiro "server.fifo" existe
        if("server.fifo" == NULL){
            so_error("C1","ficheiro nao existe");
            exit(1);
        }
    }
 

    so_debug(">");
}

/**
 * @brief C2   Ler a descrição da tarefa C2 no enunciado
 */
void triggerSignals_C2 () {
    so_debug("<");

    // quando receber um destes sinais o cliente é redirecionado para as respetivas funçoes
    signal( SIGUSR1 , trataSinalSIGUSR1_C8 );
    signal( SIGHUP , trataSinalSIGHUP_C9 );
    signal( SIGINT, trataSinalSIGINT_C10 );
    signal( SIGALRM , trataSinalSIGALRM_C11 );

    // ver se houve algum erro ao armar os sinais
    if(signal( SIGUSR1 , trataSinalSIGUSR1_C8)  == SIG_ERR ){
        so_error("C2", "Erro ao armar");
        exit(1);
    }
    if(signal( SIGHUP , trataSinalSIGHUP_C9 ) == SIG_ERR){
        so_error("C2", "Erro ao armar");
        exit(1);
    }
    if(signal( SIGINT, trataSinalSIGINT_C10 ) == SIG_ERR){
        so_error("C2", "Erro ao armar");
        exit(1);
    }
    if(signal( SIGALRM , trataSinalSIGALRM_C11 ) == SIG_ERR){
        so_error("C2", "Erro ao armar");
        exit(1);
    }
    so_success("C2", "Sinais armados");

    so_debug(">");
}

/**
 * @brief C3+C4    Ler a descrição das tarefas C3 e C4 no enunciado
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn getDadosPedidoUtilizador_C3_C4 () {
    CheckIn request;
    request.nif = -1;   // Por omissão retorna erro
    so_debug("<");

    // parte visual da questao
    printf("IscteFlight: Check-in Online\n");
    printf("----------------------------\n");
    printf("\n");
    printf("Introduza o NIF do passageiro: _");
    scanf("%d", &request.nif);
    printf("\n");
    printf("Introduza a Senha do passageiro: _");
    scanf("%40s", request.senha);

    
    int n = 0;
    int a = request.nif;
    int p = getpid();

    //ver se o nif está entre 0 e 9
    while(a > 0){
        a = a / 10;
        n++;
    }
    if(n < 0 || n > 9){
        so_error("C3","Nif invalido");
        exit(1);
    }

    request.pidCliente = p;


    so_success("C4", "%d %s %d", request.nif, request.senha, request.pidCliente);

    
    so_debug("> [@return nif:%d, senha:%s, pidCliente:%d]", request.nif, request.senha, request.pidCliente);
    return request;
}

/**
 * @brief C5       Ler a descrição da tarefa C5 no enunciado
 * @param request  Elemento com os dados a enviar
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void writeRequest_C5 (CheckIn request, char *nameFifo) {
    so_debug("< [@param request.nif:%d, request.senha:%s, request.pidCliente:%d, nameFifo:%s]",
                                        request.nif, request.senha, request.pidCliente, nameFifo);
        
        // abre o ficheiro para escrita e leitura
        FILE *fb = fopen(nameFifo, "w+");

        // ver se o fichiero existe
        if(fb == NULL){
            so_error("C5","erro ao abrir o fifo");
            exit(1);
        }else{

            // escrever a informaçao no fifo e ver se ocorreu algum erro
            if(fprintf(fb, "%d\n%s\n%d\n",request.nif, request.senha, request.pidCliente) < 0){
                so_error("C5","erro na escrita");
                exit(1);
            }else{
                so_success("C5","escrita bem sucedida");
            }
        }

        fclose(fb);


    so_debug(">");
}

/**
 * @brief C6          Ler a descrição da tarefa C6 no enunciado
 * @param tempoEspera o tempo em segundos que queremos pedir para marcar o timer do SO (i.e., MAX_ESPERA)
 */
void configureTimer_C6 (int tempoEspera) {
    so_debug("< [@param tempoEspera:%d]", tempoEspera);

    // emite um sinal quando chegar ao "tempoEspera" 
    alarm(tempoEspera);
    so_success("C6","Espera resposta em %d segundos", tempoEspera);
    
    // redireciona o Cliente quando receber esse sinal
    signal(SIGALRM, trataSinalSIGALRM_C11);

    so_debug(">");
}

/**
 * @brief C7 Ler a descrição da tarefa C7 no enunciado
 */
void waitForEvents_C7 () {
    so_debug("<");

    // faz uma pausa
    pause();

    so_debug(">");
}

/**
 * @brief C8            Ler a descrição da tarefa C8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR1_C8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C8","Check-in concluído com sucesso");
    exit(0);

    so_debug(">");
}

/**
 * @brief C9            Ler a descrição da tarefa C9 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGHUP_C9 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C9","Check-in concluído sem sucesso");
    exit(1);

    so_debug(">");
}

/**
 * @brief C10           Ler a descrição da tarefa C10 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_C10 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C10","Cliente: Shutdown");
    exit(0);

    so_debug(">");
}

/**
 * @brief C11           Ler a descrição da tarefa C11 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGALRM_C11 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    // verifica se o tempo maximo de espera foi ultrapassado
   if(sinalRecebido > MAX_ESPERA){
    so_error("C11","Cliente: Timeout");
    exit(1);
   }

    so_debug(">");
}