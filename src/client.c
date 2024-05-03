#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/messages.h"
#include "../include/utils.h"

void send_message_to_server(Msg msg_to_send) {
    int outgoing_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);
    write_file(outgoing_fd, &msg_to_send, sizeof(Msg));
    close_file(outgoing_fd);
}

void receive_and_print_tasknum(char *server_to_client_fifo) {
    // abrir fifo para receber o número da tarefa
    int tasknum;
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);
    read(incoming_fd, &tasknum, sizeof(tasknum));
    close_file(incoming_fd);

    char tasknum_buffer[50];
    sprintf(tasknum_buffer, "TASK %d Received\n", tasknum);
    write_file(STDOUT_FILENO, tasknum_buffer, strlen(tasknum_buffer));
}

void receive_and_print_status(char *server_to_client_fifo) {
    // abrir fifo para receber o status
    char buf[MAX_MESSAGE_SIZE];
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);

    while (read(incoming_fd, buf, MAX_MESSAGE_SIZE) > 0) {
        char status_buffer[MAX_MESSAGE_SIZE];
        sprintf(status_buffer, "%s", buf);
        write_file(STDOUT_FILENO, buf, strlen(buf));
    }

    close_file(incoming_fd);
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 5) {
        write_file(STDOUT_FILENO, "Usage:\n    ./client execute time(ms) -u 'prog-a arg-1 (...) arg-n'\n    ./client execute time(ms) -p 'prog-a arg-1 (...) arg-n|prog-b arg-1 (...) arg-n|prog-c arg-1 (...) arg-n'\n    ./client status\n    ./client server-stop\n", 223);
        exit(EXIT_FAILURE);
    }

    int pid = getpid();

    // cria fifo para receber a resposta do servidor com o pid
    char server_to_client_fifo[20];
    sprintf(server_to_client_fifo, "fifo_%d", pid);
    make_fifo(server_to_client_fifo);

    // preparar mensagem a enviar para o servidor
    Msg msg_to_send;

    /*-----------EXECUTE------------*/
    if (strcmp(argv[1], "execute") == 0) {
        int time = atoi(argv[2]);

        int ispipe = -1;
        if (strcmp(argv[3], "-u") == 0)
            ispipe = 0;
        else if (strcmp(argv[3], "-p") == 0)
            ispipe = 1;
        else {
            write_file(STDERR_FILENO, "Incorrect flag. Usage: -u / -p\n", 32);
            exit(EXIT_FAILURE);
        }

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
    }

    /*-----------SERVER-STOP------------*/
    else if (strcmp(argv[1], "server-stop") == 0) {
        create_message(&msg_to_send, pid, -1, -1, NULL, STOP);

        send_message_to_server(msg_to_send);

        write_file(STDOUT_FILENO, "Server shutting down...\n", 25);
    }

    else {
        write_file(STDOUT_FILENO, "Incorrect option. Usage: execute / status / server-stop\n", 57);
    }

    // apagar fifo
    if (unlink(server_to_client_fifo) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}
