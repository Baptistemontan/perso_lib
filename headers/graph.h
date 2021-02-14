#ifndef HEAD_GRAPH
#define HEAD_GRAPH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../headers/dynarr.h"

#define graph_createEmptyNode(value) graph_createNode(value, 0, NULL);

typedef struct graph_node
{
    dynarr_arr* edges;
    bool visited;
    void* value;
    double distance;
    double heuristic;
    struct graph_edge* pathEdge;
} graph_node;

typedef struct graph_edge
{

    double weight;
    graph_node* src;
    graph_node* dest;

} graph_edge;


graph_edge* graph_createEdge(graph_node* src, double weight, graph_node* dest);

graph_node* graph_createNode(void* value, size_t nb_edges, graph_edge** edges);

void* graph_freeNode(graph_node* node);

void graph_addNEdges(graph_node* node, size_t nb_edges, graph_edge** edges);

void graph_addEdge(graph_node* node, graph_edge* edge);

void* graph_DFS(graph_node* node, void* (*todo_fn)(void* value, void* args), void* args);

void* graph_BFS(graph_node* node, void* (*todo_fn)(void* value, void* args), void* args);

void graph_freeGraph(graph_node* node, void (*free_fn)(void*));

dynarr_arr* graph_Astar(graph_node* node, void* goalInfo, bool (*isGoal_fn)(void* node, void* goalInfo), double (*heuristic_fn)(void* node, void* goalInfo));

#endif