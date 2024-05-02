#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    char n;
    int read_bytes = read(STDIN_FILENO, &n, sizeof(n));
    printf("Recebi este número: %d\n", read_bytes);
    if(read_bytes > 0)
        perror("teste");
        
    

    return 0;
}
