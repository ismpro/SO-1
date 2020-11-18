#include <stdio.h>

int distance(int size, int path[size], int matrix[size][size])
{
    int dist = 0;

    for (int i = 0; i < size - 1; i++)
    {

        int curr = path[i] - 1;
        int next = path[i + 1];
        dist += matrix[curr][next];
    }

    int last = path[size - 1] - 1;
    int first = path[0] - 1;
    dist += matrix[last][first];

    return dist;
}

int main()
{

    int size = 5;
    int matrix[5][5] = {
        {0, 23, 10, 4, 1},
        {23, 0, 9, 5, 4},
        {10, 9, 0, 8, 2},
        {4, 5, 8, 0, 11},
        {1, 4, 2, 11, 0},
    };

    int path[] = {5, 4, 2, 1, 3};

    int dist = distance(size, )
}