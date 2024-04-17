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


int execute_message(int pid, char *exec_args[20], char *folder_path) {
    int status = 0, result = -1, original_stdout_fd = -1, original_stderr_fd = -1, temp_fd = -1;


    if(fork() == 0) {
        original_stdout_fd = dup(fileno(stdout)); // DUVIDA - porque temos de fazer dup e não int = ___
        original_stderr_fd = dup(fileno(stderr)); // DUVIDA - porque temos de fazer dup e não int = ___

        char buf[30];
        sprintf(buf, "%s/task_%d.bin", folder_path, pid);
        temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

        // redirecionar o stdout para o ficheiro
        dup2(temp_fd, fileno(stdout));
        
        // redirecionar o stderr para o ficheiro
        dup2(temp_fd, fileno(stderr));

        execvp(exec_args[0], exec_args);
        _exit(255);
    }

    wait(&status);

    dup2(original_stdout_fd, fileno(stdout));
    dup2(original_stderr_fd, fileno(stderr));
    close(temp_fd);

    if(WIFEXITED(status)) 
        result = WEXITSTATUS(status);
        

    return result;
}


int handle_message(Msg *msg_to_handle, char *folder_path) {
    char *exec_args[20];

    char *tofree = parse_program(msg_to_handle, exec_args);

    int result = execute_message(msg_to_handle->pid, exec_args, folder_path);

    msg_to_handle->type = DONE;

    free(tofree);

    return result;
}


