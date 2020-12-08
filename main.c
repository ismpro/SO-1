#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>

#define MILHAO 1000000L;
#define SHUFFLETIMES 3;

int *path;
int *pids;
int *best_path;
int *best_dist;
long *it;
long *best_it;
int size;
int workStatus;
int num_proc;
int hitThreshold;
sem_t *access_mem;

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

void swap(int size, int path[size])
{
	int a = rand() % size;
	int b = rand() % size;
	int tmp = path[a];
	path[a] = path[b];
	path[b] = tmp;
}

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

void parent_callback(int signal)
{
	hitThreshold = 0;
	for (int i = 0; i < num_proc; i++)
	{
		kill(pids[i], SIGUSR2);
	}
	sem_post(access_mem);
	return;
}

void child_callback(int signal)
{
	for (int z = 0; z < size; z++)
	{
		path[z] = best_path[z];
	}
	return;
}

void order_shuffle(int signal)
{
	for (int i = 0; i < SHUFFLETIMES i++)
	{
		shuffle(path, size);
	}
	return;
}

int main(int argc, char *argv[])
{

	//Processamento de agurmentos
	int threshold; //[OPCIONAL] -

	if (argc <= 3 && argc >= 4)
	{
		printf("Aborting due to lack of arguments\n");
		return (EXIT_FAILURE);
	}
	if (argc == 3)
	{
		threshold = 0;
	}
	else
	{
		threshold = atoi(argv[3]);
	}
	num_proc = atoi(argv[1]);	  //[OBRIGATORIO] - Numero de processos filhos
	int max_time = atoi(argv[2]); //[OBRIGATORIO] - Tempo maximo de execusão

	//Declaração de algumas variaveis
	hitThreshold = 0;

	//Para tirar depois de ter os ficheiros
	/*+------------------------------+*/
	size = 5;

	int matrix[5][5] = {
		{0, 23, 10, 4, 1},
		{23, 0, 9, 5, 4},
		{10, 9, 0, 8, 2},
		{4, 5, 8, 0, 11},
		{1, 4, 2, 11, 0},
	};
	/*+------------------------------+*/
	pids = (int *)malloc(sizeof(int) * num_proc);

	path = (int *)malloc(sizeof(int) * size);
	for (int i = 0; i < size; i++)
	{
		path[i] = i + 1;
	}

	struct timespec begin, timer, randNum;
	clock_gettime(CLOCK_REALTIME, &begin);
	signal(SIGUSR1, parent_callback);

	//Declaração de semaforos
	sem_unlink("access_mem");
	access_mem = sem_open("access_mem", O_CREAT, 0644, 1);

	//Declaração de memoria patrilhada
	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_ANONYMOUS | MAP_SHARED;

	best_dist = mmap(NULL, sizeof(int), protection, visibility, 0, 0);
	best_path = mmap(NULL, sizeof(int) * 5, protection, visibility, 0, 0);
	it = mmap(NULL, sizeof(long), protection, visibility, 0, 0);
	double *best_time = mmap(NULL, sizeof(double), protection, visibility, 0, 0);
	best_it = mmap(NULL, sizeof(long), protection, visibility, 0, 0);

	*best_dist = 99999;

	for (int i = 0; i < num_proc; i++)
	{
		pids[i] = fork();
		if (pids[i] == 0)
		{
			signal(SIGUSR2, child_callback);
			signal(SIGINT, order_shuffle);
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
					*best_time = (timer.tv_sec - begin.tv_sec) +
								 (double)(timer.tv_nsec - begin.tv_nsec) / (double)MILHAO;
					for (int z = 0; z < size; z++)
					{
						best_path[z] = path[z];
					}
					kill(getppid(), SIGUSR1);
				}
				else
				{
					sem_post(access_mem);
				}
			}
			exit(0);
		}
	}

	//Program LOOP
	while (time(NULL) - begin.tv_sec < max_time)
	{
		if (threshold != 0 && *it - *best_it >= threshold)
		{
			hitThreshold++;
			if (hitThreshold <= 3)
			{
				printf("\n\nKilling process due hiting 3 times the threshold\n\n");
				break;
			}
			else
			{
				for (int i = 0; i < num_proc; i++)
				{
					kill(pids[i], SIGINT);
				}
			}
		}
	}

	// Kill worker processes
	for (int i = 0; i < num_proc; i++)
	{
		kill(pids[i], SIGKILL);
	}

	//Tabela de Resultados
	printf("\nDistance: %d\n\n", *best_dist);
	printf("Best Path -");
	for (int i = 0; i < size; i++)
	{
		printf(" %d ", best_path[i]);
	}
	printf("\n\nIterations: %ld", *it);

	printf("\n\nBest Iteration: %ld", *best_it);

	printf("\n\nBest Time: %.2f ms\n\n", *best_time);

	//Libertar memoria e fechar semaforo
	free(path);
	free(pids);
	sem_close(access_mem);

	return EXIT_SUCCESS;
}