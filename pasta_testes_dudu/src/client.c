#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "../include/utils.h"
#include "../include/messages.h"

int main(int argc, char **argv) {
    // ./client execute time -u "prog-a [args]" 
    // ./client check tasknum

    /*-----------EXECUTE------------*/
    if(strcmp(argv[1], "execute") == 0) {
        int pid = getpid();

        // cria fifo para receber a resposta do servidor com o pid
        char server_to_client_fifo[20];
        sprintf(server_to_client_fifo, "fifo_%d", pid);
        make_fifo(server_to_client_fifo);


        // preparar mensagem a enviar para o servidor
        Msg msg_to_send;

        char program[300]; /*DUVIDA - Podemos usar strcpy?*/
        strcpy(program, argv[4]);

        int time = atoi(argv[2]);
        create_message(&msg_to_send, pid, program, time, SINGLE);

        // enviar a mensagem para o servidor
        int outgoing_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);
        write(outgoing_fd, &msg_to_send, sizeof(Msg));
        close_file(outgoing_fd);


        // abrir fifo para receber o número da tarefa
        int tasknum;
        int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);
        read(incoming_fd, &tasknum, sizeof(int));
        close(incoming_fd);

        printf("TASK %d Received\n", tasknum);

    }

    /*-----------CHECK------------*/
    else if(strcmp(argv[1], "check") == 0) {
        char buf[20]; 
        sprintf(buf, "TASK_%s.bin", argv[2]);

        int result;
        int fildes = open_file(buf, O_RDONLY, 0);
        read(fildes, &result, sizeof(int));
        printf("O RSULTADO È %d\n", result);

        close_file(fildes);
    }

    /*-----------STATUS------------*/
    else if(strcmp(argv[1], "status") == 0) {
        // TO DO....
    }

    else
        printf("POR DEFINIR\n");

    return 0;
}


