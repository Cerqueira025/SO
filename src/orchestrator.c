#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/messages.h"
#include "../include/utils.h"

void send_messages(Msg list[], int list_size, int outgoing_fd) {
    Msg outgoing_msg, incoming_msg;
    char buffer[MAX_PROGRAM_SIZE];

    for (int i = 0; i < list_size; i++) {
        incoming_msg = list[i];
        if (sprintf(buffer, "%d %s\n", incoming_msg.pid, incoming_msg.program) <
            0) {
            perror("[ERROR 15] sprintf:");
            exit(EXIT_FAILURE);
        }

        create_message(&outgoing_msg, -1, -1, -1, buffer, STATUS);
        write_file(outgoing_fd, &outgoing_msg, sizeof(Msg));
    }
}

void read_and_send_messages(char *shared_file_path, int outgoing_fd) {
    // usa-se a flag O_CREAT no caso deste ficheiro ainda não ter sido aberto
    int incoming_fd = open_file(shared_file_path, O_RDONLY | O_CREAT, 0777);

    Msg incoming_and_outgoing_msg;
    while (read_file(incoming_fd, &incoming_and_outgoing_msg, sizeof(Msg)) > 0
    ) {
        write_file(outgoing_fd, &incoming_and_outgoing_msg, sizeof(Msg));
    }

    close_file(incoming_fd);
}

void send_status_to_client(
    Msg_list messages, int message_pid, char *shared_file_path
) {
    int outgoing_fd = open_file_pid(message_pid, O_WRONLY, 0);

    Msg msg_to_send;
    create_message(&msg_to_send, -1, -1, -1, "Executing\n", STATUS);

    write_file(outgoing_fd, &msg_to_send, sizeof(Msg));

    send_messages(
        messages.executing_messages, messages.executing_messages_size,
        outgoing_fd
    );

    create_message(&msg_to_send, -1, -1, -1, "\nScheduled\n", STATUS);
    write_file(outgoing_fd, &msg_to_send, sizeof(Msg));

    send_messages(
        messages.scheduled_messages, messages.scheduled_messages_size,
        outgoing_fd
    );

    create_message(&msg_to_send, -1, -1, -1, "\nCompleted\n", STATUS);
    write_file(outgoing_fd, &msg_to_send, sizeof(Msg));

    read_and_send_messages(shared_file_path, outgoing_fd);

    close_file(outgoing_fd);
}

void send_task_number_to_client(int message_pid) {
    int outgoing_fd = open_file_pid(message_pid, O_WRONLY, 0);
    write_file(outgoing_fd, &message_pid, sizeof(message_pid));
    close_file(outgoing_fd);
}

void write_time_spent(
    char *shared_file_path, Msg msg_to_write, long time_spent
) {
    int shared_fd =
        open_file(shared_file_path, O_WRONLY | O_APPEND | O_CREAT, 0777);

    Msg formated_msg_to_write;
    char formated_text[MAX_PROGRAM_SIZE];
    if (sprintf(
            formated_text, "%d %s %ld ms\n", msg_to_write.pid,
            msg_to_write.program, time_spent
        ) < 0) {
        perror("[ERROR 16] sprintf:");
        exit(EXIT_FAILURE);
    }

    create_message(&formated_msg_to_write, -1, -1, -1, formated_text, STATUS);
    write_file(shared_fd, &formated_msg_to_write, sizeof(Msg));
    close_file(shared_fd);
}

