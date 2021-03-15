#include "../headers/all.h"

#define NVALUES 5
#define NLINKS 5

void* printInt(void* a, void* args) {
    printf("%d\n", *(int*)a);
    return NULL;
}

int main(int argc, char const *argv[])
{
    int intVal[NVALUES] = {0, 1, 2, 3, 4};
    void* values[NVALUES];
    for (size_t i = 0; i < NVALUES; i++) {
        values[i] = intVal + i;
    }
    
    graph_link_t links[NLINKS] = {
        {.src = 1, .dest = 2},
        {.src = 2, .dest = 3},
        {.src = 3, .dest = 2},
        {.src = 1, .dest = 4},
        {.src = 4, .dest = 0}
    };
    graph_node_t** graph = graph_constructFromLinksArr(NVALUES, values, NLINKS, links, 0);

    graph_DFS(graph[1], printInt, NULL);

    graph_freeGraph(NVALUES, graph, NULL);

    printf("\n");

    double adjMat[NVALUES][NVALUES] = {
        {NaN, NaN, NaN, NaN, NaN}, // 0 -> .
        {NaN, NaN,   0, NaN,   0}, // 1 -> 2, 4
        {NaN, NaN, NaN,   0, NaN}, // 2 -> 3
        {NaN, NaN,   0, NaN, NaN}, // 3 -> 2
        {  0, NaN, NaN, NaN, NaN}  // 4 -> 0
    };

    graph = graph_constructFromAdjencyMat(NVALUES, values, adjMat);

    graph_DFS(graph[1], printInt, NULL);

    intVal[0] = 17;
    printf("\n");

    graph_BFS(graph[1], printInt, NULL);

    graph_freeGraph(NVALUES, graph, NULL);

    return EXIT_SUCCESS;
}
