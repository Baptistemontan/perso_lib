#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../headers/graph.h"
#include "../headers/dynarr.h"

#define UNVISIT(visited) for(size_t i = dynarr_getSize(visited); i > 0; i--){graph_private_reset(visited[i - 1]);}; dynarr_free(visited)

graph_edge_t* graph_createEdge(graph_node_t* src, graph_node_t* dest, double weight) {
    graph_edge_t* edge = malloc(sizeof(graph_edge_t));
    if(edge == NULL) return NULL;
    edge->src = src;
    edge->weight = weight;
    edge->dest = dest;
    return edge;
}

graph_node_t* graph_createNode(void* value) {
    graph_node_t* node = malloc(sizeof(graph_node_t));
    node->edges = DYNARR_INIT(graph_edge_t*);
    node->value = value;
    node->visited = false;
    node->distance = 0;
    node->pathEdge = NULL;
    node->heuristic = 0;
    return node;
}

void* graph_freeNode(graph_node_t* node) {
    if(node == NULL) return NULL;
    void* tmp = node->value;
    for(size_t i = dynarr_getSize(node->edges); i > 0; i--) {
        free(node->edges[i - 1]);
    }
    dynarr_free(node->edges);
    free(node);
    return tmp;
}

static void graph_private_reset(graph_node_t* node) {
    node->pathEdge = NULL;
    node->distance = 0;
    node->visited = false;
    node->pathEdge = NULL;
}

// static void graph_private_freeGraph(graph_node_t* node, graph_node_t*** queue) {
//     if(node == NULL || node->visited) return;
//     node->visited = true;
//     dynarr_pushBack(queue, &node);
//     for(size_t i = dynarr_getSize(node->edges); i > 0; i--) {
//         graph_private_freeGraph(node->edges[i - 1]->dest, queue);
//     }
// }

// void graph_freeGraph(graph_node_t* node, void (*free_fn)(void*)) {
//     graph_node_t** queue = DYNARR_INIT(graph_node_t*);
//     graph_private_freeGraph(node, &queue);
//     for(size_t i = dynarr_getSize(queue); i > 0; i--) {
//         if(free_fn != NULL) free_fn(queue[i - 1]->value);
//         graph_freeNode(queue[i - 1]);
//     }
//     dynarr_free(queue);
// }

static void graph_private_dummyFree(void* ptr) {
    return;
}

void graph_freeGraph(size_t nvalues, graph_node_t** graph, void (*free_fn)(void*)){
    if(free_fn == NULL) free_fn = graph_private_dummyFree;
    for(size_t i = 0; i < nvalues; i++) {
        free_fn(graph_freeNode(graph[i]));
    }
}

void graph_addEdge(graph_node_t* node, graph_edge_t* edge) {
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
    for(size_t i = dynarr_getSize(node->edges); i > 0; i--) {
        test = graph_private_DFS(node->edges[i - 1]->dest, visited, todo_fn, args);
        if(test != NULL) return test;
    }
    return todo_fn(node->value, args);
}

void* graph_DFS(graph_node_t* node, graph_todo_fn todo_fn, void* args) {
    if(node == NULL) return NULL;
    graph_node_t** visited = DYNARR_INIT(graph_node_t*);
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
    while(dynarr_getSize(*queue)) {
        currentNode = *(graph_node_t**)dynarr_popFront(queue);
        test = todo_fn(currentNode->value, args);
        if(test != NULL) return test;
        for(size_t i = dynarr_getSize(currentNode->edges); i > 0; i--) {
            graph_private_BFS_addUnvisited(currentNode->edges[i - 1]->dest,queue,visited);
        }
    }
    return NULL;
}

void* graph_BFS(graph_node_t* node, graph_todo_fn todo_fn, void* args) {
    if(node == NULL) return NULL;
    graph_node_t** queue = DYNARR_INIT(graph_node_t*);
    graph_node_t** visited = DYNARR_INIT(graph_node_t*);
    graph_private_BFS_addUnvisited(node, &queue, &visited);
    void* tmp = graph_private_BFS(&queue, todo_fn, &visited, args);
    UNVISIT(visited);
    dynarr_free(queue);
    return tmp;
}


static int graph_private_sortDistWeighted(const void* a, const void* b) {
    const graph_node_t* ia = a;
    const graph_node_t* ib = b;
    const double da = ia->distance + ia->heuristic;
    const double db = ib->distance + ib->heuristic;
    return (da > db) - (da < db);
}

