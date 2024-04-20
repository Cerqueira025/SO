#ifndef MESSAGES_H
#define MESSAGES_H

#define MAX_MESSAGE_NUMBER 100

typedef enum message_type {
    ERR,
    STATUS, // status
    SCHEDULED, 
    EXECUTING,
  //FINISHED
    COMPLETED,

} MESSAGE_TYPE;


typedef struct msg {
    int pid;
    int child_pid;
    int time;
    int is_pipe;
    char program[300]; /*DUVIDA - fixed size de 300 ou não?*/
    MESSAGE_TYPE type;
} Msg;

typedef struct msg_list {
    int scheduled_messages_size;
    Msg scheduled_messages[MAX_MESSAGE_NUMBER];

    int executing_messages_size;
    Msg executing_messages[MAX_MESSAGE_NUMBER];

    int parallel_tasks;
} Msg_list;



//long calculate_time_diff(struct timeval time_before, struct timeval time_after);

void create_message(Msg *msg, int pid, int is_pipe, int time, char *program, MESSAGE_TYPE type);

//void free_message(Msg msg);

long handle_message(Msg *msg_to_handle, char *folder_path);

void init_messages_list(Msg_list *messages_list, int parallel_tasks);

void insert_scheduled_messages_list(Msg_list *messages_list, Msg message_to_isert);

void delete_from_executing_messages_list(Msg_list *messages_list, int pid);

Msg get_next_executing_message(Msg_list *messages_list);

#endif
