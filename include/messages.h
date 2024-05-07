#ifndef MESSAGES_H
#define MESSAGES_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define MAX_MESSAGE_NUMBER 100
#define MAX_PROGRAM_SIZE 300
#define MAX_STRING_SIZE 350  // 300 + 50 para manobra
#define MAX_PIPE_NUMBER 100
#define MAX_EXEC_ARGS 100

typedef enum message_type {
    ERR,
    STATUS,
    STOP,
    SCHEDULED,
    EXECUTING,
    COMPLETED,
    TEXT,
    PROGRAM_ID,
    PROGRAM_ID_TIMESPENT
} MESSAGE_TYPE;

typedef struct msg_to_print {
    int pid;
    long time_spent;
    char program[MAX_PROGRAM_SIZE];
    MESSAGE_TYPE type;
} Msg_to_print;

typedef struct msg {
    int pid;
    int child_pid;
    int time;
    int is_pipe;
    char program[MAX_PROGRAM_SIZE];
    MESSAGE_TYPE type;
    struct timeval start_time;
} Msg;

typedef struct msg_list {
    int scheduled_messages_size;
    Msg scheduled_messages[MAX_MESSAGE_NUMBER];

    int executing_messages_size;
    Msg executing_messages[MAX_MESSAGE_NUMBER];

    int parallel_tasks;
} Msg_list;

void create_message(Msg *msg, int pid, int time, int is_pipe, char *program, MESSAGE_TYPE type);

void create_message_to_print(Msg_to_print *msg, int pid, int time_spent, char *program, MESSAGE_TYPE type);

long parse_and_execute_message(Msg *msg_to_handle, char *folder_path);

void create_messages_list(Msg_list *messages_list, int parallel_tasks);

void insert_scheduled_messages_list(Msg_list *messages_list, Msg message_to_isert);

void delete_from_executing_messages_list(Msg_list *messages_list, int pid);

Msg get_next_executing_message(Msg_list *messages_list);

void sort_by_SJF(Msg *scheduled_messages, int size);

/*---------FUNÇÕES AUXILIARES ORCHESTRATOR---------*/
void send_messages(Msg list[], int list_size, int outgoing_fd);

void read_and_send_messages(char *shared_file_path, int outgoing_fd);

void send_status_to_client(Msg_list messages, int message_pid, char *shared_file_path);

void send_task_number_to_client(int message_pid);

void write_time_spent(char *shared_file_path, Msg msg_to_write, long time_spent);

double calculate_average_time_spent(char *shared_file_path);

/*---------FUNÇÕES AUXILIARES CLIENT---------*/
void send_message_to_server(Msg msg_to_send);

void receive_and_print_tasknum(char *server_to_client_fifo);

void receive_and_print_status(char *server_to_client_fifo);


#endif
