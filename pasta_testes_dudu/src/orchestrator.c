#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/utils.h"
#include "../include/messages.h"

int main(int argc, char **argv) {
    char* folder_path = NULL;

    if(argc != 4) {
        printf("usage: ./orchestrator output_folder parallel-tasks sched-policy\n");
        exit(EXIT_FAILURE);
    } 
    else folder_path = argv[1];
    
    

    // Access determina as permissões de um ficheiro. Quando é usada a flag F_OK
    // é feito apenas um teste de existência. 0 se suceder, -1 se não.
    if (access(folder_path, F_OK) == -1) {
        if (mkdir(folder_path, 0664) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    }

    make_fifo(MAIN_FIFO_NAME);
    printf("Server is running...\n");
    int incoming_fd = open_file(MAIN_FIFO_NAME, O_RDONLY, 0);
    int aux_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);

    char shared_file_path[40];
    sprintf(shared_file_path, "%s/tasks_time.bin", folder_path);
    int shared_fd = open_file(shared_file_path, O_CREAT | O_APPEND, 0640);

    Msg msg_received;
    while(read(incoming_fd, &msg_received, sizeof(Msg))) {

/*
 * A fazer:
 * é  necessário haver um wait por parte do processo pai.
 * para isso, precisamos de comunicar que o processo filho
 * acabou, para poder fazer o wait.
 * podemos comunicar com o processo pai a dizer que acabou
 * fazendo uso do pipo com nome existente, uma vez que o
 * pai está sempre bloqueado no mesmo. dito isso, também 
 * necessitamos de fazer uma distinção dos tipos de mensagens
 * recebidos.
 * */



        if(fork() == 0) {
            int result = handle_message(&msg_received);
                        
            char formated_text[50];
            sprintf(formated_text, "Task %d wasted %ld milisseconds\n", msg_received.pid, msg_received.time_spent);
            write(shared_fd, &formated_text, sizeof(formated_text));
            

/*

            char file_path[20];
            sprintf(file_path, "TASK:%d.bin", msg_received.pid);

            int outgoing_fd = open_file(file_path, O_WRONLY | O_CREAT, 0640);
            write(outgoing_fd, &result, sizeof(int));
            close_file(outgoing_fd);




*/
            

        }

    }

    close_file(shared_fd);
    close_file(incoming_fd);
    close_file(aux_fd);

    // Apagar o pipe com nome
    if (unlink(MAIN_FIFO_NAME) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }


    return 0;
}

