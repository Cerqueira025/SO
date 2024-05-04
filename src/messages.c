#include "../include/messages.h"

#include "../include/utils.h"

/**
 * preenche todos os parâmetros de uma mensagem do tipo Msg
*/
void create_message(Msg *msg, int pid, int time, int is_pipe, char *program, MESSAGE_TYPE type) {
    if (msg == NULL) {
        perror("[ERROR 8] invalid message:");
        exit(EXIT_FAILURE);
    }
    msg->pid = pid;
    msg->time = time;
    msg->is_pipe = is_pipe;
    if (program != NULL) strcpy(msg->program, program);
    msg->type = type;
}

/**
 * preenche todos os parâmetros de uma mensagem do tipo Msg_to_print
*/
void create_message_to_print(Msg_to_print *msg, int pid, int time_spent, char *program, MESSAGE_TYPE type) {
    if (msg == NULL) {
        perror("[ERROR 9] invalid message:");
        exit(EXIT_FAILURE);
    }
    msg->pid = pid;
    msg->time_spent = time_spent;
    if (program != NULL) strcpy(msg->program, program);
    msg->type = type;
}

/**
 * separa um programa em diferentes argumentos, tendo por base um formatter, 
 * e preenche o array exec_args
*/
char *parse_program(char *program, char *exec_args[20], char *formatter, int *number_args) {
    int i = 0;
    char *string, *cmd, *tofree;

    tofree = cmd = strdup(program);
    while ((string = strsep(&cmd, formatter)) != NULL) {
        exec_args[i] = string;
        i++;
    }
    exec_args[i] = NULL;

    *number_args = i;

    return tofree;
}

/**
 * dada uma lista com um programa e os seus argumentos, executa o mesmo, 
 * redirecionando o STDOUT e o STDERR para o ficheiro contruído através
 * dos restantes parâmetros
*/
void execute_message(int pid, char *exec_args[20], char *folder_path) {
    int status = 0, temp_fd = -1;

    char buf[30];
    if (sprintf(buf, "%s/task_%d.txt", folder_path, pid) < 0) {
        perror("[ERROR 10] sprintf:");
        exit(EXIT_FAILURE);
    }
    // cria e abre o ficheiro
    temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

    if (fork() == 0) {
        dup2(temp_fd, STDOUT_FILENO);  // redirecionar o stdout para o ficheiro
        dup2(temp_fd, STDERR_FILENO);  // redirecionar o stderr para o ficheiro
        close_file(temp_fd);
        execvp(exec_args[0], exec_args);
        _exit(255);
    }

    close_file(temp_fd);
    wait(&status);

    if (WIFEXITED(status) && WEXITSTATUS(status) > 0) {
        perror("[ERROR 11] fork execution failure:");
        exit(EXIT_FAILURE);
    }
}

/**
 * dada uma lista com vários programas e os seus argumentos, separa cada
 * programa através do parse_program e executa os mesmos, redirecionando o STDOUT
 * e o STDERR para o ficheiro contruído através dos restantes parâmetros
*/
void execute_pipe_message(int message_pid, char *exec_args[20], char *folder_path, int number_args) {
    int num_pipes = number_args - 1;
    int pipes[MAX_PIPE_NUMBER][2];
    char *tofree[MAX_PIPE_NUMBER];

    char buf[40];
    if (sprintf(buf, "%s/task_%d.txt", folder_path, message_pid) < 0) {
        perror("[ERROR 12] sprintf:");
        exit(EXIT_FAILURE);
    }
    int temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("[ERROR 13] pipe error:");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < number_args; i++) {
        if (fork() == 0) {
            char *command_args[20];
            char *formatter = " ";

            int number_args_commands = 0;
            tofree[i] = parse_program(exec_args[i], command_args, formatter, &number_args_commands);

            if (i > 0) {
                dup2(pipes[i - 1][0], 0);
                close_file(pipes[i - 1][0]);
            }
            if (i < num_pipes) {
                dup2(pipes[i][1], 1);
                close_file(pipes[i][1]);
            } else {
                dup2(temp_fd, 1);  // std_out
                dup2(temp_fd, 2);  // std_err
                close_file(temp_fd);
            }

            for (int j = 0; j < num_pipes; j++) {
                close_file(pipes[j][0]);
                close_file(pipes[j][1]);
            }

            execvp(command_args[0], command_args);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_pipes; i++) {
        close_file(pipes[i][0]);
        close_file(pipes[i][1]);
    }

    tofree[number_args] = NULL;
    for (int j = 0; tofree[j] != NULL; j++) free(tofree[j]);

    for (int i = 0; i < number_args; i++) {
        int status;
        wait(&status);
        if (WIFEXITED(status) && WEXITSTATUS(status) > 0) perror("[ERROR 14] fork execution failure:");
    }
}

