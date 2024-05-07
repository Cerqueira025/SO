#include "../include/messages.h"
#include "../include/utils.h"

/**
 * preenche todos os parâmetros de uma mensagem do tipo Msg
*/
void create_message(Msg *msg, int pid, int time, int is_pipe, char *program, MESSAGE_TYPE type) {
    if (msg == NULL) {
        perror("[ERROR 8] invalid message:");
        exit(EXIT_FAILURE);
    }
    msg->pid = pid;
    msg->time = time;
    msg->is_pipe = is_pipe;
    if (program != NULL) strcpy(msg->program, program);
    msg->type = type;
}

/**
 * preenche todos os parâmetros de uma mensagem do tipo Msg_to_print
*/
void create_message_to_print(Msg_to_print *msg, int pid, int time_spent, char *program, MESSAGE_TYPE type) {
    if (msg == NULL) {
        perror("[ERROR 9] invalid message:");
        exit(EXIT_FAILURE);
    }
    msg->pid = pid;
    msg->time_spent = time_spent;
    if (program != NULL) strcpy(msg->program, program);
    msg->type = type;
}

/**
 * separa um programa em diferentes argumentos, tendo por base um formatter, 
 * e preenche o array exec_args
*/
char *parse_program(char *program, char *exec_args[MAX_EXEC_ARGS], char *formatter, int *number_args) {
    int i = 0;
    char *string, *cmd, *tofree;

    tofree = cmd = strdup(program);
    while ((string = strsep(&cmd, formatter)) != NULL) {
        exec_args[i] = string;
        i++;
    }
    exec_args[i] = NULL;

    *number_args = i;

    return tofree;
}

/**
 * dada uma lista com um programa e os seus argumentos, executa o mesmo, 
 * redirecionando o STDOUT e o STDERR para o ficheiro contruído através
 * dos restantes parâmetros
*/
void execute_message(int pid, char *exec_args[MAX_EXEC_ARGS], char *folder_path) {
    int status = 0, temp_fd = -1;

    char buf[30];
    if (sprintf(buf, "%s/task_%d.txt", folder_path, pid) < 0) {
        perror("[ERROR 10] sprintf:");
        exit(EXIT_FAILURE);
    }
    // cria e abre o ficheiro
    temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

    if (fork() == 0) {
        dup2(temp_fd, STDOUT_FILENO);  // redirecionar o stdout para o ficheiro
        dup2(temp_fd, STDERR_FILENO);  // redirecionar o stderr para o ficheiro
        close_file(temp_fd);
        execvp(exec_args[0], exec_args);
        _exit(255);
    }

    close_file(temp_fd);
    wait(&status);

    if (WIFEXITED(status) && WEXITSTATUS(status) > 0)
        perror("[ERROR 11] fork execution failure:");
}

