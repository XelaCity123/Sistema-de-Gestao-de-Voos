/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 3 de Sistemas Operativos 2023/2024, Enunciado Versão 1+
 **
 ** Aluno: Nº: 122657      Nome: Alexandre Duarte
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "defines.h"

/*** Variáveis Globais ***/
int shmId;                              // Variável que tem o ID da Shared Memory
int msgId;                              // Variável que tem o ID da Message Queue
int semId;                              // Variável que tem o ID do Grupo de Semáforos
MsgContent clientRequest;               // Variável que serve para as mensagens trocadas entre Cliente e Servidor
DadosServidor *database = NULL;         // Variável que vai ficar com UM POINTER PARA a memória partilhada
int indexClient = -1;                   // Índice do passageiro que fez o pedido ao servidor/servidor dedicado na BD
int indexFlight = -1;                   // Índice do voo do passageiro que fez o pedido ao servidor/servidor dedicado na BD
int nrServidoresDedicados = 0;          // Número de servidores dedicados (só faz sentido no processo Servidor)

/**
 * @brief Processamento do processo Servidor e dos processos Servidor Dedicado
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // S1
    shmId = initShm_S1();
    if (RETURN_ERROR == shmId) terminateServidor_S7();
    // S2
    msgId = initMsg_S2();
    if (RETURN_ERROR == msgId) terminateServidor_S7();
    // S3
    semId = initSem_S3();
    if (RETURN_ERROR == semId) terminateServidor_S7();
    // S4
    if (RETURN_ERROR == triggerSignals_S4()) terminateServidor_S7();

    // S5: CICLO1
    while (TRUE) {
        // S5
        int result = readRequest_S5();
        if (CYCLE1_CONTINUE == result) // S5: "Se receber um sinal (...) retorna o valor CYCLE_CONTINUE"
            continue;                  // S5: "para que main() recomece automaticamente o CICLO1 no passo S5"
        if (RETURN_ERROR == result) terminateServidor_S7();
        // S6
        int pidServidorDedicado = createServidorDedicado_S6();
        if (pidServidorDedicado > 0)   // S6: "o processo Servidor (pai) (...) retorna um valor > 0"
            continue;                  // S6: "(...) recomeça o Ciclo1 no passo S4 (ou seja, volta a aguardar novo pedido)"
        if (RETURN_ERROR == pidServidorDedicado) terminateServidor_S7();
        // S6: "o Servidor Dedicado (...) retorna 0 para que main() siga automaticamente para o passo SD10

        // SD10
        if (RETURN_ERROR == triggerSignals_SD10()) terminateServidorDedicado_SD18();
        // SD11
        indexClient = searchClientDB_SD11();
        int erroValidacoes = TRUE;
        if (RETURN_ERROR != indexClient) {
            // SD12: "Se o passo SD11 concluiu com sucesso: (...)"
            indexFlight = searchFlightDB_SD12();
            if (RETURN_ERROR != indexFlight) {
                // SD13: "Se os passos SD11 e SD12 tiveram sucesso, (...)"
                if (!updateClientDB_SD13())
                    erroValidacoes = FALSE; // erroValidacoes = "houve qualquer erro nas validações dos passos SD11, SD12, ou SD13"
            }
        }
        // SD14: CICLO5
        int escolheuLugarDisponivel = FALSE;
        while (!escolheuLugarDisponivel) {
            // SD14.1: erroValidacoes = "houve qualquer erro nas validações dos passos SD11, SD12, ou SD13"
            if (RETURN_ERROR == sendResponseClient_SD14(erroValidacoes)) terminateServidorDedicado_SD18();
            if (erroValidacoes)
                terminateServidorDedicado_SD18();

            // SD15: "Se os pontos anteriores tiveram sucesso, (...)"
            if (RETURN_ERROR == readResponseClient_SD15()) terminateServidorDedicado_SD18();
            // SD16
            if (RETURN_ERROR == updateFlightDB_SD16())  // SD16: "Se lugarEscolhido no pedido NÃO estiver disponível (...) retorna erro (-1)"
                continue;                               // SD16: "para que main() recomece o CICLO5 em SD14"
            else
                escolheuLugarDisponivel = TRUE;
        }
        sendConfirmationClient_SD17();
        terminateServidorDedicado_SD18();
    }
}

/**
 *  "O módulo Servidor é responsável pelo processamento do check-in dos passageiros.
 *   Está dividido em duas partes, um Servidor (pai) e zero ou mais Servidores Dedicados (filhos).
 *   Este módulo realiza as seguintes tarefas:"
 */

