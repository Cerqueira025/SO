# SO
Trabalho Prático - Orquestrador de Tarefas

TODO:
```
- antes de enviar mensagem para o servidor, verificar se a string que o cliente envia faz sentido. se o cliente
enviar "ls | wc" mas disse que era -u então está errado. o mesmo aplica-se ao contrário. (NÃO BASTA O ERRO NO EXEC, POR DESCONHECER A SINTAXE, UMA VEZ QUE A SEPARAÇÃO DA STRING VAI CORRER MAL?)
- verificar os includes. nunca houve client.h??
- fazer política de escalonamento (FEITO - FALTA TESTAR)
- dar free do toFree de cada parse, dentro do ciclo for, no execute_pipe_message
- tentar colocar os redirecionamentos (dup2) apenas dentro do processo filho, na função execute_message, para não haver necessidade de redirecionar de volta para os descritores originais
- my_printf
- Criar script para testes
- Verificar se TAREFA xxxx RECEBIDA acontece tanto com -p como com -u

- Ver ponto Makefile
- Ver ponto Avaliação de políticas de escalonamento
- Mudar organização de funções
- Ver casos de exceção

DÚVIDA - pastas .bin

```
