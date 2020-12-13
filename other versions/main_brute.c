/***********************************************************
 * 
 * Este ficheiro pode não estar a funcionar como
 * deve ser
 * 
 **********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#define MILHAO 1000000L;

int size;

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

	time_t seconds;
	srand(time(NULL));

	if (argc != 3)
	{
		return (EXIT_FAILURE);
	}

	char pathFile[] = "tests/"; //[OBRIGATORIO] - Nome do Ficheiro
	strcat(pathFile, argv[1]);
	int max_time = atoi(argv[2]); //[OBRIGATORIO] - Tempo maximo de execusão

	struct timespec begin;
	clock_gettime(CLOCK_REALTIME, &begin);

	int firstRow = 1;
	int matrix[5][5];

	FILE *file;
	char string[1000];

	file = fopen(pathFile, "r");
	if (file == NULL)
	{
		printf("Could not open file %s", pathFile);
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
			printf("%s", string);
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

	int path[size];
	for (int i = 0; i < size; i++)
	{
		path[i] = i + 1;
	}
	int bestPath[size];
	for (int i = 0; i < size; i++)
	{
		bestPath[i] = i + 1;
	}

	shuffle(path, size);
	int bestDist = 9999999;
	long it = 0;
	long bestIt = 0;
	double bestTime;

	while (time(&seconds) - begin.tv_sec < max_time)
	{
		swap(size, path);
		int dist = distance(size, path, matrix);
		it = it + 1;
		if (dist < bestDist)
		{
			struct timespec timer;
			clock_gettime(CLOCK_REALTIME, &timer);
			bestDist = dist;
			bestIt = it;
			bestTime = (timer.tv_sec - begin.tv_sec) +
					   (double)(timer.tv_nsec - begin.tv_nsec) / (double)MILHAO;
			for (int z = 0; z < size; z++)
			{
				bestPath[z] = path[z];
			}
		}
	}
	printf("\nDistance: %d\n\n", bestDist);
	printf("Best Path -");
	for (int i = 0; i < size; i++)
	{
		printf(" %d ", bestPath[i]);
	}
	printf("\n\nIterations: %ld", it);

	printf("\n\nBest Iteration: %ld", bestIt);

	printf("\n\nBest Time: %.2f ms\n\n", bestTime);
	return 0;
}