static int graph_private_sortDistUnweighted(const void* a, const void* b) {
    const graph_node_t* ia = a;
    const graph_node_t* ib = b;
    return (ia->heuristic > ib->heuristic) - (ia->heuristic < ib->heuristic);
}

static void* graph_private_findPath_visitNeighbours(graph_edge_t* edge, graph_heuristic_fn heuristic_fn, graph_node_t*** visited, graph_node_t*** queue, void* goalInfo) {
    graph_node_t* parent = edge->src;
    if(parent->distance + edge->weight < edge->dest->distance) {
        edge->dest->pathEdge = edge;
        edge->dest->distance = parent->distance + edge->weight;
    }
    if(edge->dest->visited == false){
        dynarr_pushBack(queue, &edge->dest);
        graph_private_addVisited(visited, edge->dest);
        if(heuristic_fn != NULL) {
            edge->dest->heuristic = heuristic_fn(edge->dest->value, goalInfo);
        }
    } 
    return NULL;
}

static graph_edge_t** graph_private_findPath(graph_node_t*** queue, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, graph_node_t*** visited, void* goalInfo, bool weighted) {
    graph_node_t* currentNode = NULL;
    while(dynarr_getSize(*queue)) {
        currentNode = dynarr_popFront(queue);
        if(isGoal_fn(currentNode->value, goalInfo)) {
            graph_edge_t** path = DYNARR_INIT(graph_edge_t*);
            while(currentNode != NULL) {
                dynarr_pushFront(&path, &currentNode->pathEdge);
                currentNode = currentNode->pathEdge->src;
            }
            return path;
        }
        for(size_t i = dynarr_getSize(currentNode->edges); i > 0; i--) {
            graph_private_findPath_visitNeighbours(currentNode->edges[i - 1], heuristic_fn, visited, queue, goalInfo);
        }
        if(weighted) {
            dynarr_qsort(queue, graph_private_sortDistWeighted);
        } else if(heuristic_fn != NULL) {
            dynarr_qsort(queue, graph_private_sortDistUnweighted);
        }
    }
    return NULL;
}

graph_edge_t** graph_findPath(graph_node_t* node, void* goalInfo, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, bool weighted) {
    if(node == NULL) return NULL;
    graph_node_t** queue = DYNARR_INIT(graph_node_t*);
    graph_node_t** visited = DYNARR_INIT(graph_node_t*);
    node->visited = true;
    dynarr_pushBack(&visited, &node);
    dynarr_pushBack(&queue, &node);
    graph_edge_t** tmp = graph_private_findPath(&queue, isGoal_fn, heuristic_fn, &visited, goalInfo, weighted);
    UNVISIT(visited);
    dynarr_free(queue);
    graph_edge_t** tmp2 = malloc(sizeof(graph_edge_t*) * dynarr_getSize(tmp));
    memcpy(tmp2,tmp, sizeof(graph_edge_t*) * dynarr_getSize(tmp));
    dynarr_free(tmp);
    return tmp2;
}

graph_node_t** graph_constructFromAdjencyMat(size_t nvalues, void** values, double (*adjencyMat)[nvalues]) {
    if(nvalues == 0) return NULL;
    graph_node_t** nodes = malloc(sizeof(graph_node_t*) * nvalues);
    for(size_t i = 0; i < nvalues; i++) {
        nodes[i] = graph_createNode(values[i]);
    }
    for(size_t i = 0; i < nvalues; i++) {
        for(size_t j = 0; j < nvalues; j++) {
            if(isnan(adjencyMat[i][j])) continue;
            graph_link(nodes[i], nodes[j], adjencyMat[i][j]);
        }
    }
    return nodes;
}

graph_node_t** graph_constructFromLinksArr(size_t nvalues, void** values, size_t nlinks, graph_link_t* links, bool weighted) {
    if(nvalues == 0) return NULL;
    graph_node_t** nodes = malloc(sizeof(graph_node_t*) * nvalues);
    for(size_t i = 0; i < nvalues; i++) {
        nodes[i] = graph_createNode(values[i]);
    }
    size_t dest, src;
    double weight = 0;
    for(size_t i = 0; i < nlinks; i++) {
        if(weighted) weight = links[i].weight;
        src = links[i].src;
        dest = links[i].dest;
        if(src >= 0 && src < nvalues && dest >= 0 && dest < nvalues) {
            graph_link(nodes[src], nodes[dest], weight);
        }
    }
    return nodes;
}