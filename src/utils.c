#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>

/**
 * Algumas funções auxiliares
*/

void make_fifo(char *fifo_name) {
  if (mkfifo(fifo_name, 0666) == -1) {
    perror("mkfifo");
    exit(EXIT_FAILURE);
  }
}

void open_fifo(int *fd, char *fifo_name, int flags) {
  *fd = open(fifo_name, flags);

  if (*fd == -1) {
    perror("open");
    exit(EXIT_FAILURE);
  }
}


void close_fifo(int fd) {
  if (close(fd) == -1) {
    perror("close");
    exit(EXIT_FAILURE);
  }
}
