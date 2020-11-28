#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>

#define MILHAO 1000000L;

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

int main(int argc, char *argv[])
{

	if (argc != 3)
	{
		return (EXIT_FAILURE);
	}

	int num_proc = atoi(argv[1]);
	int max_time = atoi(argv[2]);
	struct timespec begin;

	clock_gettime(CLOCK_REALTIME, &begin);

	srand(time(NULL));

	int size = 5;
	int matrix[5][5] = {
		{0, 23, 10, 4, 1},
		{23, 0, 9, 5, 4},
		{10, 9, 0, 8, 2},
		{4, 5, 8, 0, 11},
		{1, 4, 2, 11, 0},
	};

	int path[size];
	for (int i = 0; i < size; i++)
	{
		path[i] = i + 1;
	}

	shuffle(path, size);

	sem_unlink("access_mem");
	sem_t *access_mem = sem_open("access_mem", O_CREAT, 0644, 1);

	int protection = PROT_READ | PROT_WRITE;
	int visibility = MAP_ANONYMOUS | MAP_SHARED;

	int *best_dist = mmap(NULL, sizeof(int), protection, visibility, 0, 0);
	int *best_path = mmap(NULL, sizeof(int) * 5, protection, visibility, 0, 0);
	long *it = mmap(NULL, sizeof(long), protection, visibility, 0, 0);
	double *best_time = mmap(NULL, sizeof(double) * 5, protection, visibility, 0, 0);
	long *best_it = mmap(NULL, sizeof(long) * 5, protection, visibility, 0, 0);

	*best_dist = 99999;

	int pids[num_proc];

	for (int i = 0; i < num_proc; i++)
	{
		pids[i] = fork();
		if (pids[i] == 0)
		{
			printf("Worker process #%d!\n", i + 1);
			while (1)
			{
				swap(size, path);
				int dist = distance(size, path, matrix);
				sem_wait(access_mem);
				*it = *it + 1;
				if (dist < *best_dist)
				{
					struct timespec timer;
					clock_gettime(CLOCK_REALTIME, &timer);
					*best_dist = dist;
					*best_it = *it;
					*best_time = (timer.tv_sec - begin.tv_sec) +
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

	/*while (begin + max_time*1000 < time(NULL))
	{

	}
	*/

	sleep(max_time);

	sem_close(access_mem);

	// Kill worker processes
	for (int i = 0; i < num_proc; i++)
	{
		printf("Killing Worker %d\n", i + 1);
		kill(pids[i], SIGKILL);
	}

	printf("\nDistance: %d\n\n", *best_dist);
	printf("Best Path -");
	for (int i = 0; i < size; i++)
	{
		printf(" %d ", best_path[i]);
	}
	printf("\n\nIterations: %ld", *it);

	printf("\n\nBest Iteration: %ld", *best_it);

	printf("\n\nBest Time: %.1f Milissegundos\n\n", *best_time);

	sem_close(access_mem);

	return 0;
}