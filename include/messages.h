#ifndef MESSAGES_H
#define MESSAGES_H

#define MAX_MESSAGE_NUMBER 100
#define MAX_MESSAGE_SIZE 350
#define MAX_PIPE_NUMBER 20

typedef enum message_type {
    ERR,
    STATUS,
    SCHEDULED,
    EXECUTING,
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

void create_message(
    Msg *msg, int pid, int time, int is_pipe, char *program, MESSAGE_TYPE type
);

//void free_message(Msg msg);

long parse_and_execute_message(Msg *msg_to_handle, char *folder_path);

void create_messages_list(Msg_list *messages_list, int parallel_tasks);

void insert_scheduled_messages_list(
    Msg_list *messages_list, Msg message_to_isert
);

void delete_from_executing_messages_list(Msg_list *messages_list, int pid);

Msg get_next_executing_message(Msg_list *messages_list);

#endif