/**
 * faz uso das últimas 3 funções, ao passo que, ao receber uma mensagem, averigua
 * o seu tipo, separa o programa de acordo com o anterior e executa, aproveitando
 * para contabilixar o tempo que o programa demorou a executar e para alterar o estado
 * da mensagem para COMPLETED
*/
long parse_and_execute_message(Msg *msg_to_handle, char *folder_path) {
    int number_args;
    char *exec_args[20];
    char *formatter = msg_to_handle->is_pipe ? "|" : " ";

    // este é o primeiro parse, que funcionar para mensagens do tipo pipe ou singular.
    char *tofree = parse_program(msg_to_handle->program, exec_args, formatter, &number_args);

    struct timeval time_before, time_after;
    if (gettimeofday(&time_before, NULL) < 0) {
        perror("[ERROR 15] gettimeofday:");
        exit(EXIT_FAILURE);
    }

    if (msg_to_handle->is_pipe)
        // nesta função, usa-se novamente "parse_program()", mas a string que esta devolve
        // leva um "free" dentro desta função.
        execute_pipe_message(msg_to_handle->pid, exec_args, folder_path, number_args);
    else
        // nesta função não se usa "parse_program()"
        execute_message(msg_to_handle->pid, exec_args, folder_path);

    if (gettimeofday(&time_after, NULL) < 0) {
        perror("[ERROR 16] gettimeofday:");
        exit(EXIT_FAILURE);
    }
    long time_spent = calculate_time_diff(time_before, time_after);

    msg_to_handle->type = COMPLETED;

    free(tofree);

    return time_spent;
}

/**
 * inicializa uma lista de mensagens
*/
void create_messages_list(Msg_list *messages_list, int parallel_tasks) {
    messages_list->scheduled_messages_size = 0;
    messages_list->executing_messages_size = 0;
    messages_list->parallel_tasks = parallel_tasks;
}

/**
 * insere uma mensagem recebida to tipo SCHEDULED na lista de mensagens agendadas
*/
void insert_scheduled_messages_list(Msg_list *messages_list, Msg message_to_insert) {
    if (message_to_insert.type != SCHEDULED) {
        perror("[ERROR 17] message of incorrect type:");
        exit(EXIT_FAILURE);
    }
    messages_list->scheduled_messages[messages_list->scheduled_messages_size++] = message_to_insert;
}

/**
 * retira a primeira mensagem da lista de mensagens agendadas e devolve a mesma
*/
Msg pop_scheduled_messages_list(Msg_list *messages_list) {
    Msg first = messages_list->scheduled_messages[0];

    for (int i = 0; i < messages_list->scheduled_messages_size - 1; i++) messages_list->scheduled_messages[i] = messages_list->scheduled_messages[i + 1];

    messages_list->scheduled_messages_size--;
    return first;
}

/**
 * insere uma mensagem na lista de mensagens a executar
*/
void insert_executing_messages_list(Msg_list *messages_list, Msg *message) {
    message->type = EXECUTING;
    messages_list->executing_messages[messages_list->executing_messages_size++] = *message;
}

/**
 * retira uma mensagem da lista de mensagens a executar
*/
void delete_from_executing_messages_list(Msg_list *messages_list, int pid) {
    int flag = 1;
    for (int i = 0; i < messages_list->executing_messages_size && flag; i++)
        if (messages_list->executing_messages[i].pid == pid) {
            flag = 0;
            for (int j = i; j < messages_list->executing_messages_size - 1; j++) messages_list->executing_messages[j] = messages_list->executing_messages[j + 1];
        }

    messages_list->executing_messages_size--;
}

/**
 * recebe a próxima mensagem na lista de mensagens agendadas. a mensagem candidata
 * é sempre a primeira da lista de mensagens agendadas, uma vez que dá-se sempre,
 * anteriormente, a ordenação da lista de acordo com a política de escalonamento
*/
Msg get_next_executing_message(Msg_list *messages_list) {
    Msg to_execute;
    if (messages_list->parallel_tasks > messages_list->executing_messages_size && messages_list->scheduled_messages_size > 0) {
        to_execute = pop_scheduled_messages_list(messages_list);
        insert_executing_messages_list(messages_list, &to_execute);
    } else
        to_execute.type = ERR;

    return to_execute;
}

/**
 * ordena a lista de mensagens agendadas, por ordem crescente, de
 * acordo com o tempo previsto que o programa demorará a executar.
*/
void sort_by_SJF(Msg *scheduled_messages, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = i; j < size; j++) {
            if (scheduled_messages[i].time > scheduled_messages[j].time) {
                Msg temp = scheduled_messages[i];
                scheduled_messages[i] = scheduled_messages[j];
                scheduled_messages[j] = temp;
            }
        }
    }
}