/**
 * dada uma lista com vários programas e os seus argumentos, separa cada
 * programa através do parse_program e executa os mesmos, redirecionando o STDOUT
 * e o STDERR para o ficheiro construído através dos restantes parâmetros
*/
void execute_pipe_message(int message_pid, char *exec_args[MAX_EXEC_ARGS], char *folder_path, int number_args) {
    int pipes[MAX_PIPE_NUMBER][2];
    char *tofree[MAX_PIPE_NUMBER + 1];
	int status[MAX_PIPE_NUMBER + 1];

    char buf[40];
    if (sprintf(buf, "%s/task_%d.txt", folder_path, message_pid) < 0) {
        perror("[ERROR 12] sprintf:");
        exit(EXIT_FAILURE);
    }
    int temp_fd = open_file(buf, O_CREAT | O_WRONLY, 0640);

	for (int c = 0; c < number_args; c++) {
        
        /*----PRIMEIRO PROCESSO----*/
		if (c == 0) {
			if (pipe(pipes[c]) != 0) {
				perror("[ERROR 13] pipe:");
				exit(EXIT_FAILURE);
			}
			switch(fork()) {
				case -1:
					perror("[ERROR 33] fork:");
					exit(EXIT_FAILURE);

				case 0:
					close_file(pipes[c][0]);

					dup2(pipes[c][1],1);
					close_file(pipes[c][1]);


					char *command_args[MAX_EXEC_ARGS];
                    char *formatter = " ";
                    int number_args_commands = 0;
                    tofree[c] = parse_program(exec_args[c], command_args, formatter, &number_args_commands);

					execvp(command_args[0], command_args);
                    exit(EXIT_FAILURE);

				default:
					close_file(pipes[c][1]);
			}
		}

        /*----ÚLTIMO PROCESSO----*/
		else if (c == number_args - 1) {
			// não se cria um pipe novo, uma vez que não haverá um próximo processo
            switch(fork()) {
				case -1:
					perror("[ERROR 34] fork:");
					exit(EXIT_FAILURE);

				case 0:
					dup2(pipes[c-1][0], 0);
					close_file(pipes[c-1][0]);

                    dup2(temp_fd, 1);
                    close_file(temp_fd);


					char *command_args[MAX_EXEC_ARGS];
                    char *formatter = " ";
                    int number_args_commands = 0;
                    tofree[c] = parse_program(exec_args[c], command_args, formatter, &number_args_commands);

					execvp(command_args[0], command_args);
                    exit(EXIT_FAILURE);

				default:
					close_file(pipes[c-1][0]);
                    close_file(temp_fd);
			}
		}

        /*----PROCESSOS INTERMÉDIOS----*/
		else {
			if (pipe(pipes[c]) != 0) {
				perror("[ERROR 35] pipe:");
				exit(EXIT_FAILURE);
			}
			switch(fork()) {
				case -1:
					perror("[ERROR 36] fork:");
					exit(EXIT_FAILURE);

				case 0:
					close_file(pipes[c][0]);

					dup2(pipes[c][1],1);
					close_file(pipes[c][1]);

					dup2(pipes[c-1][0],0);
					close_file(pipes[c-1][0]);


					char *command_args[MAX_EXEC_ARGS];
                    char *formatter = " ";
                    int number_args_commands = 0;
                    tofree[c] = parse_program(exec_args[c], command_args, formatter, &number_args_commands);

					execvp(command_args[0], command_args);
                    exit(EXIT_FAILURE);

				default:
					close_file(pipes[c][1]);
					close_file(pipes[c-1][0]);
			}
		}
	}

	for (int i = 0; i < number_args; i++) {
        wait(&status[i]);
        if (WIFEXITED(status[i]) && WEXITSTATUS(status[i]) > 0) 
            perror("[ERROR 14] fork execution failure:");
    }
    
    // libertação da memória alocada
    tofree[number_args] = NULL;
    for (int j = 0; tofree[j] != NULL; j++) free(tofree[j]);
}

/**
 * faz uso das últimas 3 funções, ao passo que, ao receber uma mensagem, averigua
 * o seu tipo, separa o programa de acordo com o anterior e executa, aproveitando
 * para contabilixar o tempo que o programa demorou a executar e para alterar o estado
 * da mensagem para COMPLETED
*/
long parse_and_execute_message(Msg *msg_to_handle, char *folder_path) {
    int number_args;
    char *exec_args[MAX_EXEC_ARGS];
    char *formatter = msg_to_handle->is_pipe ? "|" : " ";

    // este é o primeiro parse, que funcionar para mensagens do tipo pipe ou singular.
    char *tofree = parse_program(msg_to_handle->program, exec_args, formatter, &number_args);

    struct timeval time_before, time_after;
    if (gettimeofday(&time_before, NULL) < 0) {
        perror("[ERROR 15] gettimeofday:");
        exit(EXIT_FAILURE);
    }

    if (msg_to_handle->is_pipe)
        // nesta função, usa-se novamente "parse_program()", mas a string que esta devolve
        // leva um "free" dentro desta função.
        execute_pipe_message(msg_to_handle->pid, exec_args, folder_path, number_args);
    else
        // nesta função não se usa "parse_program()"
        execute_message(msg_to_handle->pid, exec_args, folder_path);

    if (gettimeofday(&time_after, NULL) < 0) {
        perror("[ERROR 16] gettimeofday:");
        exit(EXIT_FAILURE);
    }
    long time_spent = calculate_time_diff(time_before, time_after);

    msg_to_handle->type = COMPLETED;

    free(tofree);

    return time_spent;
}

/**
 * inicializa uma lista de mensagens
*/
void create_messages_list(Msg_list *messages_list, int parallel_tasks) {
    messages_list->scheduled_messages_size = 0;
    messages_list->executing_messages_size = 0;
    messages_list->parallel_tasks = parallel_tasks;
}

/**
 * insere uma mensagem recebida to tipo SCHEDULED na lista de mensagens agendadas
*/
void insert_scheduled_messages_list(Msg_list *messages_list, Msg message_to_insert) {
    if (message_to_insert.type != SCHEDULED) {
        perror("[ERROR 17] message of incorrect type:");
        exit(EXIT_FAILURE);
    }
    messages_list->scheduled_messages[messages_list->scheduled_messages_size++] = message_to_insert;
}

