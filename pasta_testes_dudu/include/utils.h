#define MAIN_FIFO_NAME "fifo_main"

void make_fifo(char *fifo_name);

int open_fifo(char *fifo_name, int flags);

void close_fifo(int fd);

