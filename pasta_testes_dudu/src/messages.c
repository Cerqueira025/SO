#include <sys/time.h>
#include <stdio.h>
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

void create_message(Msg *msg, int pid, char *program, int time) {
    msg->pid = pid;
    strcpy(msg->program, program); /*DUVIDA - outro strcpy?*/
    msg->time = time;
    msg->time_spent = -1;
}

void free_message(Msg msg) {
    free(msg.program);
}


long calculate_time_diff(struct timeval time_before, struct timeval time_after) {
    long seconds = time_after.tv_sec - time_before.tv_sec;
    long micro_seconds = time_after.tv_usec - time_before.tv_usec;
    if(micro_seconds < 0) {
        micro_seconds += 1000000;
        seconds--;
    }

    return seconds*1000 + micro_seconds/1000;
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

    /*DUVIDA - Onde medir o tempo gasto na execução do programa?
     * Usar pipes ou não?*/

    struct timeval time_before, time_after;

    gettimeofday(&time_before, NULL);
    int result = execute_message(exec_args);
    gettimeofday(&time_after, NULL);

    msg_to_handle->time_spent = calculate_time_diff(time_before, time_after);

    free(tofree);

    return result;
}


