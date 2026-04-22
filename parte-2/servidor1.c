/******************************************************************************
 ** ISCTE-IUL: Trabalho prático 2 de Sistemas Operativos 2023/2024, Enunciado Versão 3+
 **
 ** Aluno: Nº:       Nome: 
 ** Nome do Módulo: servidor.c
 ** Descrição/Explicação do Módulo:
 **
 **
 ******************************************************************************/

// #define SO_HIDE_DEBUG                // Uncomment this line to hide all @DEBUG statements
#include "common.h"

/*** Variáveis Globais ***/
CheckIn clientRequest; // Variável que tem o pedido enviado do Cliente para o Servidor

/**
 * @brief Processamento do processo Servidor e dos processos Servidor Dedicado
 *        "os alunos não deverão alterar a função main(), apenas compreender o que faz.
 *         Deverão, sim, completar as funções seguintes à main(), nos locais onde está claramente assinalado
 *         '// Substituir este comentário pelo código da função a ser implementado pelo aluno' "
 */
int main () {
    // S1
    checkExistsDB_S1(FILE_DATABASE);
    // S2
    createFifo_S2(FILE_REQUESTS);
    // S3
    triggerSignals_S3(FILE_REQUESTS);

    int indexClient;       // Índice do cliente que fez o pedido ao servidor/servidor dedicado na BD

    // S4: CICLO1
    while (TRUE) {
        // S4
        clientRequest = readRequest_S4(FILE_REQUESTS); // S4: "Se houver erro (...) clientRequest.nif == -1"
        if (clientRequest.nif < 0)   // S4: "Se houver erro na abertura do FIFO ou na leitura do mesmo, (...)"
            continue;                // S4: "(...) e recomeça o Ciclo1 neste mesmo passo S4, lendo um novo pedido"

        // S5
        int pidServidorDedicado = createServidorDedicado_S5();
        if (pidServidorDedicado > 0) // S5: "o processo Servidor (pai) (...)"
            continue;                // S5: "(...) recomeça o Ciclo1 no passo S4 (ou seja, volta a aguardar novo pedido)"
        // S5: "o Servidor Dedicado (que tem o PID pidServidorDedicado) segue para o passo SD9"

        // SD9
        triggerSignals_SD9();
        // SD10
        CheckIn itemBD;
        indexClient = searchClientDB_SD10(clientRequest, FILE_DATABASE, &itemBD);
        // SD11
        checkinClientDB_SD11(&clientRequest, FILE_DATABASE, indexClient, itemBD);
        // SD12
        sendAckCheckIn_SD12(clientRequest.pidCliente);
        // SD13
        closeSessionDB_SD13(clientRequest, FILE_DATABASE, indexClient);
        so_exit_on_error(-1, "ERRO: O servidor dedicado nunca devia chegar a este ponto");
    }
}

/**
 *  "O módulo Servidor é responsável pelo processamento do check-in dos passageiros. 
 *   Está dividido em duas partes, um Servidor (pai) e zero ou mais Servidores Dedicados (filhos).
 *   Este módulo realiza as seguintes tarefas:"
 */

/**
 * @brief S1     Ler a descrição da tarefa S1 no enunciado
 * @param nameDB O nome da base de dados (i.e., FILE_DATABASE)
 */
void checkExistsDB_S1 (char *nameDB) {
    so_debug("< [@param nameDB:%s]", nameDB);

    // ver se o ficheiro existe
    if ( nameDB == NULL) {
        so_error("S1", "Erro na existencia do ficheiro");
        exit(1);
    }

    //ver se temos acesso ao ficheiro
    if(access(nameDB, F_OK) == -1){
        so_error("S1", "Erro no acesso ao ficheiro");
        exit(1);
    }

    // ver se podemos escrever no ficheiro
    if( access(nameDB, W_OK) == -1){
        so_error("S1", "Erro na escrita");
        exit(1);
    }

    // ver se podemos ler do ficheiro
    if( access(nameDB, R_OK) == -1){
        so_error("S1", "Erro na leitura");
        exit(1);
    }
    so_success("S1", "sem erros");

    so_debug(">");
}

/**
 * @brief S2       Ler a descrição da tarefa S2 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 */