/**
 * retira a primeira mensagem da lista de mensagens agendadas e devolve a mesma
*/
Msg pop_scheduled_messages_list(Msg_list *messages_list) {
    Msg first = messages_list->scheduled_messages[0];

    for (int i = 0; i < messages_list->scheduled_messages_size - 1; i++) messages_list->scheduled_messages[i] = messages_list->scheduled_messages[i + 1];

    messages_list->scheduled_messages_size--;
    return first;
}

/**
 * insere uma mensagem na lista de mensagens a executar
*/
void insert_executing_messages_list(Msg_list *messages_list, Msg *message) {
    message->type = EXECUTING;
    messages_list->executing_messages[messages_list->executing_messages_size++] = *message;
}

/**
 * retira uma mensagem da lista de mensagens a executar
*/
void delete_from_executing_messages_list(Msg_list *messages_list, int pid) {
    int flag = 1;
    for (int i = 0; i < messages_list->executing_messages_size && flag; i++)
        if (messages_list->executing_messages[i].pid == pid) {
            flag = 0;
            for (int j = i; j < messages_list->executing_messages_size - 1; j++) messages_list->executing_messages[j] = messages_list->executing_messages[j + 1];
        }

    messages_list->executing_messages_size--;
}

/**
 * recebe a próxima mensagem na lista de mensagens agendadas. a mensagem candidata
 * é sempre a primeira da lista de mensagens agendadas, uma vez que dá-se sempre,
 * anteriormente, a ordenação da lista de acordo com a política de escalonamento
*/
Msg get_next_executing_message(Msg_list *messages_list) {
    Msg to_execute;
    if (messages_list->parallel_tasks > messages_list->executing_messages_size && messages_list->scheduled_messages_size > 0) {
        to_execute = pop_scheduled_messages_list(messages_list);
        insert_executing_messages_list(messages_list, &to_execute);
    } else
        to_execute.type = ERR;

    return to_execute;
}

/**
 * ordena a lista de mensagens agendadas, por ordem crescente, de
 * acordo com o tempo previsto que o programa demorará a executar.
*/
void sort_by_SJF(Msg *scheduled_messages, int size) {
    for (int i = 0; i < size; i++) {
        for (int j = i; j < size; j++) {
            if (scheduled_messages[i].time > scheduled_messages[j].time) {
                Msg temp = scheduled_messages[i];
                scheduled_messages[i] = scheduled_messages[j];
                scheduled_messages[j] = temp;
            }
        }
    }
}

/*---------FUNÇÕES AUXILIARES ORCHESTRATOR---------*/

/**
 * dado um array de mensagens to tipo Msg, cria mensagens to tipo 
 * Msg_to_print, para facilitar no envio de informação de mensagens
 * para o cliente
*/
void send_messages(Msg list[], int list_size, int outgoing_fd) {
    Msg incoming_msg;
    Msg_to_print outgoing_msg;

    for (int i = 0; i < list_size; i++) {
        incoming_msg = list[i];

        create_message_to_print(&outgoing_msg, incoming_msg.pid, -1, incoming_msg.program, PROGRAM_ID);
        write_file(outgoing_fd, &outgoing_msg, sizeof(Msg_to_print));
    }
}

/**
 * lê do ficheiro partilhado a informação guardada das tarefas concluídas,
 * criando mensagens to tipo Msg_to_print para facilitar no envio de informação
 * para o cliente
*/
void read_and_send_messages(char *shared_file_path, int outgoing_fd) {
    // usa-se a flag O_CREAT no caso deste ficheiro ainda não ter sido aberto
    int incoming_fd = open_file(shared_file_path, O_RDONLY | O_CREAT, 0777);

    Msg_to_print incoming_and_outgoing_msg;
    while (read_file(incoming_fd, &incoming_and_outgoing_msg, sizeof(Msg_to_print)) > 0) {
        write_file(outgoing_fd, &incoming_and_outgoing_msg, sizeof(Msg_to_print));
    }

    close_file(incoming_fd);
}

/**
 * envia um estado geral das tarefas agendadas, a executar e concluídas
 * para o cliente. faz uso das últimas 2 funções e, adicionalmente, envia mensagens
 * do tipo Msg_to_print que apenas contêm strings "delimitadoras" do estado das mensagens. 
*/
void send_status_to_client(Msg_list messages, int message_pid, char *shared_file_path) {
    int outgoing_fd = open_file_pid(message_pid, O_WRONLY, 0);

    Msg_to_print msg_to_send;
    create_message_to_print(&msg_to_send, -1, -1, "Executing\n", TEXT);

    write_file(outgoing_fd, &msg_to_send, sizeof(Msg_to_print));

    send_messages(messages.executing_messages, messages.executing_messages_size, outgoing_fd);

    create_message_to_print(&msg_to_send, -1, -1, "\nScheduled\n", TEXT);
    write_file(outgoing_fd, &msg_to_send, sizeof(Msg_to_print));

    send_messages(messages.scheduled_messages, messages.scheduled_messages_size, outgoing_fd);

    create_message_to_print(&msg_to_send, -1, -1, "\nCompleted\n", TEXT);
    write_file(outgoing_fd, &msg_to_send, sizeof(Msg_to_print));

    read_and_send_messages(shared_file_path, outgoing_fd);

    close_file(outgoing_fd);
}

