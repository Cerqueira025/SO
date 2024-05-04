#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/messages.h"
#include "../include/utils.h"

/**
 * dada uma mensagem pronta a enviar, envia para o destino fixo
 * definido por MAIN_FIFO_NAME.
*/
void send_message_to_server(Msg msg_to_send) {
    int outgoing_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);
    write_file(outgoing_fd, &msg_to_send, sizeof(Msg));
    close_file(outgoing_fd);
}

/**
 * dado o caminho para o fifo temporário e único já criado,
 * é recebido um inteiro correspondente ao id da tarefa
 * atribuido pelo servidor
*/
void receive_and_print_tasknum(char *server_to_client_fifo) {
    // abrir fifo para receber o id da tarefa
    int tasknum;
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);
    read_file(incoming_fd, &tasknum, sizeof(tasknum));
    close_file(incoming_fd);

    char tasknum_buffer[50];
    if (sprintf(tasknum_buffer, "TASK %d Received\n", tasknum) < 0) {
        perror("[ERROR 1] sprintf:");
        exit(EXIT_FAILURE);
    }
    write_file(STDOUT_FILENO, tasknum_buffer, strlen(tasknum_buffer));
}

/**
 * dado o caminho para o fifo temporário e único já criado,
 * é recebido um conjunto de mensagens prontas a serem imprimidas 
*/
void receive_and_print_status(char *server_to_client_fifo) {
    // abrir fifo para receber o status
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);

    Msg_to_print incoming_msg;
    char status_buffer[MAX_STRING_SIZE];
    while (read_file(incoming_fd, &incoming_msg, sizeof(Msg_to_print)) > 0) {
        if (incoming_msg.type != TEXT && incoming_msg.type != PROGRAM_ID && incoming_msg.type != PROGRAM_ID_TIMESPENT) {
            perror("[ERROR 2] message of incorrect type:");
            exit(EXIT_FAILURE);
        }

        // a mensagem apenas inclui texto
        if (incoming_msg.type == TEXT) {
            if (sprintf(status_buffer, "%s", incoming_msg.program) < 0) {
                perror("[ERROR 3] sprintf:");
                exit(EXIT_FAILURE);
            }
        }
        // a mensagem inclui o id da tarefa e o programa da mesma
        else if (incoming_msg.type == PROGRAM_ID) {
            if (sprintf(status_buffer, "%d %s\n", incoming_msg.pid, incoming_msg.program) < 0) {
                perror("[ERROR 4] sprintf:");
                exit(EXIT_FAILURE);
            }
        }
        // a mensagem inclui o id da tarefa, o programa da mesma e o tempo que demorou a executar
        else {
            if (sprintf(status_buffer, "%d %s %ld ms\n", incoming_msg.pid, incoming_msg.program, incoming_msg.time_spent) < 0) {
                perror("[ERROR 5] sprintf:");
                exit(EXIT_FAILURE);
            }
        }

        write_file(STDOUT_FILENO, status_buffer, strlen(status_buffer));
    }

    close_file(incoming_fd);
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 5) {
        write_file(STDOUT_FILENO, "Usage:\n    ./client execute time(ms) -u 'prog-a arg-1 (...) arg-n'\n    ./client execute time(ms) -p 'prog-a arg-1 (...) arg-n|prog-b arg-1 (...) arg-n|prog-c arg-1 (...) arg-n'\n    ./client status\n    ./client server-stop\n", 223);
        exit(EXIT_FAILURE);
    }

    int pid = getpid();

    // cria fifo para receber a resposta do servidor com o id da tarefa
    char server_to_client_fifo[25];
    if (sprintf(server_to_client_fifo, "tmp/fifo_%d", pid) < 0) {
        perror("[ERROR 6] sprintf:");
        exit(EXIT_FAILURE);
    }
    make_fifo(server_to_client_fifo);

    // inicialização da mensagem a enviar para o servidor
    Msg msg_to_send;

    /*-----------EXECUTE------------*/
    if (strcmp(argv[1], "execute") == 0) {
        int time = atoi(argv[2]);

        int is_pipe = -1;
        if (strcmp(argv[3], "-u") == 0)
            is_pipe = 0;
        else if (strcmp(argv[3], "-p") == 0)
            is_pipe = 1;
        else {
            perror("Incorrect flag. Usage: -u / -p");
            exit(EXIT_FAILURE);
        }

        char program[MAX_PROGRAM_SIZE];
        strcpy(program, argv[4]);

        if (!check_correct_format(program, is_pipe)) {
            perror("[ERROR 32] incorrect flag for program:");
            exit(EXIT_FAILURE);
        }

        // cria-se a mensagem a enviar para o servidor
        create_message(&msg_to_send, pid, time, is_pipe, program, SCHEDULED);

        // envia-se a mensagem para o servidor
        send_message_to_server(msg_to_send);

        // como a opção, neste caso, é execute, lê-se e imprime-se o id da tarefa a receber do servidor
        receive_and_print_tasknum(server_to_client_fifo);
    }

    /*-----------STATUS------------*/
    else if (strcmp(argv[1], "status") == 0) {
        // cria-se a mensagem a enviar para o servidor, preenchendo os parâmetros não usados com valores distintos
        create_message(&msg_to_send, pid, -1, -1, NULL, STATUS);

        // envia-se a mensagem para o servidor
        send_message_to_server(msg_to_send);

        // como a opção, neste caso, é status, lê-se e imprime-se o as mensagens formatadas a receber do servidor
        receive_and_print_status(server_to_client_fifo);
    }

    /*-----------SERVER-STOP------------*/
    else if (strcmp(argv[1], "server-stop") == 0) {
        // cria-se a mensagem a enviar para o servidor, preenchendo os parâmetros não usados com valores distintos
        create_message(&msg_to_send, pid, -1, -1, NULL, STOP);

        // envia-se a mensagem para o servidor
        send_message_to_server(msg_to_send);

        write_file(STDOUT_FILENO, "Server shutting down...\n", 25);
    }

    else {
        write_file(STDOUT_FILENO, "Incorrect option. Usage: execute / status / server-stop\n", 57);
    }

    // apagar fifo
    if (unlink(server_to_client_fifo) == -1) {
        perror("[ERROR 7] unlink:");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
