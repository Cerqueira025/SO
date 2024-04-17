#include <sys/time.h>
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
    char* folder_path = NULL;

    if(argc != 4) {
        printf("usage: ./orchestrator output_folder parallel-tasks sched-policy\n");
        exit(EXIT_FAILURE);
    } 
    else folder_path = argv[1];



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
    Msg msg_received;
    while(read(incoming_fd, &msg_received, sizeof(Msg))) {

        // pai apanha o processo filho
        if(msg_received.type == DONE) {
            int status;
            waitpid(msg_received.pid, &status, WUNTRACED);

            /*
            if(WIFEXITED(status) > 0 && WI)
                msg_received.type = OK;
            */
        }

        // fork para libertar o pai para continuar a leitura
        else if(msg_received.type == SINGLE) {
            
            // enviar o numero do tarefa através do fifo criado pelo cliente
            send_task_number_to_client(msg_received.pid);

            if(fork() == 0) {
                
                // iniciar a contagem de tempo da tarefa
                struct timeval time_before, time_after;
                gettimeofday(&time_before, NULL);

                // processamento da mensagem
                int result = handle_message(&msg_received, folder_path);

                /*DUVIDA - Onde medir o tempo gasto na execução do programa?
                 * Usar pipes ou não?*/

                // finaliza a contagem de tempo da tarefa
                gettimeofday(&time_after, NULL);
                long time_spent = calculate_time_diff(time_before, time_after);





                /*------------------NAO ESTA OPEN_FILE--------------------*/
                int shared_fd = open(shared_file_path, O_CREAT | O_WRONLY | O_APPEND, 0777); // DUVIDA - temos sempre usado o open_file, mas neste especificamente não funciona
                /*------------------NAO ESTA OPEN_FILE--------------------*/
                // escreve no ficheiro partilhado o ID e tempo de execução do programa 
                char formated_text[30];
                sprintf(formated_text, "%d %s %ld ms\n", msg_received.pid, msg_received.program, time_spent); // DUVIDA - não deveriamos colocar também no ficheiro partilhado o nome do programa, para além do tempo e id de execução?
                write(shared_fd, formated_text, sizeof(formated_text));
                close_file(shared_fd);









                /*----------------ACABAR COM O PROCESSO------------------*/
                msg_received.pid = getpid(); /*DUVIDA - estamos a dar overwrite to pid vindo do programa do cliente para facilitar o waitpid por parte do pai*/
                write(aux_fd, &msg_received, sizeof(msg_received)); // a mensagem já tem tipo DONE e o pid refere-se a este processo
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

