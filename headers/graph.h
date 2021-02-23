#ifndef HEAD_GRAPH
#define HEAD_GRAPH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../headers/dynarr.h"

// create a node with no edge
// must be free with graph_freeNode()
#define graph_createEmptyNode(value) graph_createNode(value, 0, NULL)

// create an edge from src to dest and add it to the src node
#define graph_link(src, dest, weight) graph_addEdge(src, graph_createEdge(src,dest,weight))

// create a bidirectionnal link beetween a and b
#define graph_linkBi(a, weight, b) graph_link(a,b,weight), graph_link(b,a,weight)

typedef struct graph_node
{
    struct graph_edge** edges;
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


typedef void* (*graph_todo_fn)(void* value, void* args);

// return an heuristic value for a given value based on the goalInfo
typedef double (*graph_heuristic_fn)(void* value, void* goalInfo);

// return if the given value is the goal based on the goalInfo
typedef bool (*graph_isGoal_fn)(void* value, void* goalInfo);

// create an edge with the given src, dest and weight
// must be free with free()
graph_edge* graph_createEdge(graph_node* src, graph_node* dest, double weight);

// create a node with the given value and set of edges
// nb_edges can be set to 0 and edges to NULL
// must be free with graph_freeNode()
graph_node* graph_createNode(void* value, size_t nb_edges, graph_edge** edges);

// free a node and return its value
void* graph_freeNode(graph_node* node);

// add nb_edges of edges to the given node
void graph_addNEdges(graph_node* node, size_t nb_edges, graph_edge** edges);

// add the given edge to the given node
void graph_addEdge(graph_node* node, graph_edge* edge);

// execute a DFS from the given node
// stop and return the first non NULL value returned by todo_fn
// if todo_fn don't return a non NULL value when all node are visited, return NULL
// args is passed as the second param of todo_fn
void* graph_DFS(graph_node* node, graph_todo_fn todo_fn, void* args);

// execute a BFS from the given node
// stop and return the first non NULL value returned by todo_fn
// if todo_fn don't return a non NULL value when all node are visited, return NULL
// args is passed as the second param of todo_fn
void* graph_BFS(graph_node* node, graph_todo_fn todo_fn, void* args);

// do a DFS on the given node and free all the reachable nodes and their edges
// pass the value of all nodes to free_fn if its not NULL
void graph_freeGraph(graph_node* node, void (*free_fn)(void*));

// output a dynarr of edges that makes the shortest path from the node to the goal
// isGoal_fn takes a node value and return true if its the goal based on the given goalInfo
// heuristic_fn return the heuristic value of the node with the given value based on the goalInfo
// if weighted is false, consider all edges weight to be 0, but still use the heuristic value
// if heuristic_fn is NULL, this is just a dijkstra algorithm
// if heuristic_fn is NULL and all edge weight is 0 this is just a BFS of the shortest path
// return NULL if their is no path or the starting node is the goal 
// (if you know all edges weight are 0, give false to the weighted parameter for optimisation)
graph_edge** graph_Astar(graph_node* node, void* goalInfo, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, bool weighted);

// return an array of nvalues nodes where nodes[k] has the value values[k]
// all nodes are linked together has the adjency matrice say
// adjencyMat must be has follow:
// adjencyMat[a][b] is a double adress of the weight of the edge from a to b
// if adjencyMat[a][b] is NULL their is no edge
// exemple: their is n values, consider 0 <= a,b < n 
// the distance from values[a] to values[b] is adjencyMat[a][b]
// and the distance from values[b] to values[a] is adjencyMat[b][a];
// return NULL is nvalues == 0
graph_node** graph_constructAdjency(size_t nvalues, void** values, double*** adjencyMat);


#endif