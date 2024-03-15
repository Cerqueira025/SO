#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h> //chamadas ao sistema: defs e decls essenciais
#include <fcntl.h>  //O_RDONLY, O_WRONLY, O_CREAT, O_*


int main(int argc, char* argv[]){

  //execução de uma pipeline de programas
  if (strcmp(argv[1],"-p") == 0){
    /**
     * prog-a [args] | prog-b [args] | prog-c [args]
     * 
     * dividir este input em chamadas de programas individuais
    */
  }

  //execução de um programa individual
  if (strcmp(argv[1],"-u") == 0){
  }

  //imprime o estado das tarefas no programa
  if (strcmp(argv[1],"status") == 0){
  }
}

/**
 * NOTAS:
 * - obj é utilizar o paralelismo
 * - o output vai para um ficheiro
 * - explorar as politicas de escalonamento
*/