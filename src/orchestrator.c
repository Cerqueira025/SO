#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../include/messages.h"
#include "../include/utils.h"

void send_messages(Msg list[], int list_size, int outgoing_fd) {
    char buffer[350];
    for (int i = 0; i < list_size; i++) {
        Msg msg = list[i];
        sprintf(buffer, "%d %s\n", msg.pid, msg.program);
        write(outgoing_fd, buffer, sizeof(buffer));
    }
}

void read_and_send_messages(char *shared_file_path, int outgoing_fd) {
    // usa-se a flag O_CREAT no caso deste ficheiro ainda não ter sido aberto
    int incoming_fd = open_file(shared_file_path, O_RDONLY | O_CREAT, 0777);

    char buffer[MAX_MESSAGE_SIZE];
    while (read(incoming_fd, &buffer, MAX_MESSAGE_SIZE) > 0)
        write(outgoing_fd, &buffer, MAX_MESSAGE_SIZE);

    close(incoming_fd);
}

void send_status_to_client(
    Msg_list messages, int message_pid, char *shared_file_path
) {
    int outgoing_fd = open_file_pid(message_pid, O_WRONLY, 0);

    write(outgoing_fd, "Executing\n", MAX_MESSAGE_SIZE);
    send_messages(
        messages.executing_messages, messages.executing_messages_size,
        outgoing_fd
    );

    write(outgoing_fd, "\nScheduled\n", MAX_MESSAGE_SIZE);
    send_messages(
        messages.scheduled_messages, messages.scheduled_messages_size,
        outgoing_fd
    );

    write(outgoing_fd, "\nCompleted\n", MAX_MESSAGE_SIZE);
    read_and_send_messages(shared_file_path, outgoing_fd);

    close(outgoing_fd);
}

void send_task_number_to_client(int message_pid) {
    int outgoing_fd = open_file_pid(message_pid, O_WRONLY, 0);
    write(outgoing_fd, &message_pid, sizeof(message_pid));
    close(outgoing_fd);
}

void write_time_spent(
    char *shared_file_path, Msg msg_to_write, long time_spent
) {
    int shared_fd =
        open_file(shared_file_path, O_WRONLY | O_APPEND | O_CREAT, 0777);

    char formated_text[MAX_MESSAGE_SIZE];
    sprintf(
        formated_text, "%d %s %ld ms\n", msg_to_write.pid, msg_to_write.program,
        time_spent
    );

    write(shared_fd, formated_text, sizeof(formated_text));
    close_file(shared_fd);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf(
            "usage: ./orchestrator output_folder parallel-tasks sched-policy\n"
        );
        exit(EXIT_FAILURE);
    }

    char *folder_path = argv[1];
    int parallel_tasks = atoi(argv[2]);

    // Access determina as permissões de um ficheiro. Quando é usada a flag F_OK
    // é feito apenas um teste de existência. 0 se suceder, -1 se não.
    create_folder(folder_path);

    // cria fifo para receber a mensagem do cliente
    make_fifo(MAIN_FIFO_NAME);

    printf("Server is running...\n");

    // abre fifo de modo de leitura
    int incoming_fd = open_file(MAIN_FIFO_NAME, O_RDONLY, 0);
    // aberto em modo de write para haver bloqueio no read
    int aux_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);

    // criar e abrir o ficheiro partilhado que terá os IDs e tempos de execução
    char shared_file_path[50];
    sprintf(shared_file_path, "%s/tasks_info.txt", folder_path);

    // inicialização de variáveis
    Msg message_received;
    Msg_list messages_list;
    create_messages_list(&messages_list, parallel_tasks);

    while (read(incoming_fd, &message_received, sizeof(Msg)) &&
           (message_received.type != STOP)) {
        if (message_received.type == STATUS)
            send_status_to_client(
                messages_list, message_received.pid, shared_file_path
            );
        else {
            // pai apanha o processo filho
            if (message_received.type == COMPLETED) {
                waitpid(message_received.child_pid, NULL, WUNTRACED);
                delete_from_executing_messages_list(
                    &messages_list, message_received.pid
                );
            } else {
                // enviar o numero do tarefa através do fifo criado pelo cliente
                send_task_number_to_client(message_received.pid);

                insert_scheduled_messages_list(
                    &messages_list, message_received
                );
            }

            //sort(POLICY);

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
                    write(aux_fd, &msg_to_execute, sizeof(msg_to_execute));
                    exit(0);
                }
            }
        }
    }

    printf("Server shutting down...\n");

    // fechar ficheiros
    close_file(incoming_fd);
    close_file(aux_fd);

    // apagar o pipe com nome
    if (unlink(MAIN_FIFO_NAME) == -1) {
        perror("unlink");
        exit(EXIT_FAILURE);
    }

    return 0;
}
