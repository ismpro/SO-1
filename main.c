#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

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

void trim(char *string)
{
	int i, j;
	for (i = j = 0; string[i]; ++i)
		if (!isspace(string[i]) || (i > 0 && !isspace(string[i - 1])))
			string[j++] = string[i];
	string[j] = '\0';
}

void print_progress(size_t count, size_t max)
{
	const char prefix[] = "Progress: [";
	const char suffix[] = "]";
	const size_t prefix_length = sizeof(prefix) - 1;
	const size_t suffix_length = sizeof(suffix) - 1;
	char *buffer = calloc(max + prefix_length + suffix_length + 1, 1); // +1 for \0
	size_t i = 0;

	strcpy(buffer, prefix);
	for (; i < max; ++i)
	{
		buffer[prefix_length + i] = i < count ? '#' : ' ';
	}

	strcpy(&buffer[prefix_length + i], suffix);
	printf("\b%c[2K\r%s\n", 27, buffer);
	fflush(stdout);
	free(buffer);
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
	int threshold; //[OPCIONAL] - ???????????

	if (argc <= 4 && argc >= 5)
	{
		printf("Aborting due to lack of arguments\n");
		return (EXIT_FAILURE);
	}
	if (argc == 4)
	{
		threshold = 0;
	}
	else
	{
		threshold = atoi(argv[4]);
	}
	char[] path = "tests/" + argv[1]; //[OBRIGATORIO] - Nome do Ficheiro
	num_proc = atoi(argv[2]);		  //[OBRIGATORIO] - Numero de processos filhos
	int max_time = atoi(argv[3]);	  //[OBRIGATORIO] - Tempo maximo de execusão

	//Começo do tempo
	struct timespec begin, timer, randNum, final;
	clock_gettime(CLOCK_REALTIME, &begin);
	signal(SIGUSR1, parent_callback);

	//Declaração de algumas variaveis
	hitThreshold = 0;

	int firstRow = 1;
	int matrix[5][5];

	FILE *file;
	char string[1000];

	file = fopen(path, "r");
	if (file == NULL)
	{
		printf("Could not open file %s", path);
		return 1;
	}

	int line = 0;
	while (fgets(string, 1000, file) != NULL)
	{
		if (firstRow)
		{
			size = atoi(string);
			matrix[size][size];
			firstRow = 0;
		}
		else
		{
			trim(string);
			int col = 0;
			for (int i = 0; string[i] != '\0'; i++)
			{
				int z = 0;
				for (int j = i; string[j] != ' ' && string[j] != '\0'; j++)
				{
					z = z + 1;
				}
				char number[z];
				int y = i;
				for (int x = 0; x < z; x++)
				{
					number[x] = string[y];
					y = y + 1;
				}
				matrix[line][col] = atoi(number);
				col = col + 1;

				i = i + z;
			}
		}
		line = line + 1;
	}
	fclose(file);

	pids = (int *)malloc(sizeof(int) * num_proc);

	path = (int *)malloc(sizeof(int) * size);
	for (int i = 0; i < size; i++)
	{
		path[i] = i + 1;
	}

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

	printf("\rIn progress %d", i / 100);

	printf("\e[?25l");

	//Tempo progresso e tempo total
	clock_gettime(CLOCK_REALTIME, &final);
	double time = (final.tv_sec - begin.tv_sec) +
				  (double)(final.tv_nsec - begin.tv_nsec) / (double)MILHAO;

	double lastTime = 0;

	//Program LOOP
	while (time(NULL) - begin.tv_sec < max_time)
	{
		clock_gettime(CLOCK_REALTIME, &final);
		time = (final.tv_sec - begin.tv_sec) +
			   (double)(final.tv_nsec - begin.tv_nsec) / (double)MILHAO;

		if (time != lastTime)
		{
			lastTime = time;
			printf("\n\tIn progress %d\n\t[", time);
			print_progress(time * 1000, max_time);
		}

		if (threshold != 0 && *it - *best_it >= threshold)
		{
			hitThreshold++;
			if (hitThreshold <= 3)
			{
				system('cls');
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

	printf("\n\nTotal Time: %.2f ms\n\n", time);

	printf("\n\nBest Time: %.2f ms\n\n", *best_time);

	//Libertar memoria e fechar semaforo
	free(path);
	free(pids);
	sem_close(access_mem);

	return EXIT_SUCCESS;
}