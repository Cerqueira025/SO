#Eiinclude <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

void create_message(Msg *msg, int pid, char *program, int time, MESSAGE_TYPE type) {
    msg->pid = pid;
    strcpy(msg->program, program); /*DUVIDA - outro strcpy?*/
    msg->time = time;
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


int execute_message(char *exec_args[20]) {
    int status = 0, result = -1;

    if(fork() == 0) {
        execvp(exec_args[0], exec_args);
        _exit(255);
    }
    wait(&status);

    if(WIFEXITED(status))
        result = WEXITSTATUS(status);

    return result;
}


int handle_message(Msg *msg_to_handle) {
    char *exec_args[20];

    char *tofree = parse_program(msg_to_handle, exec_args);

    int result = execute_message(exec_args);

    msg_to_handle->type = DONE;

    free(tofree);

    return result;
}


