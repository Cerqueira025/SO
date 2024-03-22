#include <stdio.h>
#include "aux.h"

int mysystem (const char* command) {

	int res;

  char *exec_args[20];
  char *string, *cmd, *tofree; 
  int i = 0;
  tofree = cmd = strdup(command);
  while ((string=strsep(&cmd, " "))!=NULL){
    exec_args[i] = string;
    i++;
  }
  exec_args[i] = NULL;  

  pid_t pid = fork();

  if(pid < 0){
    printf("Fork failed");
    res = -1;
  }
  else if(pid == 0){ //processo filho
    res = execvp(exec_args[0], exec_args);

    perror("erros no exec");
    _exit(res);
  }
  else{ //processo pai
    int status;
    wait(&status);
    if(WIFEXITED(status)){
      if(WEXITSTATUS(status) == 255){
        res = -1;
      }
      else{
        res = WEXITSTATUS(status);
      }
    }
  }

  free(tofree);
	return res;
}

void pipeline_func (int N, char** commands){
  int count[N];

  for (int i = 0; i < N; i++){
    pid_t pid = fork();
    if(pid == 0){ //código executado pelo processo filho
      while (mysystem(commands[i]) != 0){
        count[i]++;
      }
      _exit(count[i]);
    }
  }
  int status;
  pid_t wait_pid;
  while ((wait_pid = wait(&status)) > 0);

  for(int i = 0; i<N; i++){
    printf("command[%d] = %d\n", i, WEXITSTATUS(status));
  }
}