void createFifo_S2 (char *nameFifo) {
    so_debug("< [@param nameFifo:%s]", nameFifo);

    // ver se temos acesso ao fifo, caso sim, apagamos esse fifo e vemos se tudo correu bem
    if(access(nameFifo, F_OK) == 0 ){
        if (unlink(nameFifo) != 0 ) {
            so_error("S2","erro ao remover");
            exit(1);
        }
    }

    // criamos um fifo e vemos se correu bem
    if (mkfifo(nameFifo, 0666) != 0) {
        so_error("S2","erro ao criar");
        exit(1);
    }

    so_success("S2","Fifo criado com sucesso");

    so_debug(">");
}

/**
 * @brief S3   Ler a descrição da tarefa S3 no enunciado
 */
void triggerSignals_S3 () {
    so_debug("<");

    // armamento de sinais
    signal(SIGINT, trataSinalSIGINT_S6);
    signal(SIGCHLD, trataSinalSIGCHLD_S8);

    // verifica se ocorreu algum erro ao armar
    if(signal(SIGINT, trataSinalSIGINT_S6) == SIG_ERR){
        so_error("S3","Erro ao armar");
        deleteFifoAndExit_S7();
    }
    if(signal(SIGINT, trataSinalSIGCHLD_S8) == SIG_ERR){
        so_error("S3","Erro ao armar");
        deleteFifoAndExit_S7();
    }

    so_success("S3","Sinais bem armados");

    so_debug(">");
}

/**
 * @brief S4       O CICLO1 já está a ser feito na função main(). Ler a descrição da tarefa S4 no enunciado
 * @param nameFifo O nome do FIFO do servidor (i.e., FILE_REQUESTS)
 * @return CheckIn Elemento com os dados preenchidos. Se nif=-1, significa que o elemento é inválido
 */
CheckIn readRequest_S4 (char *nameFifo) {
    CheckIn request;
    request.nif = -1;   // Por omissão retorna erro
    so_debug("< [@param nameFifo:%s]", nameFifo);

    int a = 0;

    FILE *f = fopen(nameFifo, "r");

    // ver se o fifo existe
    if(f == NULL){
        so_error("S4","erro ao abrir o fifo");
        deleteFifoAndExit_S7();
    }

    // lê do fifo e vê se correu tudo bem
    if(fread(&request.nif, sizeof(request), 1, f) <= 0){
        so_error("S4.1","erro ao ler");
        deleteFifoAndExit_S7();
    }

    // vê se o nif têm 9 algarismos
    while(request.nif > 0){
        request.nif = request.nif / 10;
        a++;
    }
   
    // vê se o nif e o pidCliente são iguas
    if(a <= 0 || request.pidCliente <= 0){
        so_error("S4.1","nif ou pid invalidos");
        deleteFifoAndExit_S7();
    }

    fclose(f);

    so_success("S4","%d %s %d", request.nif, request.senha, request.pidCliente);

    so_debug("> [@return nif:%d, senha:%s, pidCliente:%d]", request.nif, request.senha, request.pidCliente);
    return request;
}

/**
 * @brief S5   Ler a descrição da tarefa S5 no enunciado
 * @return int PID do processo filho, se for o processo Servidor (pai),
 *             0 se for o processo Servidor Dedicado (filho), ou -1 em caso de erro.
 */
int createServidorDedicado_S5 () {
    int pid_filho = -1;    // Por omissão retorna erro
    so_debug("<");

    // usamos a funcao fork()    
    pid_filho = fork();

    // vemos se ocorreu algum erro
    if( pid_filho < 0){
        so_error("S5","erro no fork na criação");
        deleteFifoAndExit_S7();

    }else{

        // vemos se estamos no processo pai ou filho
        if(pid_filho == 0){
            triggerSignals_SD9();
        }else{
            so_success("S5","Servidor: Iniciei SD %d", pid_filho);
            triggerSignals_SD9();
           
            
        }
    }

    
    return pid_filho;

    so_debug("> [@return:%d]", pid_filho);
    return pid_filho;
}

