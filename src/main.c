#include "../headers/all.h"

#define SIZE 30
#define DEFAULTDIST 1
#define START 1 * SIZE + 2
#define END 27 * SIZE + 29

typedef struct {
    uint x, y;
} node_t;

node_t (*createNodes(size_t size))[] {
    node_t (*nodes)[size] = malloc(sizeof(node_t) * size * size);
    for(uint i = 0; i < size;i++) {
        for(uint j = 0; j < size; j++) {
            nodes[i][j].x = i;
            nodes[i][j].y = j;
        }
    }
    return nodes;
}

double heuristic(void* value, void* goal_) {
    node_t *current = value, *goal = goal_;
    uint a = (goal->x > current->x ? goal->x - current->x : current->x - goal->x);
    uint b = (goal->y > current->y ? goal->y - current->y : current->y - goal->y);
    return (a + b) * 2;
}


bool checkGoal(void *value, void* args) {
    return value == args;
}


int main(int argc, char const *argv[])
{
    node_t* nodes = (node_t*)createNodes(SIZE);

    node_t* values[SIZE * SIZE];
    double adjencyMat[SIZE * SIZE][SIZE * SIZE];

    for(uint i = 0; i < SIZE * SIZE; i++) {
        values[i] = nodes + i;
        for(uint j = 0; j < SIZE * SIZE; j++) {
            adjencyMat[i][j] = NaN;
        }
    }

    for(int i = 0; i < SIZE; i++) {
        for(int j = 0; j < SIZE; j++) {
            if(i - 1 >= 0) adjencyMat[i * SIZE + j][(i - 1) * SIZE + j] = DEFAULTDIST;
            if(i + 1 < SIZE) adjencyMat[i * SIZE + j][(i + 1) * SIZE + j] = DEFAULTDIST;
            if(j - 1 >= 0) adjencyMat[i * SIZE + j][i * SIZE + j - 1] = DEFAULTDIST;
            if(j + 1 < SIZE) adjencyMat[i * SIZE + j][i * SIZE + j + 1] = DEFAULTDIST;
        }
    }

    graph_node_t* graph = graph_constructFromAdjencyMat(SIZE * SIZE, values, adjencyMat);
    size_t size;

    printf("path from %u %u to %u %u :\n", values[START]->x, values[START]->y, values[END]->x, values[END]->y);
    printf("\nDijkstra :\n");
    free(graph_findPath(graph + START, values[END], checkGoal, NULL, true, &size));
    printf("\nA* :\n");
    free(graph_findPath(graph + START, values[END], checkGoal, heuristic, true, &size));


    graph_freeGraph(SIZE * SIZE, graph, NULL);
    free(nodes);

    return EXIT_SUCCESS;
}
