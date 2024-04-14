#ifndef MESSAGES_H
#define MESSAGES_H

typedef enum message_type {
    SINGLE,
   // PIPELINE,
   // STATUS,
   // WAITING,
   // FINISHED,
    DONE,

} MESSAGE_TYPE;


typedef struct msg {
    int pid;
    char program[300]; /*DUVIDA - fixed size de 300 ou não?*/
    int time;
    MESSAGE_TYPE type;
} Msg;


/*
 * Debater a necessidade destas funções. Retirar comentario se necessário
void set_message_pid(Msg msg, int pid);

void set_message_program(Msg msg,char* program);

void set_message_time(Msg msg, int time);
*/

//long calculate_time_diff(struct timeval time_before, struct timeval time_after);

void create_message(Msg *msg, int pid, char *program, int time, MESSAGE_TYPE type);

//void free_message(Msg msg);

int handle_message(Msg *msg_to_handle);

#endif
