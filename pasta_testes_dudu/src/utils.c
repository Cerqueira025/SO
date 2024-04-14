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
    if (close(fd) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}
