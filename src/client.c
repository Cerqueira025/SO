#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/messages.h"
#include "../include/utils.h"

void send_message_to_server(Msg msg_to_send) {
    int outgoing_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);
    write(outgoing_fd, &msg_to_send, sizeof(Msg));
    close_file(outgoing_fd);
}

void receive_and_print_tasknum(char *server_to_client_fifo) {
    // abrir fifo para receber o número da tarefa
    int tasknum;
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);
    read(incoming_fd, &tasknum, sizeof(int));
    close(incoming_fd);

    printf("TASK %d Received\n", tasknum);
}

void receive_and_print_status(char *server_to_client_fifo) {
    // abrir fifo para receber o status
    char buf[MAX_MESSAGE_SIZE];
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);
    while (read(incoming_fd, buf, sizeof(buf)) > 0) printf("%s", buf);

    close(incoming_fd);
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 5) {
        printf(
            "Usage:\n    ./client execute time -u 'prog-a [args]'\n    ./client status\n"
        );
        exit(EXIT_FAILURE);
    }

    int pid = getpid();

    // cria fifo para receber a resposta do servidor com o pid
    char server_to_client_fifo[20];
    sprintf(server_to_client_fifo, "fifo_%d", pid);
    make_fifo(server_to_client_fifo);

    // preparar mensagem a enviar para o servidor
    Msg msg_to_send;

    if (strcmp(argv[1], "execute") == 0) {
        int time = atoi(argv[2]);

        int ispipe = -1; 
        if (strcmp(argv[3], "-u") == 0) ispipe = 0;
        else if (strcmp(argv[3], "-p") == 0) ispipe = 1;
        else exit(EXIT_FAILURE);
        
        printf("É PIPE: %d\n", ispipe);

        char program[300];
        strcpy(program, argv[4]);

        create_message(&msg_to_send, pid, time, ispipe, program, SCHEDULED);

        send_message_to_server(msg_to_send);

        receive_and_print_tasknum(server_to_client_fifo);
    }

    /*-----------STATUS------------*/
    else if (strcmp(argv[1], "status") == 0) {
        create_message(&msg_to_send, pid, -1, -1, NULL, STATUS);

        send_message_to_server(msg_to_send);

        receive_and_print_status(server_to_client_fifo);
    } else
        printf("Incorrect option\n");


    // apagar fifo
    if (unlink(server_to_client_fifo) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}
