/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2023/2024, Enunciado Versão 1+
 **
 ** Aluno: Nº: 122657      Nome: Alexandre Duarte
 ** Nome do Módulo: cliente.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int msgId;                              // Variável que tem o ID da Message Queue
MsgContent clientRequest;               // Variável que serve para as mensagens trocadas entre Cliente e Servidor

/**
 * @brief Processamento do processo Cliente
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // C1
    msgId = initMsg_C1();
    so_exit_on_error(msgId, "initMsg_C1");
    // C2
    so_exit_on_error(triggerSignals_C2(), "triggerSignals_C2");
    // C3
    so_exit_on_error(getDadosPedidoUtilizador_C3(), "getDadosPedidoUtilizador_C3");
    // C4
    so_exit_on_error(sendRequest_C4(), "sendRequest_C4");
    // C5: CICLO6
    while (TRUE) {
        // C5
        configureTimer_C5(MAX_ESPERA);
        // C6
        so_exit_on_error(readResponseSD_C6(), "readResponseSD_C6");
        // C7
        int lugarEscolhido = trataResponseSD_C7();
        if (RETURN_ERROR == lugarEscolhido)
            terminateCliente_C9();
        // C8
        if (RETURN_ERROR == sendSeatChoice_C8(lugarEscolhido))
            terminateCliente_C9();
    }
}

/**
 *  "O módulo Cliente é responsável pela interação com o utilizador.
 *   Após o login do utilizador, este poderá realizar atividades durante o tempo da sessão.
 *   Assim, definem-se as seguintes tarefas a desenvolver:"
 */

/**
 * @brief C1: Ler a descrição da tarefa no enunciado
 * @return o valor de msgId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initMsg_C1 () {
    msgId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // tenta ligar-se á memoria partilhada e ve se aconteceu algum erro
    msgId = msgget(IPC_KEY, 0);
    if(msgId < 0 ){
        so_error("C1","erro ao aceder o MSG");
        return -1;
    }else{
        so_success("C1","%d msgId");
        return msgId;
    }

    so_debug("> [@return:%d]", msgId);
    return msgId;
}

/**
 * @brief C2: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_C2 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //arma os sinais
    if( signal(SIGHUP, trataSinalSIGHUP_C10) == SIG_ERR || signal(SIGINT, trataSinalSIGINT_C11) == SIG_ERR || signal(SIGALRM, trataSinalSIGALRM_C12) == SIG_ERR ){
        so_error("C2","Erro ao armar os sinais");
        return -1;
    }else{
        so_success("C2","sinais bem armados");
        return 0;
    }

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C3: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int getDadosPedidoUtilizador_C3 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //parte visual da questão
    printf("IscteFlight: Check-in Online\n");
    printf("----------------------------\n");
    printf("\n");
    printf("Introduza o NIF do passageiro: _");
    scanf("%d", &clientRequest.msgData.infoCheckIn.nif);
    printf("\n");
    printf("Introduza a Senha do passageiro: _");
    scanf("%20s", clientRequest.msgData.infoCheckIn.senha);

    //verifica se o nif têm no maximo 9 elementos
    int n = 0;
    int a = clientRequest.msgData.infoCheckIn.nif;
    while(a > 0){
        a = a / 10;
        n++;
    }

    if(n <= 0 || n > 9){
        so_error("C3","%d invalido",&n);
        return -1;
    }
    
    so_success("C3","nif correto");
    return 0;


    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C4: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendRequest_C4 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //preenche o tipo da mensagem, o pidCliente e o pidServidorDedicado
    clientRequest.msgType = MSGTYPE_LOGIN;
    clientRequest.msgData.infoCheckIn.pidCliente = getpid();
    clientRequest.msgData.infoCheckIn.pidServidorDedicado = PID_INVALID;

    //envia o pedido e vê se aconteceu algum erro
    int status = msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0);
    if ( status < 0 ) {
        so_error("C4","erro ao enviar a mensagem");
        return -1;
    }

    so_success("C4","%d %s %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.senha, getpid());
    return 0;

    //preenche os campos pidCliente e pidServidorDedicado
    clientRequest.msgData.infoCheckIn.pidCliente = getpid();
    clientRequest.msgData.infoCheckIn.pidServidorDedicado = PID_INVALID;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C5: Ler a descrição da tarefa no enunciado
 * @param tempoEspera o tempo em segundos que queremos pedir para marcar o timer do SO (i.e., MAX_ESPERA)
 */
