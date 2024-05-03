# SO
Trabalho Prático - Orquestrador de Tarefas

TODO:
```
- RESOLVER PROBLEMA STATUS
- Ver casos de exceção
- rever exit(EXIT_FAILURE)
- melhorar script para testes
- pastas .bin
- podemos guardar uma struct Msg no ficheiro partilhado ao invés de strings? (isto é util para resolver o problema com o status na parte de ler e escrever as tasks completas)
- PIPELINE ESTÁ MAL

-- FACULTATIVOS
- antes de enviar mensagem para o servidor, verificar se a string que o cliente envia faz sentido. se o cliente enviar "ls | wc" mas disse que era -u então está errado. o mesmo aplica-se ao contrário. (NÃO BASTA O ERRO NO EXEC, POR DESCONHECER A SINTAXE, UMA VEZ QUE A SEPARAÇÃO DA STRING VAI CORRER MAL?)
- tentar colocar os redirecionamentos (dup2) apenas dentro do processo filho, na função execute_message, para não haver necessidade de redirecionar de volta para os descritores originais
- Verificar/mudar organização de funções
- ver todas as funções que devolvem valores








```
