#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>

#define MAIN_FIFO_NAME "fifo_main"

typedef enum sched_policy {
    FCFS,
    SJF
    // PP
} SCHED_POLICY;

void create_folder(char *folder_path);

void make_fifo(char *fifo_name);

int open_file(char *fifo_name, int flags, mode_t mode);

int open_file_pid(int message_pid, int flags, mode_t mode);

void close_file(int fd);

void write_file(int outgoing_fd, const void *msg_to_send, size_t n_byes);

int read_file(int outgoing_fd, void *msg_to_send, size_t n_byes);

long calculate_time_diff(struct timeval time_before, struct timeval time_after);
