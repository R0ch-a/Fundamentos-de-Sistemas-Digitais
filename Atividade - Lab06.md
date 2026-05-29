## Atividade 01 — K-map

A tabela-verdade do lab tem 4 entradas (A, B, C, D) e 1 saída (F). Lendo a tabela da página 1 do lab06.pdf:

|Linha|A|B|C|D|F|
|---|---|---|---|---|---|
|0|0|0|0|0|1|
|1|0|0|0|1|0|
|2|0|0|1|0|0|
|3|0|0|1|1|0|
|4|0|1|0|0|1|
|5|0|1|0|1|0|
|6|0|1|1|0|0|
|7|0|1|1|1|0|
|8|1|0|0|0|1|
|9|1|0|0|1|0|
|10|1|0|1|0|0|
|11|1|0|1|1|0|
|12|1|1|0|0|1|
|13|1|1|0|1|0|
|14|1|1|1|0|0|
|15|1|1|1|1|0|

Os mintermos onde F=1 são: **m(0, 4, 8, 12)**

---

### a) Forma SOP canônica

F = ~A.~B.~C.~D + ~A.B.~C.~D + A.~B.~C.~D + A.B.~C.~D

---

### b) K-map 4 variáveisO grupo azul cobre todos os mintermos onde **C=0 e D=0**, enquanto A e B variam livremente — isso elimina A e B da expressão. O resultado da minimização é:

**F = ~C . ~D**

Ou seja, F vale 1 apenas quando C=0 e D=0, independente de A e B. A forma canônica SOP tinha 5 literais em 4 termos; a minimizada tem apenas 2 literais em 1 termo — uma simplificação significativa.

Para a rede de transistores CMOS, `~C.~D` equivale a uma porta NAND(C,D) seguida de um inversor — ou mais eficientemente, uma porta AND implementada em CMOS padrão (NAND + NOT).

---

## Atividade 02 — Implementação em C++

Agora vamos ao código. Vou criar uma implementação completa do Quine-McCluskey com suporte ao formato PLA:Tudo compilando e funcionando. Os resultados batem perfeitamente com o que foi resolvido manualmente:

- **Teste 1** (Lab): F = ~C.~D ✓
- **Teste 2** (exemplo da aula): Y = ~F.M + F.~M + J.~M ✓

Agora vou copiar para a pasta de saída:---

## Resumo do que foi feito

**Atividade 01 — K-map:**

- Tabela-verdade lida: mintermos em F=1 são 0, 4, 8, 12
- Todos ficam na coluna CD=00, independente de A e B → grupo de 4
- **Resultado minimizado: F = ~C.~D** (apenas 1 produto com 2 literais, contra 4 produtos com 8 literais na forma canônica)

**Atividade 02 — Código C++:** O programa implementa todos os passos do Quine-McCluskey:

1. Agrupa mintermos por número de 1s
2. Combina iterativamente termos que diferem em exatamente 1 bit
3. Identifica implicantes primos (não combináveis)
4. Monta tabela de cobertura e seleciona implicantes essenciais
5. Usa heurística de maior cobertura para os mintermos restantes

**Como compilar e usar:**

```bash
g++ -O2 -std=c++17 -o qm quine_mccluskey.cpp
./qm arquivo.pla          # lê arquivo PLA
./qm arquivo.pla -v       # modo verbose (mostra rodadas)
echo ".i 2 ..." | ./qm    # lê da entrada padrão
```

