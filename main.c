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
#include <math.h>
#include <inttypes.h>

#define MILHAO 1000000L;

int *path;
int *pids;
int *best_path;
int size;
int num_proc;
int hitThreshold;

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

void shuffle(int *array, int n)
{
	if (n > 1)
	{
		int i;
		for (i = 0; i < n - 1; i++)
		{
			int j = i + rand() / (RAND_MAX / (n - i) + 1);
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
	shuffle(path, size);
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
	char pathFile[] = "tests/"; //[OBRIGATORIO] - Nome do Ficheiro
	strcat(pathFile, argv[1]);
	strcat(pathFile, ".txt");
	num_proc = atoi(argv[2]);	  //[OBRIGATORIO] - Numero de processos filhos
	int max_time = atoi(argv[3]); //[OBRIGATORIO] - Tempo maximo de execusão

	//Começo do tempo
	struct timespec begin, timer, randNum, final;
	clock_gettime(CLOCK_REALTIME, &begin);
	signal(SIGUSR1, parent_callback);

	//Declaração de algumas variaveis
	hitThreshold = 0;

	FILE *file;
	char string[1000];

	file = fopen(pathFile, "r");
	if (file == NULL)
	{
		printf("Could not open file %s", pathFile);
		return 1;
	}

	size = atoi(fgets(string, 1000, file));
	int matrix[size][size];

	int line = 0;
	while (fgets(string, 1000, file) != NULL)
	{

		trim(string);

		//https://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
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

	pids = (int *)malloc(sizeof(int) * num_proc);

	path = (int *)malloc(sizeof(int) * size);
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
	best_path = mmap(NULL, sizeof(int) * 5, protection, visibility, 0, 0);
	long *it = mmap(NULL, sizeof(long), protection, visibility, 0, 0);
	double *best_time = mmap(NULL, sizeof(double), protection, visibility, 0, 0);
	long *best_it = mmap(NULL, sizeof(long), protection, visibility, 0, 0);

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
					*best_time = ((timer.tv_sec - begin.tv_sec) * 1000) +
								 (double)(timer.tv_nsec - begin.tv_nsec) / (double)MILHAO;
					for (int z = 0; z < size; z++)
					{
						best_path[z] = path[z];
					}
					kill(getppid(), SIGUSR1);
				}
				sem_post(access_mem);
			}
			exit(0);
		}
	}

	double ttime;
	double lastTime = 0;
	long ms;  // Milliseconds
	time_t s; // Seconds

	//Program LOOP
	while (time(NULL) - begin.tv_sec < max_time)
	{

		clock_gettime(CLOCK_REALTIME, &final);
		ttime = ((final.tv_sec - begin.tv_sec) * 1000) +
				(double)(final.tv_nsec - begin.tv_nsec) / (double)MILHAO;

		system("clear");
		print_progress((final.tv_sec - begin.tv_sec), max_time);
		printf("\nTime: %.1f", ttime);
		fflush(stdout);

		if (threshold != 0 && *it - *best_it >= threshold * (hitThreshold + 1))
		{
			hitThreshold++;
			if (hitThreshold >= 3)
			{
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

	system("clear");

	if (hitThreshold >= 3)
	{
		printf("\n\nKilling process due hiting 3 times the threshold\n\n");
	}

	// Kill worker processes
	for (int i = 0; i < num_proc; i++)
	{
		kill(pids[i], SIGKILL);
	}

	clock_gettime(CLOCK_REALTIME, &final);
	ttime = ((final.tv_sec - begin.tv_sec) * 1000) +
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

	//Libertar memoria e fechar semaforo
	free(path);
	free(pids);
	sem_close(access_mem);

	return EXIT_SUCCESS;
}