#ifndef HEAD_GRAPH
#define HEAD_GRAPH

#include <stdbool.h>

#define NaN __builtin_nanf("")

// create an edge from src to dest and add it to the src node
#define graph_link(src, dest, weight) graph_addEdge(src, graph_createEdge(src,dest,weight))

// create a bidirectionnal link beetween a and b
#define graph_linkBoth(a, b, weight) (graph_link(a,b,weight), graph_link(b,a,weight))

typedef struct graph_node_t
{
    struct graph_edge_t** edges;
    bool visited;
    void* value;
    double distance;
    double heuristic;
    struct graph_node_t* parent;
} graph_node_t;

typedef struct graph_edge_t
{

    double weight;
    graph_node_t* src;
    graph_node_t* dest;

} graph_edge_t;

typedef struct {
    size_t src, dest;
    double weight;
} graph_link_t;


typedef void* (*graph_todo_fn)(void* value, void* args);

// return an heuristic value for a given value based on the goalInfo
typedef double (*graph_heuristic_fn)(void* value, void* goalInfo);

// return if the given value is the goal based on the goalInfo
typedef bool (*graph_isGoal_fn)(void* value, void* goalInfo);

// create an edge with the given src, dest and weight
// must be free with free()
graph_edge_t* graph_createEdge(graph_node_t* src, graph_node_t* dest, double weight);

// initialize the given node
void graph_initNode(graph_node_t* node, void* value);

// free a node and return its value
void* graph_freeNode(graph_node_t* node);

// add the given edge to the given node
void graph_addEdge(graph_node_t* node, graph_edge_t* edge);

// execute a DFS from the given node
// stop and return the first non NULL value returned by todo_fn
// if todo_fn don't return a non NULL value when all node are visited, return NULL
// args is passed as the second param of todo_fn
void* graph_DFS(graph_node_t* node, graph_todo_fn todo_fn, void* args);

// execute a BFS from the given node
// stop and return the first non NULL value returned by todo_fn
// if todo_fn don't return a non NULL value when all node are visited, return NULL
// args is passed as the second param of todo_fn
void* graph_BFS(graph_node_t* node, graph_todo_fn todo_fn, void* args);

// free all the nodes in the given graph
// free each value with the given free_fn
// (do nothing if free_fn is NULL)
void graph_freeGraph(size_t nvalues, graph_node_t* graph, void (*free_fn)(void*));

// output a malloced array of edges that makes the shortest path from the node to the goal
// isGoal_fn takes a node value and return true if its the goal based on the given goalInfo
// heuristic_fn return the heuristic value of the node with the given value based on the goalInfo
// make the function behave like A* algo
// if weighted is false, consider all edges weight to be 0, but still use the heuristic value
// if heuristic_fn is NULL, this is like dijkstra algo
// if heuristic_fn is NULL and all edge weight is 0 this is just a BFS of the shortest path
// return NULL if their is no path or the starting node is the goal 
// (if you know all edges weight are 0, give false to the weighted parameter for optimisation)
void* graph_findPath(graph_node_t* node, void* goalInfo, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, bool weighted, size_t* size);

// return an array of nvalues nodes where nodes[k] has the value values[k]
// all nodes are linked together has the adjency matrice say
// adjencyMat must be has follow:
// adjencyMat[a][b] is the weight of the edge from a to b
// if adjencyMat[a][b] is NaN their is no edge
// exemple: their is n values, consider 0 <= a,b < n 
// the distance from values[a] to values[b] is adjencyMat[a][b]
// and the distance from values[b] to values[a] is adjencyMat[b][a];
// return NULL is nvalues == 0
graph_node_t* graph_constructFromAdjencyMat(size_t nvalues, void* values, double (*adjencyMat)[nvalues]);


// return an array of nvalues nodes where nodes[k] has the value values[k]
// all nodes are linked together has the links array say
// return NULL is nvalues == 0
graph_node_t* graph_constructFromLinksArr(size_t nvalues, void* values, size_t nlinks, graph_link_t* links, bool weighted);


#endif