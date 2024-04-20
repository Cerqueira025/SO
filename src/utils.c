#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

void make_fifo(char *fifo_name) {
    if (mkfifo(fifo_name, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }
}

int open_file(char *file_name, int flags, mode_t mode) {
    int fd;
    if(mode == 0) fd = open(file_name, flags);
    else fd = open(file_name, flags, mode);
    

    if (fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    return fd;
}

void close_file(int fd) {
    if(close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void exec_command(char **exec_args) {
    if(execvp(exec_args[0], exec_args) == -1) {
        perror("execvp");
        exit(EXIT_FAILURE);
    }
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