/**
 * @brief S1: Ler a descrição da tarefa no enunciado
 * @return o valor de shmId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initShm_S1 () {
    shmId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //ver se temos acesso ao ficheiro passageiros
    if( access("bd_passageiros.dat", F_OK) == -1){
        so_error("S1.1","bd_passageiros.dat não esta na diretoria");
        return -1;
    }
    //ver se temos acesso ao ficheiro voos
    if( access("bd_voos.dat", F_OK) == -1){
        so_error("S1.1","bd_voos.dat não esta na diretoria");
        return -1;
    }

    //ver se conseguimos ler e escrever em ambos os fcheiros
    if( access("bd_passageiros.dat", R_OK) == -1){
        so_error("S1.1","bd_passageiros.dat leitura");
        return -1;
    }
    if( access("bd_voos.dat", R_OK) == -1){
        so_error("S1.1","bd_voos.dat leitura");
        return -1;
    }
    if( access("bd_passageiros.dat", W_OK) == -1){
        so_error("S1.1","bd_passageiros.dat escrita");
        return -1;
    }
    if( access("bd_voos.dat", W_OK) == -1){
        so_error("S1.1","bd_voos.dat escrita");
        return -1;
    }

    so_success("S1.1","sem erro nos ficheiros");

    //tenta ligar-se à memória partilhada
    int shmId = shmget(IPC_KEY, sizeof(database), 0 );
    if(shmId == -1){
        so_error("S1.2","erro ao criar a memoria");
        
        //ve se o erro é de nao existencia da memoria
        if(errno == ENOENT){
            so_success("S1.3","a memoria nao existe");

            shmId = shmget(IPC_KEY, sizeof(CheckIn) + sizeof(Voo), 0666 | IPC_CREAT | IPC_EXCL);
            if(shmId == -1){
                so_error("S1.4","erro ao criar a memoria");
                return -1;
            }else{
                //aponta a database para a memoria
                database = shmat(shmId, NULL, 0);
                if(database == NULL){
                    so_error("S1.4","erro no database");
                    return -1;
                }else{
                    so_success("S1.4","%d", shmId);
                }
            }
        }else{
            so_error("S1.3","Erro na memoria");
            return -1;
        }
    }else{
        so_success("S1.2","memoria criada");
        
        database = shmat(shmId, NULL, 0);
        if(database == NULL){
            so_error("S1.2.1","Erro no ponteiro");
            return -1;
        }else{
            so_success("S1.2.1","%d", shmId);
        }
    }

    // "limpa" a lista de passageiros e nrVoo
    for(int i = 0; i < MAX_PASSENGERS; i++){
        database->listClients[i].nif=PASSENGER_NOT_FOUND;
        strcpy(database->listFlights[i].nrVoo, FLIGHT_NOT_FOUND);
    }
    so_success("S1.5","limpeza feita");

    // abre o ficheiro passageiros para leitura
    FILE *f = fopen("bd_passageiros.dat", "r");
    if(f == NULL){
        so_error("S1.6","erro ao abrir o ficheiro");
        return -1;
    }
    
    //preenche os campos pidCliente e pidServidorDedicado
    int p = 0;
    while(fread(database, sizeof(DadosServidor), 1, f) == 1){
        database->listClients[p].pidCliente = PID_INVALID;
        database->listClients[p].pidServidorDedicado = PID_INVALID;
        p++;
        if(p >= MAX_PASSENGERS){
            break;
        }
    }
    so_success("S1.6","pids preenchidos");

    fclose(f);

    // abre o ficheiro voos para leitura
    FILE *fb = fopen("bd_voos.dat", "r");
    if(fb == NULL){
        so_error("S1.7","erro ao abrir o voos");
        return -1;
    }

    Voo a;
    // preenche os dados de variavel "a"
    for(int i = 0; i < MAX_FLIGHTS; i++){
       
        if(fread(&a, sizeof(Voo), 1, fb) != 1){
            break;
        }         

        strcpy(database->listFlights[i].nrVoo, a.nrVoo);
        strcpy(database->listFlights[i].origem, a.origem);
        strcpy(database->listFlights[i].destino, a.destino);
        
        for (int j = 0; j < MAX_SEATS; j++) {
            database->listFlights[i].lugares[j] = a.lugares[j];
        }
    }
    
    so_success("S1.7","voos preenchidos");

    fclose(fb);

    so_debug("> [@return:%d]", shmId);
    return shmId;
}

/**
 * @brief S2: Ler a descrição da tarefa no enunciado
 * @return o valor de msgId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initMsg_S2 () {
    msgId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //cria a memoria partilhada
    msgId = msgget(IPC_KEY, IPC_CREAT | 0666 );
    // caso ela exista apaga-a
    if(msgId > 0 ){
        int m = msgctl(msgId, IPC_RMID, NULL);
        if( m == -1){
            so_error("S2.1","Erro ao remover");
            return(-1);
        }else{
            so_success("S2.1","Sucesso ao remover");
        }
    }
    msgId = msgget(IPC_KEY, IPC_CREAT | 0666 );
    if( msgId < 0){
        so_error("S2.2","Erro ao criar");
        return(-1);
    }else{
        so_success("S2.2","%d", msgId);
    }


    so_debug("> [@return:%d]", msgId);
    return msgId;
}

/**
 * @brief S3: Ler a descrição da tarefa no enunciado
 * @return o valor de semId em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int initSem_S3 () {
    semId = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //cira o semaforo
    semId = semget(IPC_KEY, 3, IPC_CREAT | 0666);

    // vê se já existia
    if(semId > 0 ){
        if( semctl(IPC_KEY, 0, IPC_RMID) < 0){
            so_error("S3.1","erro ao remover");
            return(-1);
        }else{
            so_success("S3.1","sucesso ao remover");
        }
    }

    // cria o array de semaforos
    semId = semget(IPC_KEY, 3, IPC_CREAT | 0666);
    if( semId < 0){
        so_error("S3.2","erro ao criar");
        return(-1);
    }else{
        so_success("S3.2","%d", semId);
    }

    // altera os valores dos semaforos e testa se ocorreu algum erro
    int valueP = semctl(semId, SEM_PASSAGEIROS, SETVAL, 1);
    int valueV = semctl(semId, SEM_VOOS, SETVAL, 1);
    int valueD = semctl(semId, SEM_NR_SRV_DEDICADOS, SETVAL, 0);
    
    if( valueP < 0 || valueV < 0 || valueD < 0){
        so_error("S3.3","erro ao mudar o valor");
        return(-1);
    }else{
        so_success("S3.3","%d", semId);
    }

    so_debug("> [@return:%d]", semId);
    return semId;
}

/**
 * @brief S4: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_S4 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // trata os sinais e vê se aconteceu algum erro
    if(signal(SIGINT, trataSinalSIGINT_S8) == SIG_ERR || signal(SIGCHLD, trataSinalSIGCHLD_S9)){
        so_error("S4","erro ao armar");
        return(-1);
    }else{
        so_success("S4","sucesso ao armar");
        return(0);
    }

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief S5: O CICLO1 já está a ser feito na função main(). Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readRequest_S5 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // redireciona caso receba este sinal
    signal(SIGCHLD, trataSinalSIGCHLD_S9);

    //lê uma mensagem
    result = msgrcv(msgId, &clientRequest, sizeof(clientRequest.msgData), MSGTYPE_LOGIN, 0);

    //vê se aconteceu algum erro
    if(result < 0){
        if(errno == EINTR){
            return CICLO1_CONTINUE;
        }else{
            so_error("S5","Erro ao ler a mensagem");
            return -1;
        }
    }

    so_success("S5","%d %s %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.senha, clientRequest.msgData.infoCheckIn.pidCliente );
    return 0; 
    
    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief S6: Ler a descrição da tarefa no enunciado
 * @return PID do processo filho, se for o processo Servidor (pai),
 *         0 se for o processo Servidor Dedicado (filho),
 *         ou PID_INVALID (-1) em caso de erro
 */
