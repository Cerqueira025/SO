#include "../include/messages.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "../include/utils.h"


/*
 * Debater uso destas funções. Remover comentário se necessário.
void set_message_pid(Msg msg, int pid) {
    msg.pid = pid;
}

void set_message_program(Msg msg,char* program) {
    msg.program = strdup(program);
}

void set_message_time(Msg msg, int time) {
    msg.time = time;
}
*/

void create_message(
    Msg *msg, int pid, int time, int is_pipe, char *program, MESSAGE_TYPE type
) {
    msg->pid = pid;
    msg->time = time;
    msg->is_pipe = is_pipe;
    if (program != NULL) strcpy(msg->program, program);
    msg->type = type;
}


char *parse_program(
    char *program, char *exec_args[20], char *formatter, int *number_args
) {
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

void execute_message(int pid, char *exec_args[20], char *folder_path) {
    int status = 0, original_stdout_fd = -1, original_stderr_fd = -1,
        temp_fd = -1;

    original_stdout_fd = dup(STDOUT_FILENO);
    original_stderr_fd = dup(STDERR_FILENO);

    char buf[30];
    sprintf(buf, "%s/task_%d.bin", folder_path, pid);
    temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

    dup2(temp_fd, STDOUT_FILENO);
    dup2(temp_fd, STDERR_FILENO);
    close(temp_fd);

    if (fork() == 0) {
        execvp(exec_args[0], exec_args);
        _exit(255);
    }
    wait(&status);

    dup2(original_stdout_fd, STDOUT_FILENO);
    dup2(original_stderr_fd, STDERR_FILENO);

    if (WIFEXITED(status) && WEXITSTATUS(status) > 0) {
        perror("[ERROR] Execution failure\n");
        exit(EXIT_FAILURE);
    }
}

void execute_pipe_message(
    int message_pid, char *exec_args[20], char *folder_path, int number_args
) {
    int num_pipes = number_args - 1;
    int pipes[MAX_PIPE_NUMBER][2];

    char buf[30];
    sprintf(buf, "%s/task_%d.bin", folder_path, message_pid);
    int temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe error");
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < number_args; i++) {
        if (fork() == 0) {
            char *command_args[20];
            char *formatter = " ";

            /* o facto de não ter um inteiro a receber o valor do número de argumentos
            *  e sim o NULL que nós tinhamos, levava ao programa não funcionar
            */
            int number_args_commands = 0;
            parse_program(
                exec_args[i], command_args, formatter, &number_args_commands
            );

            if (i > 0) {
                dup2(pipes[i - 1][0], 0);
                close(pipes[i - 1][0]);
            }
            if (i < num_pipes) {
                dup2(pipes[i][1], 1);
                close(pipes[i][1]);
            } else {
                dup2(temp_fd, 1); // std_out
                dup2(temp_fd, 2); // std_err
                close(temp_fd);
            }

            for (int j = 0; j < num_pipes; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            execvp(command_args[0], command_args);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < num_pipes; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < number_args; i++) {
        wait(NULL);
    }
}

long parse_and_execute_message(Msg *msg_to_handle, char *folder_path) {
    int number_args;
    char *exec_args[20];
    char *formatter = msg_to_handle->is_pipe ? "|" : " ";
    char *tofree = parse_program(
        msg_to_handle->program, exec_args, formatter, &number_args
    );

    struct timeval time_before, time_after;
    gettimeofday(&time_before, NULL);

    if (msg_to_handle->is_pipe)
        execute_pipe_message(
            msg_to_handle->pid, exec_args, folder_path, number_args
        );
    else
        execute_message(msg_to_handle->pid, exec_args, folder_path);

    gettimeofday(&time_after, NULL);
    long time_spent = calculate_time_diff(time_before, time_after);

    msg_to_handle->type = COMPLETED;

    free(tofree);

    return time_spent;
}

void create_messages_list(Msg_list *messages_list, int parallel_tasks) {
    messages_list->scheduled_messages_size = 0;
    messages_list->executing_messages_size = 0;
    messages_list->parallel_tasks = parallel_tasks;
}

void insert_scheduled_messages_list(
    Msg_list *messages_list, Msg message_to_insert
) {
    if (message_to_insert.type != SCHEDULED) {
        perror("message incorrect type");
        exit(EXIT_FAILURE);
    }
    messages_list
        ->scheduled_messages[messages_list->scheduled_messages_size++] =
        message_to_insert;
}

Msg pop_scheduled_messages_list(Msg_list *messages_list) {
    Msg first = messages_list->scheduled_messages[0];

    for (int i = 0; i < messages_list->scheduled_messages_size - 1; i++)
        messages_list->scheduled_messages[i] =
            messages_list->scheduled_messages[i + 1];

    messages_list->scheduled_messages_size--;
    return first;
}

void insert_executing_messages_list(Msg_list *messages_list, Msg *message) {
    message->type = EXECUTING;
    messages_list
        ->executing_messages[messages_list->executing_messages_size++] =
        *message;
}

void delete_from_executing_messages_list(Msg_list *messages_list, int pid) {
    int flag = 1;
    for (int i = 0; i < messages_list->executing_messages_size && flag; i++)
        if (messages_list->executing_messages[i].pid == pid) {
            flag = 0;
            for (int j = i; j < messages_list->executing_messages_size - 1; j++)
                messages_list->scheduled_messages[j] =
                    messages_list->scheduled_messages[j + 1];
        }

    messages_list->executing_messages_size--;
}

Msg get_next_executing_message(Msg_list *messages_list) {
    Msg to_execute;
    if (messages_list->parallel_tasks >
            messages_list->executing_messages_size &&
        messages_list->scheduled_messages_size > 0) {
        to_execute = pop_scheduled_messages_list(messages_list);
        insert_executing_messages_list(messages_list, &to_execute);

    } else
        to_execute.type = ERR;

    return to_execute;
}

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
