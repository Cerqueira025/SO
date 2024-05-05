#include "../include/messages.h"
#include "../include/utils.h"


int main(int argc, char **argv) {
    if (argc != 2 && argc != 5) {
        write_file(STDOUT_FILENO, "Usage:\n    ./client execute time(ms) -u 'prog [args]'\n    ./client execute time(ms) -p 'prog-a [args] | prog-b [args] | prog-c [args]'\n    ./client status\n    ./client server-stop\n", 181);
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
