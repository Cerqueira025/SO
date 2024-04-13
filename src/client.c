#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "../include/utils.h"

int main(int argc, char **argv) {
  if(argc < 5) {
    printf("usage: %s execute time -u 'prog-a [args]'\n", argv[0]);
    return 1;
  }

  size_t time = atoi(argv[2]);

  // Espera pela devida conexão com o pipe do servidor
  int client_fd;
  do {
    client_fd = open(MAIN_FIFO_NAME, O_WRONLY);
    if (client_fd == -1) {
      perror("open");
      sleep(1); // Tenta novamente em 1 segundo
    }
  } while (client_fd == -1);


  int n = 10;
  write(client_fd, &n, sizeof(int));


  return 0;
}