int createServidorDedicado_S6 () {
    int pid_filho = PID_INVALID;    // Por omissão retorna erro
    so_debug("<");

    // faz um fork do processo
   pid_filho = fork();

    //ve se aconteceu algum erro
   if(pid_filho < 0){
        so_error("S6","erro no fork");
        return(-1);
    }

    //escreve no ficheiro filho
    if( pid_filho == 0){
        so_success("S6","Servidor Dedicado: Nasci");
        return 0;
    }else{
        // escerve no ficheiro pai
        so_success("S6","Servidor: Iniciei SD %d", pid_filho);
        nrServidoresDedicados++;
        return pid_filho;
    }

    so_debug("> [@return:%d]", pid_filho);
    return pid_filho;
}

/**
 * @brief S7: Ler a descrição da tarefa no enunciado
 */
void terminateServidor_S7 () {
    so_debug("<");

    so_success("S7","Servidor: Start Shutdown");

    //ve se a memoria partilhada existe e se está apontada para a database
    if( shmId < 0 || database == NULL){
        so_error("S7.1","erro ao aceder a shm");
    }else{
        so_success("S7.1","shm acedido com sucesso");
        
        //percorre a lista de passageiros e cada elemento que tiver um pidServidorDedicado valido envia um sinal
        for(int i = 0; i < MAX_PASSENGERS; i++){
            if(database->listClients[i].pidServidorDedicado > 0){
                kill(database->listClients[i].pidServidorDedicado, SIGUSR2);
                so_success("S7.2","Servidor: Shutdown SD %d", database->listClients[i].pidServidorDedicado);
            }
        }

        // estruturas dos semafros
        struct sembuf S = {
            .sem_num = SEM_NR_SRV_DEDICADOS, 
            .sem_op = -nrServidoresDedicados,
            .sem_flg = 0
        };
        //altera o valor do semaforo
        semop(semId, &S, 1);
        so_success("S7.3","sem tratados");

        // abre o ficheiro passageiro para escrita
        FILE *fp = fopen("bd_passageiros.dat","w");
        if(fp == NULL){
            so_error("S7.4","erro no ficheiro passageiros");
        }

        //escreve os dados do cliente no ficheiro
        fwrite(database->listClients, sizeof(CheckIn), MAX_PASSENGERS, fp);
        fclose(fp);
        
        //abre o ficheiro voos para escrita
        FILE *fv = fopen("bd_voos.dat", "w");
        if(fv == NULL){
            so_error("S7.4","erro no ficheiro voos");
        }

        //escreve os dados do Voo no ficheiro
        fwrite(database->listFlights, sizeof(Voo), MAX_FLIGHTS, fv);
        fclose(fv);

        so_success("S7.4","passageiros e voos alterado");
      
    }

    //apaga a memoria partilhada, os semaforos e a mensagem
    semctl(semId, 0, IPC_RMID);
    shmctl(shmId, IPC_RMID, NULL);
    msgctl(msgId, IPC_RMID, NULL);

    so_success("S7.5","Servidor: End Shutdown");

    so_debug(">");
    exit(0);
}

