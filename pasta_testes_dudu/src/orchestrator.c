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
    else 
        folder_path = argv[1];
    
    

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

    int incoming_fd = open_fifo(MAIN_FIFO_NAME, O_RDONLY);
    int aux_fd = open_fifo(MAIN_FIFO_NAME, O_WRONLY);

    Msg msg_received;
    while(read(incoming_fd, &msg_received, sizeof(Msg))) {
        if(fork() == 0) {
            int result = handle_func(msg_received.program, msg_received.time);
            
            char file_path[20];
            sprintf(file_path, "TASK:%d.bin", msg_received.pid);

            int outgoing_fd = open(file_path, O_WRONLY | O_CREAT);
            write(outgoing_fd, &result, sizeof(int));
            close(outgoing_fd);
        }
    }

    close_fifo(incoming_fd);
    close_fifo(aux_fd);

    // Apagar o pipe com nome
    if (unlink(MAIN_FIFO_NAME) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }


    return 0;
}

