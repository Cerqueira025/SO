#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/utils.h"
#include "../include/messages.h"


void send_task_number_to_client(int message_pid) {
    char buffer[20];
    sprintf(buffer, "fifo_%d", message_pid);
    int outgoing_fd = open_file(buffer, O_WRONLY, 0);
    write(outgoing_fd, &message_pid, sizeof(message_pid));
    close(outgoing_fd);
}


int main(int argc, char **argv) {
    if(argc != 4) {
        printf("usage: ./orchestrator output_folder parallel-tasks sched-policy\n");
        exit(EXIT_FAILURE);
    } 

    char *folder_path = argv[1];
    int parallel_tasks = atoi(argv[2]);




    // Access determina as permissões de um ficheiro. Quando é usada a flag F_OK
    // é feito apenas um teste de existência. 0 se suceder, -1 se não.
    if(access(folder_path, F_OK) == -1) {
        if(mkdir(folder_path, 0777) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    }

    // cria fifo para receber a mensagem do cliente
    make_fifo(MAIN_FIFO_NAME);

    printf("Server is running...\n");

    // abre fifo de modo de leitura
    int incoming_fd = open_file(MAIN_FIFO_NAME, O_RDONLY, 0);
    // aberto em modo de write para haver bloqueio no read
    int aux_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);


    // criar e abrir o ficheiro partilhado que terá os IDs e tempos de execução
    char shared_file_path[50];
    sprintf(shared_file_path, "%s/tasks_info.txt", folder_path);


    // leitura de cada mensagem
    Msg message_received;

    Msg_list messages_list;
    init_messages_list(&messages_list, parallel_tasks);

    while(read(incoming_fd, &message_received, sizeof(Msg))) {

        // pai apanha o processo filho
        if(message_received.type == COMPLETED) {
            waitpid(message_received.child_pid, NULL, WUNTRACED);
            printf("ACABEI\n");
            delete_from_executing_messages_list(&messages_list, message_received.pid);
        }

        else {
            // enviar o numero do tarefa através do fifo criado pelo cliente
            send_task_number_to_client(message_received.pid);

            insert_scheduled_messages_list(&messages_list, message_received);
            printf("ENTREI\n");
        }
        //printf("TAMANHO %d, PID %d\n", messages_list.scheduled_messages_size, messages_list.scheduled_messages[messages_list.scheduled_messages_size-1].pid);
        
        //sort(POLICY);
        Msg msg_to_execute = get_next_executing_message(&messages_list);

        // se há mensagem para executar
        if(msg_to_execute.type != ERR) {
            // fork para libertar o pai para continuar a leitura
            if(fork() == 0) {

                /*---------PROCESSAMENTO E EXECUÇÂO DA MENSAGEM---------*/
                printf("COMECEI A EXECUTAR\n");

                long time_spent = handle_message(&msg_to_execute, folder_path);

                /*----------------ESCRITA DO TEMPO GASTO----------------*/
                int shared_fd = open_file(shared_file_path, O_CREAT | O_WRONLY | O_APPEND, 0777);
                char formated_text[30];
                sprintf(formated_text, "%d %s %ld ms\n", msg_to_execute.pid, msg_to_execute.program, time_spent); 
                write(shared_fd, formated_text, sizeof(formated_text));
                close_file(shared_fd);


                /*----------------ACABAR COM O PROCESSO-----------------*/
                msg_to_execute.child_pid = getpid(); 
                write(aux_fd, &msg_to_execute, sizeof(msg_to_execute)); // a mensagem já tem tipo COMPLETED e o pid refere-se a este processo
                exit(0);
            }
        }

    }
        

    // fechar ficheiros
    close_file(incoming_fd);
    close_file(aux_fd);

    // apagar o pipe com nome
    if (unlink(MAIN_FIFO_NAME) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }


    return 0;
}