/**
 * @brief S8: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_S8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    //chama a função terminateServidor_S7
    so_success("S8","sinal recebido");
    terminateServidor_S7();

    so_debug(">");
}

/**
 * @brief S9: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGCHLD_S9 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

  
    //estrutura dos semaforos
    struct sembuf UP = { 
    .sem_num = 2, 
    .sem_op = +1,
    .sem_flg = 0
    };

    // aguardamos que um processo filho termine
    pid_t pid;
    int s;
    while((pid = wait(&s)) > 0){
        so_success("S9","Servidor: Confirmo fim de SD %d", pid);
        semop(semId, &UP, 1);
        return;
    }

    so_debug(">");
}

/**
 * @brief SD10: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int triggerSignals_SD10 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // arma os sinais para as respetivas funções
    void (*hand)(int);
    hand = signal(SIGINT, SIG_IGN);
     

    if(signal(SIGUSR1, trataSinalSIGUSR1_SD19) == SIG_ERR || signal(SIGUSR2, trataSinalSIGUSR2_SD20) == SIG_ERR || signal(SIGINT, hand) == SIG_ERR ){
        so_error("SD10","erro ao armar os sinais");
        return -1;
    }else{
        so_success("SD10","sinais armados");
        return 0;
    }

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD11: Ler a descrição da tarefa no enunciado
 * @return indexClient em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int searchClientDB_SD11 () {
    indexClient = -1;    // SD11:"inicia a variável indexClient a -1"
    so_debug("<");

    //procura um passageiro com o Nif do pedido e compara as senhas
    for(int i = 0; i < MAX_PASSENGERS; i++){
        if(database->listClients[i].nif == clientRequest.msgData.infoCheckIn.nif){
            if(strcmp(database->listClients[i].senha, clientRequest.msgData.infoCheckIn.senha) == 0){
                indexClient = i;
                so_success("SD11.3","%d", indexClient);
                return indexClient;
            }else{
                so_error("SD11.3", "Cliente %d: Senha errada", clientRequest.msgData.infoCheckIn.nif);
                return -1;
            }
        }
    }
    so_error("SD11.1","Cliente %d: não encontrado", clientRequest.msgData.infoCheckIn.nif);

    so_debug("> [@return:%d]", indexClient);
    return indexClient;
}

/**
 * @brief SD12: Ler a descrição da tarefa no enunciado
 * @return indexFlight em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int searchFlightDB_SD12 () {
    indexFlight = -1;    // SD11:"inicia a variável indexFlight a -1"    so_debug("<");
    so_debug("<");

    
        //procura um nrVoo do passageiros indexClient e comparamos esses campos 
        for(int i = 0; i < MAX_FLIGHTS; i++){
            if(database->listFlights[i].nrVoo == database->listFlights[indexClient].nrVoo){
                if(strcmp(database->listFlights[i].nrVoo, database->listClients[i].nrVoo ) == 0){
                    indexFlight = i;
                    so_success("SD12.2", "%d", indexFlight);
                    return indexFlight;
                }
            }
        }
        so_error("SD12.1","Voo %s: não encontrado", database->listClients[indexClient].nrVoo);
        return -1;
    

    so_debug("> [@return:%d]", indexFlight);
    return indexFlight;
}

/**
 * @brief SD13: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int updateClientDB_SD13 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    // estruturas dos semaforos
    struct sembuf DWON = { 
        .sem_num = SEM_PASSAGEIROS, 
        .sem_op = -1,
        .sem_flg = 0
    };

    struct sembuf UP = { 
        .sem_num = SEM_PASSAGEIROS, 
        .sem_op = +1,
        .sem_flg = 0
    };

    //diminuimos o valor do semaforo
    if(semop(semId, &DWON, 1) < 0){
        so_error("SD13.3","erro ao reduzir o valor do sem");
        return -1;
    }
    
    so_success("SD13.1","Start Check-in: %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.pidCliente);

    //vemos se o elemento pidCliente e lugarEscolhido são válidos
    if(database->listClients[indexClient].pidCliente != PID_INVALID || database->listClients[indexClient].lugarEscolhido != EMPTY_SEAT){
        
        //aumentamos o valor do semaforo
        if(semop(semId, &UP, 1) < 0){
            so_error("SD13.3","erro ao aumentar o valor do sem");
            return -1;
        }
        
        so_error("SD13.2","Cliente %d: Já fez check-in", clientRequest.msgData.infoCheckIn.nif);
        return -1;
    }

    //"adormece 4 segundos"
    sleep(4);

    //preenche o pidCliente e pidServidorDedicado
    database->listClients[indexClient].pidCliente = clientRequest.msgData.infoCheckIn.pidCliente;
    database->listClients[indexClient].pidServidorDedicado = getpid();

    //aumenta o valor so semaforo
    if(semop(semId, &UP, 1) < 0){
        so_error("SD13.3","erro ao aumentar o valor do sem");
        return -1;
    }

    so_success("SD13.5","End Check-in: %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.pidCliente);
    
    return 0;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD14: Ler a descrição da tarefa no enunciado
 * @param erroValidacoes booleano que diz se houve algum erro nas validações de SD11, SD12 e SD13
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendResponseClient_SD14 (int erroValidacoes) {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //define o tipo da mensage
    clientRequest.msgType = clientRequest.msgData.infoCheckIn.pidCliente; 
    
    //vê se aos alineas anteriores estão validas
    if(erroValidacoes){
        so_error("SD14.1", "erro nas anteriores");
        clientRequest.msgData.infoCheckIn.pidServidorDedicado = PID_INVALID;
    }else{
        //preenche os dados do CheckIn
        so_success("SD14.1", "anteriores sem erros");
        clientRequest.msgData.infoVoo = database->listFlights[indexFlight];
        clientRequest.msgData.infoCheckIn = database->listClients[indexClient];
        clientRequest.msgData.infoCheckIn.lugarEscolhido = EMPTY_SEAT;
        clientRequest.msgData.infoCheckIn.pidServidorDedicado = getpid(); 
    }

    //envia o pedido para a MSG e vê se aconteceu algum erro
    if (msgsnd(msgId, &clientRequest, sizeof(clientRequest.msgData), 0) != -1) {
        so_success("SD14.2", "");
        return 0;        
    }else{
        so_error("SD14.2", "erro ao enviar a mensagem");
        return -1;
    }
    
    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD15: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int readResponseClient_SD15 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //recebe uma mensagem e vê se aconteceu algum erro
    ssize_t size = msgrcv(msgId, &clientRequest, sizeof(clientRequest.msgData), getpid(), 0);
    if ( size < 0 ) {
       so_error("SD15","erro ao ler");
       return -1;
    }

    so_success("SD15","%d %d %d", clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido, clientRequest.msgData.infoCheckIn.pidCliente);
    return 0;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD16: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int updateFlightDB_SD16 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //estrutura do semaforo
    so_success("SD16.1", "Start Reserva lugar: %s %d %d", database->listFlights[indexFlight].nrVoo, clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido);
    struct sembuf DWON = { 
        .sem_num = SEM_VOOS, 
        .sem_op = -1,
        .sem_flg = 0
    };

    struct sembuf UP = { 
        .sem_num = SEM_VOOS, 
        .sem_op = +1,
        .sem_flg = 0
    };

    //diminui o valor do semaforo
    if (semop(semId, &DWON, 1) < 0) {
        so_error("SD16", "erro ao reduzir o valor do sem");
        return -1;
    }

    // vê se o lugar escolhido está livre
    if (database->listFlights[indexFlight].lugares[clientRequest.msgData.infoCheckIn.lugarEscolhido] != EMPTY_SEAT) {

        if (semop(semId, &UP, 1) < 0) {
            so_error("SD16", "erro ao aumentar o valor do sem");
        }
        so_error("SD16.2", "Cliente %d: Lugar já estava ocupado", clientRequest.msgData.infoCheckIn.nif);
        return -1;
    }

    //"adormece 4 segundos"
    sleep(4);

    //preenche os campos lugares[] e lugarEscolhido
    database->listFlights[indexFlight].lugares[clientRequest.msgData.infoCheckIn.lugarEscolhido] = clientRequest.msgData.infoCheckIn.nif;
    database->listClients[indexClient].lugarEscolhido = clientRequest.msgData.infoCheckIn.lugarEscolhido;

    //aumenta o valor do semaforo
    if (semop(semId, &UP, 1) < 0) {
        so_error("SD16", "erro ao aumentar o valor do sem");
        return -1;
    }


    so_success("SD16.6", "End Reserva lugar: %s %d %d", database->listFlights[indexFlight].nrVoo, clientRequest.msgData.infoCheckIn.nif, clientRequest.msgData.infoCheckIn.lugarEscolhido);
    return 0;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD17: Ler a descrição da tarefa no enunciado
 * @return RETURN_SUCCESS (0) em caso de sucesso, ou RETURN_ERROR (-1) em caso de erro
 */