void configureTimer_C5 (int tempoEspera) {
    so_debug("< [@param tempoEspera:%d]", tempoEspera);

    //arma um alarme e envia um sinal
    alarm(tempoEspera);
    so_success("C5","Espera resposta em %d segundos", tempoEspera);

    signal(SIGALRM, trataSinalSIGALRM_C12);

    so_debug(">");
}

/**
 * @brief C6: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readResponseSD_C6 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //lê a mesnagem e vê se aconteceu algum erro
    int a = msgrcv(msgId, &clientRequest, sizeof(MsgContent), clientRequest.msgData.infoCheckIn.pidCliente, 0);
    if(a == -1){
        so_error("C6","erro na leitura");
        return -1;
    }else{
        so_success("C6","%d %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido, clientRequest.msgData.infoCheckIn.pidCliente);
        return 0;
    }
    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C7: Ler a descrição da tarefa no enunciado
 * @return Nº do lugar escolhido (0..MAX_SEATS-1) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int trataResponseSD_C7 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //desativa o alarma
    int tentativas = 0;
    alarm(0);
    
    //verifica se o pidServidorDedicado é válido
    if(clientRequest.msgData.infoCheckIn.pidServidorDedicado == PID_INVALID){
        so_error("C7.2","erro nas operações");
        exit(-1);
    }

    //verifica se o lugarEscolhido é válido
    if(clientRequest.msgData.infoCheckIn.lugarEscolhido != EMPTY_SEAT){
        so_success("C7.3","Reserva concluída: %s %s %d", clientRequest.msgData.infoVoo.origem, clientRequest.msgData.infoVoo.destino, clientRequest.msgData.infoCheckIn.lugarEscolhido);
        exit(0);
    }else{
        //ve se é a primeira vez
        if(tentativas == 0){
            so_success("C7.4.1","reserva feita");
            tentativas++;
        }else{
            so_error("C7.4.1","Erro na reserva de lugar");
            printf("Lamentamos, não foi possível reservar o seu lugar, terá de selecionar um novo lugar");
        }
    }


    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C8: Ler a descrição da tarefa no enunciado
 * @param lugarEscolhido índice do array lugares que o utilizador escolheu, entre 0 e MAX_SEATS-1
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendSeatChoice_C8 (int lugarEscolhido) {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //define o tipo da mensagem e preenche os campos nif, pidCliente e lugarEscolhido
    MsgContent m;
    m.msgType = clientRequest.msgData.infoCheckIn.pidServidorDedicado;
    m.msgData.infoCheckIn.nif = clientRequest.msgData.infoCheckIn.nif;
    m.msgData.infoCheckIn.pidCliente = getpid();
    m.msgData.infoCheckIn.lugarEscolhido = clientRequest.msgData.infoCheckIn.lugarEscolhido;

    //tenta enviar essa mensagem
    int status = msgsnd(msgId, &m, sizeof(m.msgData), 0);
    if ( status < 0 ) {
        so_error("C8","Erro ao enviar");
        return -1;
    }

    so_success("C8","%d %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido, getpid());
    return 0;


    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief C9: Ler a descrição da tarefa no enunciado
 */
void terminateCliente_C9 () {
    so_debug("<");

    //verifica se o pidServidorDedicado é valido, caso que seja envia um sinal 
    if(clientRequest.msgData.infoCheckIn.pidServidorDedicado == PID_INVALID){
        so_error("C9","erro no pid");
        exit(-1);
    }else{
        so_success("C9","pid sem erros");
        kill(clientRequest.msgData.infoCheckIn.pidServidorDedicado, SIGUSR1);
        exit(0);
    }

    so_debug(">");
    exit(0);
}

/**
 * @brief C10: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGHUP_C10 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("C10","Check-in concluído sem sucesso");
    exit(0);

    so_debug(">");
}

/**
 * @brief C11: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_C11 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    //chama a funçao terminateCliente_C9
    so_success("C11","Cliente: Shutdown");
    terminateCliente_C9();

    so_debug(">");
}

/**
 * @brief C12: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGALRM_C12 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    //vê se o cliente esperou mais tempo que o MAX_ESPERA
    if( sinalRecebido > MAX_ESPERA){
        so_error("C12","Cliente: Timeout");
        terminateCliente_C9();
    }

    so_debug(">");
}