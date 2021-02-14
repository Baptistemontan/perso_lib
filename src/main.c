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


double*** createAdjMat(dynarr_arr* links, size_t nnode) {
    double*** mat = malloc(sizeof(double**) * nnode);
    for(size_t i = 0; i < nnode; i++) {
        mat[i] = malloc(sizeof(double*) * nnode);
        for(size_t j = 0; j < nnode; j++) {
            mat[i][j] = NULL;
        }
    }
    Link* linkInfo = NULL;
    for(size_t i = 0; i < dynarr_getSize(links); i++) {
        linkInfo = dynarr_get(links, i);
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
    dynarr_arr* links = DYNARR_INIT;
    dynarr_push(links, createLink(0,1,0));
    dynarr_push(links, createLink(1,0,0));
    dynarr_push(links, createLink(1,2,0));
    dynarr_push(links, createLink(2,0,0));
    dynarr_push(links, createLink(2,4,0));
    dynarr_push(links, createLink(3,0,0));
    dynarr_push(links, createLink(3,4,0));
    dynarr_push(links, createLink(4,2,0));
    dynarr_push(links, createLink(4,3,0));
    double*** adjMat = createAdjMat(links, SIZE);

    // graph creation
    graph_node** graph = graph_constructAdjency(SIZE, values, adjMat);


    // graph traversal
    graph_node* node = graph[2];
    graph_DFS(node, print_uint, NULL);
    printf("\n");
    graph_BFS(node, print_uint, NULL);


    // free allocated mem
    dynarr_free(links, free);
    freeAdjMat(SIZE, adjMat);
    graph_freeGraph(node, free);
    
    return 0;
}