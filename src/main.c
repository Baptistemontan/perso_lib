#include "../headers/all.h"

#define SIZE 5
#define cdouble(a) misc_createdouble(a, misc_printErr)
#define noweight cdouble(0)

void* print_uint(void* num, void* args) {
    printf("%u\n",*(uint*)num);
    return NULL;
}


int main(int argc, char const *argv[])
{
    double* mat[SIZE][SIZE] = {
        {NULL, noweight, NULL, NULL, NULL},
        {noweight, NULL, noweight, NULL, NULL},
        {noweight, NULL, NULL, NULL, noweight},
        {noweight, NULL, NULL, NULL, noweight},
        {NULL, NULL, noweight, noweight, NULL}
    };

    double*** mat2 = malloc(sizeof(double**) * SIZE);
    for(uint i = 0; i < SIZE; i++) {
        mat2[i] = mat[i];
    }

    void* values[SIZE];
    for(uint i = 0; i < SIZE; i++) {
        values[i] = misc_createuint(i + 1, misc_printErr);
    }

    graph_node** graph = graph_constructAdjency(SIZE, values, mat2);
    graph_node* node = graph[2];
    graph_DFS(node, print_uint, NULL);
    printf("\n");
    graph_BFS(node, print_uint, NULL);



    graph_freeGraph(node, free);
    free(graph);
    for(uint i = 0; i < SIZE; i++) {
        for(uint j = 0; j < SIZE; j++) {
            free(mat[i][j]);
        }
    }
    free(mat2);
    return 0;
}