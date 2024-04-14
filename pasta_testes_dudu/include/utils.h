#define MAIN_FIFO_NAME "fifo_main"

void make_fifo(char *fifo_name);

int open_file(char *fifo_name, int flags, mode_t mode);

void close_file(int fd);

