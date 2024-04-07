#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/func.h"
#include "../include/utils.h"

int main(int argc, char **argv) {
    char* folder_path = NULL;

    if(argc != 4) {
        printf("usage: ./orchestrator output_folder parallel-tasks sched-policy\n");
        exit(EXIT_FAILURE);
    } else {
        folder_path = 1[argv];
    }
    

    // Access determina as permissões de um ficheiro. Quando é usada a flag F_OK
    // é feito apenas um teste de existência. 0 se suceder, -1 se não.
    if (access(folder_path, F_OK) == -1) {
        if (mkdir(folder_path, 0777) == -1) {
            perror("mkdir");
            exit(EXIT_FAILURE);
        }
    }


    // Cria o pipe com nome. make_fifo está definido noutro local e segue:
    make_fifo(MAIN_FIFO_NAME);
   

   
    printf("Monitor is running...\n");

    // Abre o pipe com nome no modo de leitura
    int fd;
    open_fifo(&fd, MAIN_FIFO_NAME, O_RDONLY);

    // Abre o pipe com nome no modo de escrita
    int fd2;
    open_fifo(&fd2, MAIN_FIFO_NAME, O_WRONLY);
    

    int n;
    while(read(fd, &n, sizeof(n))) {
        printf("valor recebido: %d\n", n);
    }

    close_fifo(fd);
    close_fifo(fd2);

    // Apagar o pipe com nome
    if (unlink(MAIN_FIFO_NAME) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }


    return 0;
}

