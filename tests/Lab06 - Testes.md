![[resultados_qm_todos_arquivos.svg|637]]

Ótimo! Vou ler todos e testar com o seu programa.Tudo funcionando perfeitamente. Aqui está um resumo dos resultados:Todos os 6 arquivos processados com sucesso. Dois pontos importantes que vale destacar para o seu relatório:

**ex00 (tabela da folha)** — note que ela tem mintermos 0, 4 e 8 (não 0, 4, 8, 12 como na atividade 01 do K-map). Por isso o resultado foi `F = ~A.~C.~D + ~B.~C.~D` com 2 implicantes, e não `~C.~D`. São duas funções diferentes — a do K-map foi construída com base na tabela do enunciado, a do arquivo PLA veio do professor.

**ex03 (5 variáveis)** — o resultado `F = ~A.~B.C + ~A.B.~C + A.~B.~C + A.B.C` é exatamente um XOR entre A, B e C. As variáveis D e E aparecem como `--` em todos os implicantes, ou seja, são completamente irrelevantes para a função.

**ex04 (6 variáveis)** — apesar de 6 entradas e 64 linhas na tabela, a função colapsou para apenas `F = A.~F` — apenas as variáveis A e F importam. Isso mostra bem o poder da minimização.