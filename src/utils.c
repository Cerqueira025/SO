#include "../include/utils.h"

void create_folder(char *folder_path) {
    if (access(folder_path, F_OK) == -1) {
        if (mkdir(folder_path, 0777) == -1) {
            perror("[ERROR 24] mkdir:");
            exit(EXIT_FAILURE);
        }
    }
}

void make_fifo(char *fifo_name) {
    if (mkfifo(fifo_name, 0666) == -1) {
        perror("[ERROR 25] mkfifo:");
        exit(EXIT_FAILURE);
    }
}

int open_file(char *file_name, int flags, mode_t mode) {
    int fd;
    if (mode == 0)
        fd = open(file_name, flags);
    else
        fd = open(file_name, flags, mode);

    if (fd == -1) {
        perror("[ERROR 26] open:");
        exit(EXIT_FAILURE);
    }

    return fd;
}

int open_file_pid(int message_pid, int flags, mode_t mode) {
    char buffer[20];
    if (sprintf(buffer, "tmp/fifo_%d", message_pid) < 0) {
        perror("[ERROR 27] sprintf:");
        exit(EXIT_FAILURE);
    }
    int outgoing_fd = open_file(buffer, flags, mode);

    return outgoing_fd;
}

void close_file(int fd) {
    if (close(fd) == -1) {
        perror("[ERROR 28] close:");
        exit(EXIT_FAILURE);
    }
}

void write_file(int outgoing_fd, const void *msg_to_send, size_t n_byes) {
    if (write(outgoing_fd, msg_to_send, n_byes) == -1) {
        perror("[ERROR 29] write:");
        exit(EXIT_FAILURE);
    }
}

int read_file(int outgoing_fd, void *msg_to_read, size_t n_byes) {
    int read_bytes = 0;
    if ((read_bytes = read(outgoing_fd, msg_to_read, n_byes)) == -1) {
        perror("[ERROR 30] read:");
        exit(EXIT_FAILURE);
    }
    return read_bytes;
}

void exec_command(char **exec_args) {
    if (execvp(exec_args[0], exec_args) == -1) {
        perror("[ERROR 31] execvp:");
        exit(EXIT_FAILURE);
    }
}

long calculate_time_diff(
    struct timeval time_before, struct timeval time_after
) {
    long seconds = time_after.tv_sec - time_before.tv_sec;
    long micro_seconds = time_after.tv_usec - time_before.tv_usec;
    if (micro_seconds < 0) {
        micro_seconds += 1000000;
        seconds--;
    }

    return seconds * 1000 + micro_seconds / 1000;
}

int check_correct_format(char *program, int is_pipe) {
    char flag = '|';
    int found_flag = 0, res = 1;
    for (int i = 0; program[i] && !found_flag; i++) 
        if (program[i] == flag) 
            found_flag = 1;

    if ((is_pipe && !found_flag) || (!is_pipe && found_flag)) res = 0;

    return res;
}