int main(int argc, char **argv) {
    if (argc != 4 && argc != 5) {
        write_file(
            STDOUT_FILENO,
            "usage: ./orchestrator output_folder parallel-tasks sched-policy <test-mode>\n",
            77
        );
        exit(EXIT_FAILURE);
    }

    char folder_path[50];
    if (sprintf(folder_path, "tmp/%s", argv[1]) < 0) {
        perror("[ERROR 17] sprintf:");
        exit(EXIT_FAILURE);
    }
    int parallel_tasks = atoi(argv[2]);  // REVER
    SCHED_POLICY sched_policy;
    if (strcmp(argv[3], "FCFS") == 0)
        sched_policy = FCFS;
    else if (strcmp(argv[3], "SJF") == 0)
        sched_policy = SJF;
    else {
        perror("Incorrect schedule policy. Usage: FCFS / SJF");
        exit(EXIT_FAILURE);
    }

    int is_testing = -1;
    if (argv[4] != NULL) {
        if (strcmp(argv[4], "test-mode") == 0) {
            is_testing = 20;
            write_file(STDOUT_FILENO, "Test mode enabled\n", 19);
        } else {
            perror("Incorrect test-mode. Usage: test-mode");
            exit(EXIT_FAILURE);
        }
    }

    // Access determina as permissões de um ficheiro. Quando é usada a flag F_OK
    // é feito apenas um teste de existência. 0 se suceder, -1 se não.
    create_folder(folder_path);

    // cria fifo para receber a mensagem do cliente
    make_fifo(MAIN_FIFO_NAME);

    write_file(STDOUT_FILENO, "Server is running...\n", 22);

    // abre fifo de modo de leitura
    int incoming_fd = open_file(MAIN_FIFO_NAME, O_RDONLY, 0);
    // aberto em modo de write para haver bloqueio no read
    int aux_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);

    struct timeval time_before, time_after;
    if (is_testing > 0) {
        if (gettimeofday(&time_before, NULL) < 0) {
            perror("[ERROR 18] gettimeofday:");
            exit(EXIT_FAILURE);
        }
    }

    // criar e abrir o ficheiro partilhado que terá os IDs e tempos de execução
    char shared_file_path[50];
    if (sprintf(shared_file_path, "%s/tasks_info.bin", folder_path) < 0) {
        perror("[ERROR 19] sprintf:");
        exit(EXIT_FAILURE);
    }

    // inicialização de variáveis
    Msg message_received;
    Msg_list messages_list;
    create_messages_list(&messages_list, parallel_tasks);

    while (read_file(incoming_fd, &message_received, sizeof(Msg)) &&
           (message_received.type != STOP) && is_testing != 0) {
        if (message_received.type == STATUS)
            send_status_to_client(
                messages_list, message_received.pid, shared_file_path
            );
        else {
            // pai recolhe o processo filho
            if (message_received.type == COMPLETED) {
                waitpid(message_received.child_pid, NULL, WUNTRACED);
                delete_from_executing_messages_list(
                    &messages_list, message_received.pid
                );
                if (is_testing > 0) is_testing--;
            } else {
                // enviar o numero do tarefa através do fifo criado pelo cliente
                send_task_number_to_client(message_received.pid);

                insert_scheduled_messages_list(
                    &messages_list, message_received
                );
            }

            if (sched_policy == SJF)
                sort_by_SJF(
                    messages_list.scheduled_messages,
                    messages_list.scheduled_messages_size
                );

            Msg msg_to_execute = get_next_executing_message(&messages_list);

            // se existe uma mensagem a executar
            if (msg_to_execute.type != ERR) {
                // fork para libertar o pai para continuar a leitura
                if (fork() == 0) {
                    long time_spent =
                        parse_and_execute_message(&msg_to_execute, folder_path);

                    write_time_spent(
                        shared_file_path, msg_to_execute, time_spent
                    );

                    // a mensagem já tem tipo COMPLETED e o pid refere-se a ESTE processo
                    msg_to_execute.child_pid = getpid();
                    write_file(aux_fd, &msg_to_execute, sizeof(msg_to_execute));
                    exit(0);
                }
            }
        }
    }

    write_file(STDOUT_FILENO, "Server shutting down...\n", 25);

    if (is_testing == 0) {
        if (gettimeofday(&time_after, NULL) < 0) {
            perror("[ERROR 20] gettimeofday:");
            exit(EXIT_FAILURE);
        }

        long time_spent = calculate_time_diff(time_before, time_after);
        char time_spent_buffer[50];
        if (sprintf(
                time_spent_buffer, "Took %ld ms to execute 20 tasks\n",
                time_spent
            ) < 0) {
            perror("[ERROR 21] sprintf:");
            exit(EXIT_FAILURE);
        }
        write_file(STDOUT_FILENO, time_spent_buffer, strlen(time_spent_buffer));
    }

    // fechar ficheiros
    close_file(incoming_fd);
    close_file(aux_fd);

    // apagar o pipe com nome
    if (unlink(MAIN_FIFO_NAME) == -1) {
        perror("[ERROR 22] unlink:");
        exit(EXIT_FAILURE);
    }

    return 0;
}
