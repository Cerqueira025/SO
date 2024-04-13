#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include "../include/utils.h"
#include "../include/messages.h"

int main(int argc, char **argv) {
    // ./client execute time -u 'prog-a [args]' 
    // ./client check tasknum

    // para dar silence do warning
    int numero = argc;
    numero--;

    if(strcmp(argv[1], "execute") == 0) {
        // Espera pela devida conexão com o pipe do servidor
        int outgoing_fd = open(MAIN_FIFO_NAME, O_WRONLY);


        Msg msg_to_send;
        msg_to_send.pid = -999; // para silenciar o warning
        int pid = getpid();
        char *program = argv[4];
        int time = atoi(argv[2]);
        create_message(msg_to_send, pid, program, time);


        write(outgoing_fd, &msg_to_send, sizeof(Msg));
        printf("TASK %d Received\n", pid);
        close(outgoing_fd);

    } else if(strcmp(argv[1], "check") == 0) {
        int result;

        char buf[20]; 
        sprintf(buf, "TASK:%s.bin", argv[2]);

        int fildes = open(buf, O_RDONLY);
        read(fildes, &result, sizeof(int));
        printf("O RSULTADO È %d\n", result);

        close(fildes);
    }
    else
        printf("POR DEFINIR\n");

    return 0;
}


