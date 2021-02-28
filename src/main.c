#include "../headers/all.h"

#define SIZE 5

void* print_uint(void* num, void* args) {
    printf("%u\n",*(uint*)num);
    return NULL;
}

typedef struct {
    size_t src,dest;
    double weight;
} Link;


double*** createAdjMat(Link** links, size_t nnode) {
    double*** mat = malloc(sizeof(double**) * nnode);
    for(size_t i = 0; i < nnode; i++) {
        mat[i] = malloc(sizeof(double*) * nnode);
        for(size_t j = 0; j < nnode; j++) {
            mat[i][j] = NULL;
        }
    }
    Link* linkInfo = NULL;
    for(size_t i = dynarr_getSize(links); i > 0 ; i--) {
        linkInfo = links[i - 1];
        mat[linkInfo->src][linkInfo->dest] = misc_createdouble(linkInfo->weight, misc_printErr);
    }
    return mat;
}

Link* createLink(size_t src, size_t dest, double weight) {
    Link* linkInfo = malloc(sizeof(Link));
    linkInfo->src = src;
    linkInfo->dest = dest;
    linkInfo->weight = weight;
    return linkInfo;
}

void freeAdjMat(size_t nnode, double*** adjencyMat) {
    for(size_t i = 0; i < nnode; i++) {
        for(size_t j = 0; j < nnode; j++) {
            free(adjencyMat[i][j]);
        }
        free(adjencyMat[i]);
    }
    free(adjencyMat);
}


int main(int argc, char const *argv[])
{


    // values creation
    void* values[SIZE];
    for(uint i = 0; i < SIZE; i++) {
        values[i] = misc_createuint(i + 1, misc_printErr);
    }


    // adjency mat creation
    Link** links = DYNARR_INIT(Link*);
    Link* buff = NULL;
    buff = createLink(0,1,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(1,0,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(1,2,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(2,0,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(2,4,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(3,0,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(3,4,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(4,2,0);
    dynarr_pushBack(&links, &buff);
    buff = createLink(4,3,0);
    dynarr_pushBack(&links, &buff);
    double*** adjMat = createAdjMat(links, SIZE);
    // graph creation
    graph_node** graph = graph_constructAdjency(SIZE, values, adjMat);


    // // graph traversal
    graph_node* node = graph[2];
    printf("DFS:\n");
    graph_DFS(node, print_uint, NULL);
    printf("\n");
    printf("BFS:\n");
    graph_BFS(node, print_uint, NULL);
    printf("\n");
    // free allocated mem
    for(size_t i = dynarr_getSize(links); i > 0; i--) {
        free(links[i - 1]);
    }
    dynarr_free(links);
    freeAdjMat(SIZE, adjMat);
    graph_freeGraph(node, free);

    int* arr = DYNARR_INIT(int);
    int a;
    for(size_t i = 0; i < SIZE; i++) {
        a = i + 2;
        dynarr_pushBack(&arr, &a);
        a = SIZE + i;
        dynarr_pushFront(&arr, &a);
    }
    for(size_t i = 0; i < dynarr_getSize(arr); i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    dynarr_free(arr);

    return EXIT_SUCCESS;
}