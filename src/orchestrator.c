#include "../include/messages.h"
#include "../include/utils.h"


int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        write_file(STDOUT_FILENO, "usage: ./orchestrator output_folder parallel-tasks sched-policy <test-mode>\n", 77);
        exit(EXIT_FAILURE);
    }

    // inicialização e verificação dos argumentos que constam na execução do programa
    char folder_path[50];
    if (sprintf(folder_path, "tmp/%s", argv[1]) < 0) {
        perror("[ERROR 18] sprintf:");
        exit(EXIT_FAILURE);
    }
    int parallel_tasks = atoi(argv[2]);
    SCHED_POLICY sched_policy;
    if (strcmp(argv[3], "FCFS") == 0)
        sched_policy = FCFS;
    else if (strcmp(argv[3], "SJF") == 0)
        sched_policy = SJF;
    else {
        perror("Incorrect schedule policy. Usage: FCFS / SJF");
        exit(EXIT_FAILURE);
    }

    int is_testing = -1;
    if (argv[4] != NULL) {
        if (strcmp(argv[4], "test-mode") == 0) {
            is_testing = NUM_TESTS - 1;
            write_file(STDOUT_FILENO, "Test mode enabled\n", 19);
        } else {
            perror("Incorrect test-mode. Usage: test-mode");
            exit(EXIT_FAILURE);
        }
    }

    // cria, caso não exista, a pasta definida pelo utilizador
    create_folder(folder_path);

    // cria fifo para receber mensagens
    make_fifo(MAIN_FIFO_NAME);

    // imprime no ecrã informação de debugging
    write_file(STDOUT_FILENO, "Server is running...\n", 22);

    // abre fifo em modo leitura
    int incoming_fd = open_file(MAIN_FIFO_NAME, O_RDONLY, 0);
    // abre outro fifo em modo write, para posteriormente haver bloqueio no read
    int aux_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);

    // inicialização da contagem do tempo, caso a opção "test-mode" tenha sido usada
    struct timeval time_before, time_after;
    if (is_testing > 0) {
        if (gettimeofday(&time_before, NULL) < 0) {
            perror("[ERROR 19] gettimeofday:");
            exit(EXIT_FAILURE);
        }
    }

    // criação e abertura do ficheiro partilhado que terá os IDs e tempos de execução das tarefas concluídas
    char shared_file_path[70];
    if (sprintf(shared_file_path, "%s/tasks_info.bin", folder_path) < 0) {
        perror("[ERROR 20] sprintf:");
        exit(EXIT_FAILURE);
    }

    // inicialização de outrass variáveis
    Msg message_received;
    Msg_list messages_list;
    create_messages_list(&messages_list, parallel_tasks);

    /* o servidor encontrar-se-á permanentemente neste ciclo while. apenas
       quando uma mensagem do tipo STOP tenha sido enviada pelo cliente, ou,
       quando em "test-mode", as 20 tarefas tenham sido executadas, é que o
       servidor procede com o encerramento do programa */
    while (read_file(incoming_fd, &message_received, sizeof(Msg)) && (message_received.type != STOP) && is_testing != 0) {
        if (message_received.type == STATUS)
            send_status_to_client(messages_list, message_received.pid, shared_file_path);
        else {
            // pai recolhe o processo filho terminado, responsável pela execução da tarefa
            if (message_received.type == COMPLETED) {
                waitpid(message_received.child_pid, NULL, WUNTRACED);

                // remoção da tarefa da lista de tarefas em execução
                delete_from_executing_messages_list(&messages_list, message_received.pid);

                // quando em "test-mode", decrementa-se o número de programas restantes a
                // executar por 1
                if (is_testing > 0) is_testing--;
            }

            // trata-se de uma tarefa a executar
            else {
                // envia-se o número do tarefa através do fifo criado pelo cliente
                send_task_number_to_client(message_received.pid);

                // inserir a tarefa na lista de tarefas agendadas
                insert_scheduled_messages_list(&messages_list, message_received);
            }

            /**
             * caso a política de escalonamento do servidor tenha sido selecionada como 
             * sendo SJF, ordena-se a lista de tarefas agendadas de acordo com o tempo
             * exepctável de execução. caso contrário, trata-se da política FCFS, onde
             * não é necessário haver uma ordenação da lista em questão
             * */
            if (sched_policy == SJF) sort_by_SJF(messages_list.scheduled_messages, messages_list.scheduled_messages_size);

            /**
             * esta função é executada por duas razões: uma tarefa da lista de tarefas a
             * executar foi concluída, havendo espaço para uma nova tarefa ser executada;
             * chegou ao servidor uma nova tarefa. em ambos os casos, é necessário ver
             * qual a próxima tarefa a executar. caso os critérios para uma executar nova
             * tarefa não tenham sido cumpridos, é devolvida uma mensagem vazia com o tipo
             * ERR.
             * */
            Msg msg_to_execute = get_next_executing_message(&messages_list);

            // se existe uma mensagem a executar
            if (msg_to_execute.type != ERR) {
                // fork para libertar o pai
                if (fork() == 0) {
                    // execução geral do programa da tarefa em questão
                    long time_spent = parse_and_execute_message(&msg_to_execute, folder_path);

                    // escrita no ficheiro partilhado do id da tarefa e tempo de execução do programa
                    write_time_spent(shared_file_path, msg_to_execute, time_spent);

                    /**
                     * neste caso, a mensagem passou a ser do tipo COMPLETED e o child_pid refere-se  
                     * a ESTE processo. faz-se uso do pipe previamente aberto em modo de escrita 
                     * para enviar a informação para o pai (que se encontra bloqueado no read) de 
                     * que a tarefa terminou a sua execução. assim, o pai consegue corretamente fazer o 
                     * wait do processo filho responsável pela execução desta tarefa, fazendo tal através
                     * do parâmetro child_pid na mensagem que irá receber.
                     * */
                    msg_to_execute.child_pid = getpid();
                    write_file(aux_fd, &msg_to_execute, sizeof(Msg));
                    exit(0);
                }
            }
        }
    }

    // imprime no ecrã informação de debugging
    write_file(STDOUT_FILENO, "Server shutting down...\n", 25);

    // finalização da contagem do tempo, caso a opção "test-mode" tenha sido usada
    if (is_testing == 0) {
        if (gettimeofday(&time_after, NULL) < 0) {
            perror("[ERROR 21] gettimeofday:");
            exit(EXIT_FAILURE);
        }

        long time_spent = calculate_time_diff(time_before, time_after);
        char time_spent_buffer[50];
        if (sprintf(time_spent_buffer, "Took %ld ms to execute 20 tasks\n", time_spent) < 0) {
            perror("[ERROR 22] sprintf:");
            exit(EXIT_FAILURE);
        }
        // imprime no ecrã informação de debugging
        write_file(STDOUT_FILENO, time_spent_buffer, strlen(time_spent_buffer));
    }

    // fechar ficheiros
    close_file(incoming_fd);
    close_file(aux_fd);

    // apagar o pipe com nome
    if (unlink(MAIN_FIFO_NAME) == -1) {
        perror("[ERROR 23] unlink:");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
