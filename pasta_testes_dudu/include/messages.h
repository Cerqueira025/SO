#ifndef MESSAGES_H
#define MESSAGES_H

typedef struct msg {
    int pid;
    char* program;
    int time;
} Msg;

/*
 * Debater a necessidade destas funções. Retirar comentario se necessário
void set_message_pid(Msg msg, int pid);

void set_message_program(Msg msg,char* program);

void set_message_time(Msg msg, int time);
*/


void create_message(Msg msg, int pid, char *program, int time);

void free_message(Msg msg);

int handle_func(char *program, int time);

#endif
