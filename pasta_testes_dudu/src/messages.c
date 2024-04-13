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

void create_message(Msg msg, int pid, char *program, int time) {
    msg.pid = pid;
    msg.program = strdup(program);
    msg.time = time;
}

void free_message(Msg msg) {
    free(msg.program);
}


int handle_func(char *program, int time) {
    time--; // para dar silence ao warning
    int i = 0, res = -1;
    char *exec_args[20];
    char *string, *cmd, *tofree;

    tofree = cmd = strdup(program);
    while((string = strsep(&cmd, " ")) != NULL) {
        exec_args[i] = string;
        i++;
    }
    exec_args[i] = NULL;

    if(fork() == 0) {
        execlp("ls", "ls", "-1", NULL);
        _exit(255);
    }

    int status = 0;
    wait(&status);
    if(WIFEXITED(status))
        res = WEXITSTATUS(status);

    free(tofree);

    return res;
}

