#ifndef UTILS_H
#define UTILS_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define MAIN_FIFO_NAME "tmp/fifo_main"
#define NUM_TESTS 20

typedef enum sched_policy {
    FCFS,
    SJF
} SCHED_POLICY;

void create_folder(char *folder_path);

void make_fifo(char *fifo_name);

int open_file(char *fifo_name, int flags, mode_t mode);

int open_file_pid(int message_pid, int flags, mode_t mode);

void close_file(int fd);

void write_file(int outgoing_fd, const void *msg_to_send, size_t n_byes);

int read_file(int outgoing_fd, void *msg_to_send, size_t n_byes);

long calculate_time_diff(struct timeval time_before, struct timeval time_after);

int check_correct_format(char *program, int is_pipe);

#endif
