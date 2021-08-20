#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/graph.h"
#include "../headers/dynarr.h"


#define UNVISIT(visited) for(size_t i = dynarr_size(visited); i > 0; i--){graph_private_reset(visited[i - 1]);}; dynarr_free(visited)
#define MALLOCCHECK(val, callback) if(val == NULL) {fprintf(stderr,"Failed to allocate memory"); callback;}
#define MALLOC2(type, valname, size, errorcallback) type valname = malloc(size); MALLOCCHECK(valname, errorcallback)
#define STARTNODE ((void*)0xffffffffffffffff)

graph_edge_t* graph_createEdge(graph_node_t* src, graph_node_t* dest, double weight) {
    MALLOC2(graph_edge_t*, edge, sizeof(graph_edge_t), return NULL);
    edge->src = src;
    edge->weight = weight;
    edge->dest = dest;
    return edge;
}

void graph_initNode(graph_node_t* node, void* value) {
    node->edges = dynarr_create(sizeof(graph_edge_t*));
    node->value = value;
    node->visited = false;
    node->distance = 0;
    node->parent = NULL;
    node->heuristic = 0;
}

void* graph_freeNode(graph_node_t* node) {
    if(node == NULL) return NULL;
    for(size_t i = dynarr_size(node->edges); i > 0; i--) {
        free(node->edges[i - 1]);
    }
    dynarr_free(node->edges);
    return node->value;
}

static void graph_private_reset(graph_node_t* node) {
    node->parent = NULL;
    node->distance = 0;
    node->visited = false;
    node->heuristic = 0;
}

static void graph_private_dummyFree(void* ptr) {
    return;
}

void graph_freeGraph(size_t nvalues, graph_node_t* graph, void (*free_fn)(void*)){
    if(free_fn == NULL) free_fn = graph_private_dummyFree;
    for(size_t i = 0; i < nvalues; i++) {
        free_fn(graph_freeNode(graph + i));
    }
}

void graph_addEdge(graph_node_t* node, graph_edge_t* edge) {
    if(node == NULL || edge == NULL) return;
    dynarr_pushBack(&node->edges, &edge);
}

static void graph_private_addVisited(graph_node_t*** visited, graph_node_t* node) {
    if(node == NULL || node->visited) return;
    node->visited = true;
    dynarr_pushBack(visited, &node);
}

static void* graph_private_DFS(graph_node_t* node, graph_node_t*** visited, graph_todo_fn todo_fn, void* args) {
    if(node == NULL || node->visited) return NULL;
    graph_private_addVisited(visited, node);
    void* test = NULL;
    for(size_t i = dynarr_size(node->edges); i > 0; i--) {
        test = graph_private_DFS(node->edges[i - 1]->dest, visited, todo_fn, args);
        if(test != NULL) return test;
    }
    return todo_fn(node->value, args);
}

void* graph_DFS(graph_node_t* node, graph_todo_fn todo_fn, void* args) {
    if(node == NULL) return NULL;
    graph_node_t** visited = dynarr_create(sizeof(graph_node_t*));
    void* tmp = graph_private_DFS(node, &visited, todo_fn, args);
    UNVISIT(visited);
    return tmp;
}

static void graph_private_BFS_addUnvisited(graph_node_t* node, graph_node_t*** queue, graph_node_t*** visited) {
    if(node->visited == true) return;
    dynarr_pushBack(queue, &node);
    graph_private_addVisited(visited, node);
}

static void* graph_private_BFS(graph_node_t*** queue, graph_todo_fn todo_fn, graph_node_t*** visited, void* args) {
    graph_node_t* currentNode = NULL;
    void* test = NULL;
    while(dynarr_size(*queue)) {
        currentNode = *(graph_node_t**)dynarr_popFront(queue);
        test = todo_fn(currentNode->value, args);
        if(test != NULL) return test;
        for(size_t i = dynarr_size(currentNode->edges); i > 0; i--) {
            graph_private_BFS_addUnvisited(currentNode->edges[i - 1]->dest,queue,visited);
        }
    }
    return NULL;
}

void* graph_BFS(graph_node_t* node, graph_todo_fn todo_fn, void* args) {
    if(node == NULL) return NULL;
    graph_node_t** queue = dynarr_create(sizeof(graph_node_t*));
    graph_node_t** visited = dynarr_create(sizeof(graph_node_t*));
    graph_private_BFS_addUnvisited(node, &queue, &visited);
    void* tmp = graph_private_BFS(&queue, todo_fn, &visited, args);
    UNVISIT(visited);
    dynarr_free(queue);
    return tmp;
}


static int graph_private_sortDistWeighted(const void* a, const void* b) {
    const graph_node_t* ia = *(graph_node_t**)a;
    const graph_node_t* ib = *(graph_node_t**)b;
    const double da = ia->distance + ia->heuristic;
    const double db = ib->distance + ib->heuristic;
    return (da > db) - (da < db);
}