/**
 * @brief S6            Ler a descrição das tarefas S6 e S7 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGINT_S6 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    so_success("S6","Servidor: Start Shutdown");

    // abrimos o ficheiro em modo leitura
    FILE *f = fopen("bd_passageiros.dat", "r");

    // vemos se o ficherio foi aberto com sucesso
    if(f == NULL){
        so_error("S6.1","erro ao abrir ficheiro");
        deleteFifoAndExit_S7();
    }else{
        so_success("S6.1","Ficheiro aberto com sucesso");
    }

    // criamos uma variavel do tipo CheckIN
    CheckIn c;
     
    // enquanto nao chegarmos ao fim do ficheiro
    while(!feof(f)){

        // lemos um elemento CheckIN 
        if(fread(&c, sizeof(c), 1, f) == 0){

            // verificamos se já chegamos ao fim do ficheiro
            if(feof(f)){
                so_success("S6.2","Nao á mais nada para ler");
                deleteFifoAndExit_S7();
            }else{
                so_error("S6.2","Erro ou nao ha nada para ler");
                deleteFifoAndExit_S7();
            }
        }else{

            // vemos se o pid do servidor dedicado é positivo
            if(c.pidServidorDedicado > 0){
                kill(c.pidServidorDedicado, SIGUSR2);
                so_success("S6.3","Servidor: Shutdown SD %d", c.pidServidorDedicado);
            }
        }
    }

    fclose(f);

    so_debug(">");
}

/**
 * @brief S7 Ler a descrição da tarefa S7 no enunciado
 */
void deleteFifoAndExit_S7 () {
    so_debug("<");

    #define fifo "server.fifo"

    // apagamos o fifo e vemos se ocorreu algum erro
    if(unlink(fifo) == -1){
        so_error("S7","nao consegui apagar");
        exit(1);
    }else{
        so_success("S7","Servidor: End Shutdown");
    }

    so_debug(">");
    exit(0);
}

/**
 * @brief S8            Ler a descrição da tarefa S8 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGCHLD_S8 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

    
pid_t pid;
int s;

// aguardamos que um processo filho termine
while((pid = wait(&s)) > 0){
    so_success("S8", "Servidor: Confirmo fim de SD %d", pid);
    return;
}
     

    so_debug(">");
}

/**
 * @brief SD9  Ler a descrição da tarefa SD9 no enunciado
 */
void triggerSignals_SD9 () {
    so_debug("<");

    // criamos uma variavel void
    void (*hand)(int);

    // vemos se o sinal foi armado com sucesso
    if(signal(SIGUSR2, trataSinalSIGUSR2_SD14) == SIG_ERR){
        so_error("SD9","Erro ao armar o sinal SIGUSR2");
        exit(1);
    }

    // tratamos do sinal SIGINT
    hand = signal(SIGINT, SIG_IGN);
    signal(SIGINT, hand);


    so_success("SD9","Sinais armados");

    so_debug(">");
}

/**
 * @brief SD10    Ler a descrição da tarefa SD10 no enunciado
 * @param request O pedido do cliente
 * @param nameDB  O nome da base de dados
 * @param itemDB  O endereço de estrutura CheckIn a ser preenchida nesta função com o elemento da BD
 * @return int    Em caso de sucesso, retorna o índice de itemDB no ficheiro nameDB.
 */
int searchClientDB_SD10 (CheckIn request, char *nameDB, CheckIn *itemDB) {
    int indexClient = 0;    // SD10:"inicia uma variável indexClient a 0"
    so_debug("< [@param request.nif:%d, request.senha:%s, nameDB:%s, itemDB:%p]", request.nif,
                                                                    request.senha, nameDB, itemDB);
    FILE *f = fopen(nameDB, "r");

    int a = 0;
    

    if(nameDB == NULL){
        so_error("SD10","Ficheiro com erro");
        exit(1);
    }else{
        so_success("SD10","ficheiro aberto com sucesso");

        // lê o ficheiro e incrementa uma unidade á variavel indexCliente
        while(fread(&request, sizeof(request),1,f) == 1){       
            indexClient++;

            // vê se os nifs e as senhas são iguais
            if(request.nif == itemDB->nif){
                if(request.senha == itemDB->senha){
                    so_success("SD10.3","%d", indexClient);
                    a++;
                    break;
                    fclose(f);
                }else{
                    so_error("SD10.3","Cliente %d: Senha errada", request.nif);
                    kill(getpid(),SIGHUP);
                    exit(1);
                }
            }
        }
    }
    if(a==0){
        so_error("SD10.1","Cliente %d: não encontrado", request.nif);
        kill(getpid(),SIGHUP);
        exit(1);
    }
 
    fclose(f);


    so_debug("> [@return:%d, nome:%s, nrVoo:%s]", indexClient, itemDB->nome, itemDB->nrVoo);
    return indexClient;
}

