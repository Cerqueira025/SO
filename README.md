# SO
Trabalho Prático - Orquestrador de Tarefas

TODO:
```
- RESOLVER PROBLEMA STATUS
- dar free do toFree de cada parse, dentro do ciclo for, no execute_pipe_message
- Ver casos de exceção
- rever exit(EXIT_FAILURE)
- melhorar script para testes

-- FACULTATIVOS
- antes de enviar mensagem para o servidor, verificar se a string que o cliente envia faz sentido. se o cliente enviar "ls | wc" mas disse que era -u então está errado. o mesmo aplica-se ao contrário. (NÃO BASTA O ERRO NO EXEC, POR DESCONHECER A SINTAXE, UMA VEZ QUE A SEPARAÇÃO DA STRING VAI CORRER MAL?)
- tentar colocar os redirecionamentos (dup2) apenas dentro do processo filho, na função execute_message, para não haver necessidade de redirecionar de volta para os descritores originais
- Verificar/mudar organização de funções
- ver todas as funções que devolvem valores

DÚVIDA - pastas .bin
DÚVIDA - criar também preentive priority
DÚVIDA - que versão do c usar para livar dos warnings?
DÚVIDA - devemos colocar aqui (utils.h) struct timeval para eliminar o erro na função calculcate_time_diff??

```
