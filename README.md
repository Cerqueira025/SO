# SO
Trabalho Prático - Orquestrador de Tarefas

TODO:
```
- alterar mode que os open_files recebem. trocar de 0 para -1. 0 é capaz de ser mode aceitável (FEITO - FALTA TESTAR)
- antes de enviar mensagem para o servidor, verificar se a string que o cliente envia faz sentido. se o cliente
enviar "ls | wc" mas disse que era -u então está errado. o mesmo aplica-se ao contrário. (NÃO BASTA O ERRO NO EXEC, POR DESCONHECER A SINTAXE, UMA VEZ QUE A SEPARAÇÃO DA STRING VAI CORRER MAL?)
- verificar os includes. nunca houve client.h??
- fazer política de escalonamento (FEITO - FALTA TESTAR)
- dar free do toFree de cada parse, dentro do ciclo for, no execute_pipe_message
- tentar colocar os redirecionamentos apenas dentro do processo filho, na função execute_message, para não haver necessidade de redirecionar de volta para os descritores originais

DÚVIDA - pastas .bin

```