int sendConfirmationClient_SD17 () {
    int result = RETURN_ERROR; // Por omissão, retorna erro
    so_debug("<");

    //preenche a mensagem "m"
    MsgContent m;
    m.msgType = clientRequest.msgData.infoCheckIn.pidCliente;
    m.msgData.infoCheckIn.pidServidorDedicado = getpid();
    m.msgData.infoCheckIn.lugarEscolhido = clientRequest.msgData.infoCheckIn.lugarEscolhido;
    strcpy(m.msgData.infoVoo.origem, clientRequest.msgData.infoVoo.origem);
    strcpy(m.msgData.infoVoo.destino, clientRequest.msgData.infoVoo.destino);

    //envia essa mensagem e vê se aconteceu algum erro
    int status = msgsnd(msgId, &m, sizeof(m.msgData), 0);
    if ( status < 0 ) {
        so_error("SD17","Erro ao enviar");
        return -1;
    }

    so_success("SD17","mensagem enviada");
    return 0;

    so_debug("> [@return:%d]", result);
    return result;
}

/**
 * @brief SD18: Ler a descrição da tarefa no enunciado
 */
void terminateServidorDedicado_SD18 () {
    so_debug("<");

    //vê se o elemento IndexClient é válido e preenche os elementos pidCliente e pidServidorDedicado
    if(indexClient >= 0){
        database->listClients[indexClient].pidCliente = PID_INVALID;
        database->listClients[indexClient].pidServidorDedicado = PID_INVALID;
    }
    so_success("SD18","Terminar servidor dedicado");
    exit(0);

    so_debug(">");
    exit(0);
}

/**
 * @brief SD19: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR1_SD19 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    //chama a funçao terminateServidorDedicado_SD18
    so_success("SD19","SD: Recebi pedido do Cliente para terminar");
    terminateServidorDedicado_SD18();

    so_debug(">");
}

/**
 * @brief SD20: Ler a descrição da tarefa no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR2_SD20 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    //Caso o pidClient seja valido envia um sinal 
    so_success("SD20","SD: Recebi pedido do Servidor para terminar");
    if(clientRequest.msgData.infoCheckIn.pidCliente != PID_INVALID){
        kill(clientRequest.msgData.infoCheckIn.pidCliente, SIGHUP);
    }

    terminateServidorDedicado_SD18();


    so_debug(">");
}