/**
 * envia, através do fifo temporário e único criado pelo cliente, o id
 * da tarefa
*/
void send_task_number_to_client(int message_pid) {
    int outgoing_fd = open_file_pid(message_pid, O_WRONLY, 0);
    write_file(outgoing_fd, &message_pid, sizeof(message_pid));
    close_file(outgoing_fd);
}

/**
 * escreve, no mesmo ficheiro, informação relativa ao id, programa e tempo
 * de execução das tarefas concluídas
*/
void write_time_spent(char *shared_file_path, Msg msg_to_write, long time_spent) {
    int shared_fd = open_file(shared_file_path, O_WRONLY | O_APPEND | O_CREAT, 0777);

    Msg_to_print outgoing_msg;
    create_message_to_print(&outgoing_msg, msg_to_write.pid, time_spent, msg_to_write.program, PROGRAM_ID_TIMESPENT);

    write_file(shared_fd, &outgoing_msg, sizeof(Msg_to_print));
    close_file(shared_fd);
}



/*---------FUNÇÕES AUXILIARES CLIENT---------*/


/**
 * dada uma mensagem pronta a enviar, envia para o destino fixo
 * definido por MAIN_FIFO_NAME.
*/
void send_message_to_server(Msg msg_to_send) {
    int outgoing_fd = open_file(MAIN_FIFO_NAME, O_WRONLY, 0);
    write_file(outgoing_fd, &msg_to_send, sizeof(Msg));
    close_file(outgoing_fd);
}

/**
 * dado o caminho para o fifo temporário e único já criado,
 * é recebido um inteiro correspondente ao id da tarefa
 * atribuido pelo servidor
*/
void receive_and_print_tasknum(char *server_to_client_fifo) {
    // abrir fifo para receber o id da tarefa
    int tasknum;
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);
    read_file(incoming_fd, &tasknum, sizeof(tasknum));
    close_file(incoming_fd);

    char tasknum_buffer[50];
    if (sprintf(tasknum_buffer, "TASK %d Received\n", tasknum) < 0) {
        perror("[ERROR 1] sprintf:");
        exit(EXIT_FAILURE);
    }
    write_file(STDOUT_FILENO, tasknum_buffer, strlen(tasknum_buffer));
}

/**
 * dado o caminho para o fifo temporário e único já criado,
 * é recebido um conjunto de mensagens prontas a serem imprimidas 
*/
void receive_and_print_status(char *server_to_client_fifo) {
    // abrir fifo para receber o status
    int incoming_fd = open_file(server_to_client_fifo, O_RDONLY, 0);

    Msg_to_print incoming_msg;
    char status_buffer[MAX_STRING_SIZE];
    while (read_file(incoming_fd, &incoming_msg, sizeof(Msg_to_print)) > 0) {
        if (incoming_msg.type != TEXT && incoming_msg.type != PROGRAM_ID && incoming_msg.type != PROGRAM_ID_TIMESPENT) {
            perror("[ERROR 2] message of incorrect type:");
            exit(EXIT_FAILURE);
        }

        // a mensagem apenas inclui texto
        if (incoming_msg.type == TEXT) {
            if (sprintf(status_buffer, "%s", incoming_msg.program) < 0) {
                perror("[ERROR 3] sprintf:");
                exit(EXIT_FAILURE);
            }
        }
        // a mensagem inclui o id da tarefa e o programa da mesma
        else if (incoming_msg.type == PROGRAM_ID) {
            if (sprintf(status_buffer, "%d %s\n", incoming_msg.pid, incoming_msg.program) < 0) {
                perror("[ERROR 4] sprintf:");
                exit(EXIT_FAILURE);
            }
        }
        // a mensagem inclui o id da tarefa, o programa da mesma e o tempo que demorou a executar
        else {
            if (sprintf(status_buffer, "%d %s %ld ms\n", incoming_msg.pid, incoming_msg.program, incoming_msg.time_spent) < 0) {
                perror("[ERROR 5] sprintf:");
                exit(EXIT_FAILURE);
            }
        }

        write_file(STDOUT_FILENO, status_buffer, strlen(status_buffer));
    }

    close_file(incoming_fd);
}