/**
 * @brief SD11        Ler a descrição da tarefa SD11 no enunciado
 * @param request     O endereço do pedido do cliente (endereço é necessário pois será alterado)
 * @param nameDB      O nome da base de dados
 * @param indexClient O índica na base de dados do elemento correspondente ao cliente
 * @param itemDB      O elemento da BD correspondente ao cliente
 */
void checkinClientDB_SD11 (CheckIn *request, char *nameDB, int indexClient, CheckIn itemDB) {
    so_debug("< [@param request:%p, nameDB:%s, indexClient:%d, itemDB.pidServidorDedicado:%d]",
                                    request, nameDB, indexClient, itemDB.pidServidorDedicado);

    FILE *f = fopen(nameDB, "r+");

    // vê se o ficheiro foi aberto com sucesso
    if(f == NULL){
        so_error("SD11.2","erro ao abrir o ficheiro");
        kill(request->pidServidorDedicado, SIGHUP);
        exit(1);
        
    }else{
        so_success("SD11.2","ficheiro aberto com sucesso");
    }

    // coloca o ponteiro no devido sitio e testa se algo correu mal
    if( fseek(f, indexClient *sizeof(request), SEEK_SET) != 0 ){
        so_error("SD11.3","fseek com eror");
        kill(request->pidServidorDedicado, SIGHUP);
        exit(1);
    }else{
        so_success("SD11.3","fseek bem sucedido");
    }

    // escreve e testa se algo correu mal
    if(fwrite(&request, sizeof(request), 1, f) != 1){
        so_error("SD11.4","erro na escrita");
        kill(request->pidServidorDedicado, SIGHUP);
        fclose(f);
        exit(1);
    }else{
        so_success("SD11.4","escrita bem sucedida");
    }

    fclose(f);

    so_success("SD11.1","%s %s %d",request->nome, request->nrVoo, request->pidServidorDedicado);


    so_debug("> [nome:%s, nrVoo:%s, pidServidorDedicado:%d]", request->nome,
                                                request->nrVoo, request->pidServidorDedicado);
}

/**
 * @brief SD12       Ler a descrição da tarefa SD12 no enunciado
 * @param pidCliente PID (Process ID) do processo Cliente
 */
void sendAckCheckIn_SD12 (int pidCliente) {
    so_debug("< [@param pidCliente:%d]", pidCliente);

    srand(time(NULL));

    // criamos uma variavel t com a funçao rand()
    int t = rand() % MAX_ESPERA + 1;
    so_success("SD12","%d", t);

    // esperamos que esse "t" passe
    sleep(t);
    kill(pidCliente, SIGUSR1);

    so_debug(">");
}

/**
 * @brief SD13          Ler a descrição da tarefa SD13 no enunciado
 * @param clientRequest O endereço do pedido do cliente
 * @param nameDB        O nome da base de dados
 * @param indexClient   O índica na base de dados do elemento correspondente ao cliente
 */
void closeSessionDB_SD13 (CheckIn clientRequest, char *nameDB, int indexClient) {
    so_debug("< [@param clientRequest:%p, nameDB:%s, indexClient:%d]", &clientRequest, nameDB,
                                                                                    indexClient);

    FILE *fb = fopen(nameDB, "r+");
    if(fb == NULL){
        so_error("SD13.1","erro ao abrir");
        exit(1);
    }else{
        so_success("SD13.1","ficheiro aberto com sucesso");
    }

    if(fseek(fb, indexClient *sizeof(CheckIn), SEEK_SET) != 0){
        so_error("SD13.2","erro no fseek");
    }else{
        so_success("SD13.2","fseek bem sucedido");
    }

    if(fwrite(&clientRequest, sizeof(clientRequest), 1, fb) != 0){
        so_success("SD13.3","escrita com sucesso");
        fclose(fb);
        exit(1);
    }else{
        so_error("SD13.3","erro ao escrever");
        exit(1);
    }
 
    so_debug("> [pidCliente:%d, pidServidorDedicado:%d]", clientRequest.pidCliente, 
                                                          clientRequest.pidServidorDedicado);
}

/**
 * @brief SD14          Ler a descrição da tarefa SD14 no enunciado
 * @param sinalRecebido nº do Sinal Recebido (preenchido pelo SO)
 */
void trataSinalSIGUSR2_SD14 (int sinalRecebido) {
    so_debug("< [@param sinalRecebido:%d]", sinalRecebido);

     so_success("SD14", "SD: Recebi pedido do Servidor para terminar");
     exit(0);

    so_debug(">");
}