static int graph_private_sortDistUnweighted(const void* a, const void* b) {
    const graph_node_t* ia = *(graph_node_t**)a;
    const graph_node_t* ib = *(graph_node_t**)b;
    return (ia->heuristic > ib->heuristic) - (ia->heuristic < ib->heuristic);
}

static void graph_private_findPath_visitNeighbours(graph_edge_t* edge, graph_heuristic_fn heuristic_fn, graph_node_t*** visited, graph_node_t*** queue, void* goalInfo) {
    if(edge->src->distance + edge->weight < edge->dest->distance || edge->dest->parent == NULL) {
        edge->dest->parent = edge->src;
        edge->dest->distance = edge->src->distance + edge->weight;
    }
    if(edge->dest->visited == false){
        dynarr_pushBack(queue, &edge->dest);
        graph_private_addVisited(visited, edge->dest);
        if(heuristic_fn != NULL) {
            edge->dest->heuristic = heuristic_fn(edge->dest->value, goalInfo);
        }
    }
}

static void* graph_private_findPath(graph_node_t*** queue, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, graph_node_t*** visited, void* goalInfo, bool weighted) {
    graph_node_t* currentNode = NULL;
    while(dynarr_size(*queue)) {
        currentNode = *(graph_node_t**)dynarr_popFront(queue);
        if(isGoal_fn(currentNode->value, goalInfo)) {
            void** path = dynarr_create(sizeof(void*));
            while(currentNode != STARTNODE) {
                dynarr_pushFront(&path, &currentNode->value);
                currentNode = currentNode->parent;
            }
            return path;
        }
        for(size_t i = dynarr_size(currentNode->edges); i > 0; i--) {
            graph_private_findPath_visitNeighbours(currentNode->edges[i - 1], heuristic_fn, visited, queue, goalInfo);
        }
        if(weighted) {
            dynarr_qsort(*queue, graph_private_sortDistWeighted);
        } else if(heuristic_fn != NULL) {
            dynarr_qsort(*queue, graph_private_sortDistUnweighted);
        }
    }
    return NULL;
}

void* graph_findPath(graph_node_t* node, void* goalInfo, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, bool weighted, size_t* size) {
    if(node == NULL || isGoal_fn == NULL) return NULL;
    node->parent = STARTNODE;
    graph_node_t** queue = dynarr_create(sizeof(graph_node_t*));
    graph_node_t** visited = dynarr_create(sizeof(graph_node_t*));
    node->visited = true;
    dynarr_pushBack(&visited, &node);
    dynarr_pushBack(&queue, &node);
    void** tmp = graph_private_findPath(&queue, isGoal_fn, heuristic_fn, &visited, goalInfo, weighted);


    size_t vsize = dynarr_size(visited);
    printf("visited : %lu\n", vsize);



    UNVISIT(visited);
    dynarr_free(queue);
    size_t size_ = dynarr_size(tmp);
    MALLOC2(void**, tmp2, size_ * sizeof(void*), return NULL);
    memcpy(tmp2, tmp, size_ * sizeof(void*));
    if(size == NULL) *size = size_;
    dynarr_free(tmp);
    return tmp2;
}

graph_node_t* graph_constructFromAdjencyMat(size_t nvalues, void* values_, double (*adjencyMat)[nvalues]) {
    void** values = values_;
    if(nvalues == 0) return NULL;
    MALLOC2(graph_node_t*, nodes, sizeof(graph_node_t) * nvalues, return NULL);
    for(size_t i = 0; i < nvalues; i++) {
        graph_initNode(nodes + i, values[i]);
    }
    for(size_t i = 0; i < nvalues; i++) {
        for(size_t j = 0; j < nvalues; j++) {
            if(isnan(adjencyMat[i][j])) continue;
            graph_link(nodes + i, nodes + j, adjencyMat[i][j]);
        }
    }
    return nodes;
}

graph_node_t* graph_constructFromLinksArr(size_t nvalues, void* values_, size_t nlinks, graph_link_t* links, bool weighted) {
    void** values = values_;
    if(nvalues == 0) return NULL;
    MALLOC2(graph_node_t*, nodes, sizeof(graph_node_t) * nvalues, return NULL);
    for(size_t i = 0; i < nvalues; i++) {
        graph_initNode(nodes + i, values[i]);
    }
    size_t dest, src;
    double weight = 0;
    for(size_t i = 0; i < nlinks; i++) {
        if(weighted) weight = links[i].weight;
        src = links[i].src;
        dest = links[i].dest;
        if(src >= 0 && src < nvalues && dest >= 0 && dest < nvalues) {
            graph_link(nodes + src, nodes + dest, weight);
        }
    }
    return nodes;
}