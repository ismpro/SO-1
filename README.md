# Projeto de Otimização em C

Este projeto contém várias implementações em C para resolver problema de distancia, utilizando diferentes abordagens e técnicas de processamento paralelo.

## Estrutura do Projeto

O projeto é composto pelos seguintes ficheiros principais:

- `main_brute.c`: Implementação do método de força bruta.
- `main.c`: Versão básica do programa com processos paralelos.
- `main_advance.c`: Versão avançada com comunicação entre processos.
- `main_basic.c`: Versão básica com múltiplos processos e memória compartilhada.

### main_brute.c

**Descrição:**
`main_brute.c` é uma implementação em C do método de força bruta. Este script lê dados de um ficheiro de texto especificado, processa-os e tenta encontrar a melhor solução possível através de permutações aleatórias. Destina-se a ser executado com dois argumentos: o nome do ficheiro e o tempo máximo de execução.

**Funcionalidades:**
- **Leitura de Ficheiros:** Lê matrizes de dados de ficheiros `.txt` localizados numa pasta chamada 'tests'.
- **Processamento de Dados:** Calcula distâncias usando uma matriz de dados fornecida e permuta os dados para encontrar um caminho ótimo.
- **Resultados:** Exibe a menor distância encontrada, o melhor caminho correspondente, o número de iterações realizadas, e o tempo de execução.

**Métodos Principais:**
- `int distance(int size, int path[size], int matrix[size][size])`: Calcula a distância total de um caminho específico usando a matriz fornecida.
- `void swap(int size, int path[size])`: Troca dois elementos aleatórios no caminho para gerar uma nova permutação.
- `void shuffle(int *array, size_t n)`: Embaralha um array para gerar uma ordem aleatória inicial.

**Uso:**
```bash
./main_brute [nome_do_ficheiro] [tempo_maximo]
```

Onde `[nome_do_ficheiro]` é o nome do ficheiro de entrada (sem a extensão .txt) e `[tempo_maximo]` é o tempo máximo de execução em segundos.

### main.c

**Descrição:**
`main.c` é um programa em C que implementa uma solução básica para um problema de otimização usando processos paralelos. O programa lê uma matriz de distâncias de um ficheiro .txt, executa cálculos de distância, e tenta melhorar iterativamente um caminho dado. A execução é baseada em tempo e número de processos especificados, com um limite opcional de iterações sem melhorias.

**Funcionalidades:**

- **Leitura de Ficheiros:** Carrega matrizes de distância de ficheiros .txt dentro da pasta 'tests'.
- **Cálculo de Distância:** Calcula a distância total de um percurso usando uma matriz de distâncias.
- **Paralelismo:** Utiliza processos filhos para executar cálculos em paralelo, melhorando o tempo de execução.
- **Monitorização:** Apresenta uma barra de progresso e estatísticas de execução em tempo real.

**Métodos Principais:**

- `int distance(int size, int path[size], int matrix[size][size])`: Calcula a distância total de um percurso.
- `void swap(int size, int path[size])`: Troca dois elementos aleatórios no percurso para gerar uma nova permutação.
- `void shuffle(int *array, int n)`: Embaralha um array para gerar uma ordem inicial aleatória de caminho.*array, int n): Embaralha um array para gerar uma ordem inicial aleatória de caminho.

**Uso:**
```bash
./main [nome_do_ficheiro] [numero_de_processos] [tempo_total_de_execucao] [limite_iteracao_opcional]
```

**Argumentos:**

- `[nome_do_ficheiro]`: Nome do ficheiro de entrada (sem a extensão .txt).
- `[numero_de_processos]`: Número de processos filhos a serem utilizados.
- `[tempo_total_de_execucao]`: Tempo máximo de execução em segundos.
- `[limite_iteracao_opcional]`: Limite para iteração sem encontrar um melhor caminho (opcional).

### main_advance.c

**Descrição:**
`main_advance.c` é uma implementação avançada em C que utiliza processos paralelos e comunicação entre processos para encontrar soluções otimizadas para um problema de roteamento. O script lê uma matriz de distâncias de um ficheiro `.txt` na pasta 'tests' e utiliza técnicas de permutação aleatória e comparação para encontrar o caminho mais curto.

**Funcionalidades:**
- **Paralelismo e Sincronização:** Utiliza múltiplos processos e semáforos para explorar soluções paralelamente.
- **Comunicação entre Processos:** Usa sinais para comunicar entre processos pai e filhos, permitindo atualizações rápidas da melhor solução encontrada.
- **Melhorias Dinâmicas:** Atualiza continuamente o melhor caminho encontrado e a distância correspondente em tempo real.

**Métodos Principais:**
- `int distance(int size, int path[], int matrix[][])`: Calcula a distância total para um determinado caminho.
- `void swap(int size, int path[])`: Troca dois elementos no caminho para gerar uma nova permutação.
- `void shuffle(int *array, size_t n)`: Embaralha um array para iniciar a busca com um caminho aleatório.
- `void parent_callback(int signal)`: Callback do processo pai para receber sinais dos processos filhos.
- `void child_callback(int signal)`: Callback dos processos filhos para atualizar o caminho para o melhor encontrado.

**Uso:**
```bash
./main_advance [nome_do_ficheiro] [numero_de_processos] [tempo_total_de_execucao]
```

**Argumentos:**

- `[nome_do_ficheiro]`: Nome do ficheiro de entrada (sem a extensão .txt).
- `[numero_de_processos]`: Número de processos filhos a serem utilizados para a busca paralela.
- `[tempo_total_de_execucao]`: Tempo máximo de execução em segundos.

### main_basic.c

**Descrição:**
`main_basic.c` é uma implementação básica em C que resolve problemas de otimização usando múltiplos processos e memória compartilhada. O programa lê uma matriz de distâncias de um ficheiro .txt na pasta 'tests', e utiliza algoritmos de permutação e comparação para encontrar o caminho mais curto.

**Funcionalidades:**

- **Processamento Paralelo:** Cria múltiplos processos para executar cálculos em paralelo e melhorar a eficiência.
- **Memória Compartilhada:** Utiliza memória compartilhada para armazenar a melhor distância e o melhor caminho encontrados, permitindo a comunicação entre processos.
- **Controle de Tempo:** Encerra a execução após um tempo máximo especificado, retornando o melhor resultado encontrado até então.

**Métodos Principais:**

- `int distance(int size, int path[], int matrix[][])`: Calcula a distância total para um caminho específico.
- `void shuffle(int *array, size_t n)`: Embaralha um array para gerar uma ordem inicial aleatória de caminho.
- `void swap(int size, int path[])`: Troca dois elementos no caminho para gerar uma nova permutação.

**Uso:**
```bash
./main_basic [nome_do_ficheiro] [numero_de_processos] [tempo_total_de_execucao]
```

**Argumentos:**

- `[nome_do_ficheiro]`: Nome do ficheiro de entrada (sem a extensão .txt).
- `[numero_de_processos]`: Número de processos filhos a serem utilizados para a busca paralela.
- `[tempo_total_de_execucao]`: Tempo máximo de execução em segundos.

**Como Executar**

Para executar qualquer um dos programas, use o seguinte comando no terminal:


```bash
./[nome_do_programa] [nome_do_ficheiro] [outros_argumentos]
```

Substitute `[nome_do_programa]` with the name of the file you want to run, `[nome_do_ficheiro]` with the name of the input file (located in the 'tests' folder), and `[outros_argumentos]` as necessary for each specific program.

**License**

MIT License
