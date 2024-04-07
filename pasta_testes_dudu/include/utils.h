#define MAIN_FIFO_NAME "ficheiro_pipe.fifo"

void make_fifo(char *fifo_name);

void open_fifo(int *fd, char *fifo_name, int flags);

void close_fifo(int fd);

