#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/utils.h"
#include "../include/messages.h"


typedef struct msg Msg;

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

void create_message(Msg *msg, int pid, int is_pipe, int time, char *program, MESSAGE_TYPE type) {
    msg->pid = pid;
    msg->time = time;
    msg->is_pipe = is_pipe; 
    strcpy(msg->program, program); 
    msg->type = type;
}

void free_message(Msg msg) {
    free(msg.program);
}




char* parse_program(Msg *msg_to_handle, char *exec_args[20]) {
    int i = 0;
    char *string, *cmd, *tofree;

    tofree = cmd = strdup(msg_to_handle->program);
    while((string = strsep(&cmd, " ")) != NULL) {
        exec_args[i] = string;
        i++;
    }
    exec_args[i] = NULL;

    return tofree;
}


void execute_message(int pid, char *exec_args[20], char *folder_path) {
    int status = 0, original_stdout_fd = -1, original_stderr_fd = -1, temp_fd = -1;


    original_stdout_fd = dup(STDOUT_FILENO); 
    original_stderr_fd = dup(STDERR_FILENO); 

    char buf[30];
    sprintf(buf, "%s/task_%d.bin", folder_path, pid);
    temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

    dup2(temp_fd, STDOUT_FILENO);
    dup2(temp_fd, STDOUT_FILENO);
    close(temp_fd);

    if(fork() == 0) {
        execvp(exec_args[0], exec_args);
        _exit(255);
    }
    wait(&status);

    dup2(original_stdout_fd, STDOUT_FILENO);
    dup2(original_stderr_fd, STDERR_FILENO);


    if(WIFEXITED(status) && WEXITSTATUS(status) > 0) {
        perror("execução correu mal");
        exit(EXIT_FAILURE);
    }

}


long handle_message(Msg *msg_to_handle, char *folder_path) {
    char *exec_args[20];

    char *tofree = parse_program(msg_to_handle, exec_args);

    struct timeval time_before, time_after;
    gettimeofday(&time_before, NULL);

    execute_message(msg_to_handle->pid, exec_args, folder_path);

    gettimeofday(&time_after, NULL);
    long time_spent = calculate_time_diff(time_before, time_after);


    msg_to_handle->type = COMPLETED;

    free(tofree);

    return time_spent;
}



void init_messages_list(Msg_list *messages_list, int parallel_tasks) {
    messages_list->scheduled_messages_size = 0;
    messages_list->executing_messages_size = 0;
    messages_list->parallel_tasks = parallel_tasks;
}



void insert_scheduled_messages_list(Msg_list *messages_list, Msg message_to_insert) {
    if (message_to_insert.type != SCHEDULED) {
        perror("message incorrect type");
        exit(EXIT_FAILURE);
    }
    messages_list->scheduled_messages[messages_list->scheduled_messages_size++] = message_to_insert;
}

Msg pop_scheduled_messages_list(Msg_list *messages_list) {
    Msg first = messages_list->scheduled_messages[0];

    for(int i=0; i<messages_list->scheduled_messages_size-1; i++) 
        messages_list->scheduled_messages[i] = messages_list->scheduled_messages[i+1];

    messages_list->scheduled_messages_size--;
    return first;
}



void insert_executing_messages_list(Msg_list *messages_list, Msg *message) {
    message->type = EXECUTING;
    messages_list->executing_messages[messages_list->executing_messages_size++] = *message;
}

void delete_from_executing_messages_list(Msg_list *messages_list, int pid) {
    int flag = 1;
    for(int i=0; i<messages_list->executing_messages_size && flag; i++)
        if(messages_list->executing_messages[i].pid == pid) { 
            flag = 0; 
            for(int j=i; j<messages_list->executing_messages_size-1; j++) 
                messages_list->scheduled_messages[j] = messages_list->scheduled_messages[j+1];
        }

    messages_list->executing_messages_size--;
}



Msg get_next_executing_message(Msg_list *messages_list) {
    Msg to_execute;
    if(messages_list->parallel_tasks > messages_list->executing_messages_size && messages_list->scheduled_messages_size > 0) {
        to_execute = pop_scheduled_messages_list(messages_list);
        insert_executing_messages_list(messages_list, &to_execute);

    }
    else to_execute.type = ERR;

    return to_execute;
}





