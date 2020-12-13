/*
*	Versão Basica
*	Argumentos:
*	1 - Nome do ficheiro
*	2 - Numero de processos
*	3 - Tempo Total de execução
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <ctype.h>

#define MILHAO 1000000L;

//Calcular distancias apartir de uma matriz
int distance(int size, int path[size], int matrix[size][size])
{
	int dist = 0;

	for (int i = 0; i < size - 1; i++)
	{

		int curr = path[i] - 1;
		int next = path[i + 1] - 1;
		dist += matrix[curr][next];
	}

	int last = path[size - 1] - 1;
	int first = path[0] - 1;
	dist += matrix[last][first];

	return dist;
}

//Retirar espaços extras de strings
void trim(char *string)
{
	int i, j;
	for (i = j = 0; string[i]; ++i)
		if (!isspace(string[i]) || (i > 0 && !isspace(string[i - 1])))
			string[j++] = string[i];
	string[j] = '\0';
}

//Trocar aleatoriamente dois posições do array
void swap(int size, int path[size])
{
	int a = rand() % size;
	int b = rand() % size;
	int tmp = path[a];
	path[a] = path[b];
	path[b] = tmp;
}

//Baralhar aleatoriamente os arrays
void shuffle(int *array, size_t n)
{
	if (n > 1)
	{
		size_t i;
		for (i = 0; i < n - 1; i++)
		{
			size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
			int t = array[j];
			array[j] = array[i];
			array[i] = t;
		}
	}
}

int main(int argc, char *argv[])
{

	//Processamento de agurmentos
	if (argc != 4)
	{
		printf("Aborting due to lack of arguments\n");
		return (EXIT_FAILURE);
	}

	char pathFile[] = "tests/"; //[OBRIGATORIO] - Nome do Ficheiro
	strcat(pathFile, argv[1]);
	strcat(pathFile, ".txt");
	int num_proc = atoi(argv[2]); //[OBRIGATORIO] - Numero de processos filhos
	int max_time = atoi(argv[3]); //[OBRIGATORIO] - Tempo maximo de execusão

	//Começo do tempo
	struct timespec begin, timer, final, randNum;
	clock_gettime(CLOCK_REALTIME, &begin);

	//Leitura dos ficheiros para a criação de variaveis
	FILE *file;
	char string[1000];

	file = fopen(pathFile, "r");
	if (file == NULL)
	{
		printf("Could not open file %s", pathFile);
		return 1;
	}

	int size = atoi(fgets(string, 1000, file));
	int matrix[size][size];

	int line = 0;
	while (fgets(string, 1000, file) != NULL)
	{

		trim(string);

		char s[2] = " ";
		char *token;
		int col = 0;

		/* get the first token */
		token = strtok(string, s);

		/* walk through other tokens */
		while (token != NULL)
		{
			matrix[line][col] = atoi(token);
			token = strtok(NULL, s);
			col = col + 1;
		}
		line = line + 1;
	}
	fclose(file);

	//Declaração de algumas variaveis
	int path[size];
	for (int i = 0; i < size; i++)
	{
		path[i] = i + 1;
	}

	//Declaração de semaforos
	sem_unlink("access_mem");
	sem_t *access_mem = sem_open("access_mem", O_CREAT, 0644, 1);

	//Declaração de memoria patrilhada
	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_ANONYMOUS | MAP_SHARED;

	int *best_dist = mmap(NULL, sizeof(int), protection, visibility, 0, 0);
	int *best_path = mmap(NULL, sizeof(int) * 5, protection, visibility, 0, 0);
	long *it = mmap(NULL, sizeof(long), protection, visibility, 0, 0);
	double *best_time = mmap(NULL, sizeof(double), protection, visibility, 0, 0);
	long *best_it = mmap(NULL, sizeof(long), protection, visibility, 0, 0);

	*best_dist = 99999;

	int pids[num_proc];

	for (int i = 0; i < num_proc; i++)
	{
		/* 
		* Codigo de execução dos filhos
		* Calcula a distancia e verifica se já encontrada uma melhor
		*/
		pids[i] = fork();
		if (pids[i] == 0)
		{
			clock_gettime(CLOCK_REALTIME, &randNum);
			srand(randNum.tv_nsec);
			shuffle(path, size);

			while (1)
			{
				swap(size, path);
				int dist = distance(size, path, matrix);
				sem_wait(access_mem);
				*it = *it + 1;
				if (dist < *best_dist)
				{
					clock_gettime(CLOCK_REALTIME, &timer);
					*best_dist = dist;
					*best_it = *it;
					*best_time = ((timer.tv_sec - begin.tv_sec) * 1000) +
								 (double)(timer.tv_nsec - begin.tv_nsec) / (double)MILHAO;
					for (int z = 0; z < size; z++)
					{
						best_path[z] = path[z];
					}
				}
				sem_post(access_mem);
			}
			exit(0);
		}
	}

	sleep(max_time);

	// Kill worker processes
	for (int i = 0; i < num_proc; i++)
	{
		kill(pids[i], SIGKILL);
	}

	clock_gettime(CLOCK_REALTIME, &final);
	double ttime = ((final.tv_sec - begin.tv_sec) * 1000) +
				   (double)(final.tv_nsec - begin.tv_nsec) / (double)MILHAO;

	//Tabela de Resultados
	printf("\n------------------Table------------------\n\n");
	printf("\nDistance: %d\n\n", *best_dist);
	printf("Best Path -");
	for (int i = 0; i < size; i++)
	{
		printf(" %d ", best_path[i]);
	}
	printf("\n\nIterations: %ld", *it);

	printf("\n\nBest Iteration: %ld", *best_it);

	printf("\n\nTotal Time: %.1f ms", ttime);

	printf("\n\nBest Time: %.1f ms\n\n", *best_time);
	printf("\n\n-----------------------------------------\n\n");

	sem_close(access_mem);

	return 0;
}