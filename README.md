# SO
Trabalho Prático - Orquestrador de Tarefas

TODO:
```
- redirecionar STDERR na execução de tarefas em pipeline + rever se o stderr está a ser redirecionado.
- SUBSTITUIR PRINTF
- dar free do toFree de cada parse, dentro do ciclo for, no execute_pipe_message
- tentar colocar os redirecionamentos (dup2) apenas dentro do processo filho, na função execute_message, para não haver necessidade de redirecionar de volta para os descritores originais
- Ver casos de exceção e todas as funções que devolvem valores
- antes de enviar mensagem para o servidor, verificar se a string que o cliente envia faz sentido. se o cliente enviar "ls | wc" mas disse que era -u então está errado. o mesmo aplica-se ao contrário. (NÃO BASTA O ERRO NO EXEC, POR DESCONHECER A SINTAXE, UMA VEZ QUE A SEPARAÇÃO DA STRING VAI CORRER MAL?)
- melhorar script para testes
- Verificar/mudar organização de funções

DÚVIDA - pastas .bin
DÚVIDA - criar também preentive priority
DÚVIDA - que versão do c usar para livar dos warnings?

```
