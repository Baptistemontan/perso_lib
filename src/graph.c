#include "../headers/graph.h"

static void* graph_private_DFS(graph_node* node, void** args);


#define UNVISIT(visited) while(dynarr_size(visited)){graph_private_reset(*(graph_node**)dynarr_popBack(&visited));}; dynarr_free(visited)

enum {
    ARGS_FN = 0,
    ARGS_ARGS = 1,
    ARGS_VISITED = 2,
    ARGS_QUEUE = 3,
    ARGS_SIZE = 4
};

graph_edge* graph_createEdge(graph_node* src, graph_node* dest, double weight) {
    graph_edge* edge = malloc(sizeof(graph_edge));
    if(edge == NULL) return NULL;
    edge->src = src;
    edge->weight = weight;
    edge->dest = dest;
    return edge;
}

graph_node* graph_createNode(void* value, size_t nb_edges, graph_edge** edges) {
    graph_node* node = malloc(sizeof(graph_node));
    node->edges = DYNARR_INIT(sizeof(graph_edge*));
    for(size_t i = 0; i < nb_edges; i++) {
        dynarr_pushBack(&node->edges, edges + i);
    }
    node->value = value;
    node->visited = false;
    node->distance = 0;
    node->pathEdge = NULL;
    node->heuristic = 0;
    return node;
}

void* graph_freeNode(graph_node* node) {
    void* tmp = node->value;
    for(uint i = dynarr_size(node->edges) - 1; i >= 0; i--) {
        free(node->edges[i]);
    }
    dynarr_free(node->edges);
    free(node);
    return tmp;
}

static void graph_private_reset(graph_node* node) {
    node->pathEdge = NULL;
    node->distance = 0;
    node->visited = false;
    node->pathEdge = NULL;
}

static void graph_private_freeGraph(graph_node* node, graph_node*** queue) {
    if(node == NULL || node->visited) return;
    node->visited = true;
    dynarr_pushBack(queue, &node);
    graph_edge* edge = NULL;
    while(dynarr_size(node->edges)) {
        edge = *(graph_edge**)dynarr_popBack(node->edges);
        graph_private_freeGraph(edge->dest, queue);
        free(edge);
    }
}

void graph_freeGraph(graph_node* node, void (*free_fn)(void*)) {
    graph_node** queue = DYNARR_INIT(sizeof(graph_node*));
    graph_private_freeGraph(node, &queue);
    graph_node* currentNode = NULL;
    while(dynarr_size(queue) > 0) {
        currentNode = *(graph_node**)dynarr_popBack(queue);
        if(free_fn != NULL) free_fn(currentNode->value);
        graph_freeNode(currentNode);
    }
}

void graph_addNEdges(graph_node* node, size_t nb_edges, graph_edge** edges) {
    for(size_t i = 0; i < nb_edges; i++) {
        dynarr_pushBack(&node->edges, edges + i);
    }
}

inline void graph_addEdge(graph_node* node, graph_edge* edge) {
    graph_addNEdges(node, 1, &edge);
}

static void graph_private_addVisited(graph_node*** visited, graph_node* node) {
    if(node == NULL || node->visited) return;
    node->visited = true;
    dynarr_pushBack(visited, &node);
}

// static void* graph_private_DFS_some(void* edge, size_t index, size_t offset, void* args) {
//     return graph_private_DFS(((graph_edge*)edge)->dest, args);
// }

// static void* graph_private_DFS(graph_node* node, void** args) {
//     if(node == NULL || node->visited) return NULL;
//     graph_private_addVisited(args[ARGS_VISITED], node);
//     void* test = dynarr_some(node->edges, graph_private_DFS_some, args);
//     if(test != NULL) return test;
//     void* (*todo_fn)(void*, void*) = args[ARGS_FN];
//     return todo_fn(node->value, args[ARGS_ARGS]);
// }

// void* graph_DFS(graph_node* node, graph_todo_fn todo_fn, void* args) {
//     if(node == NULL) return NULL;
//     graph_node** visited = DYNARR_INIT(sizeof(graph_node*));
//     void* _args[ARGS_SIZE];
//     _args[ARGS_FN] = todo_fn;
//     _args[ARGS_ARGS] = args;
//     _args[ARGS_VISITED] = &visited;
//     void* tmp = graph_private_DFS(node, _args);
//     UNVISIT(visited);
//     return tmp;
// }

// static void* graph_private_BFS_addUnvisited(void* _edge, size_t index, size_t offset, void* args) {
//     graph_edge* edge = _edge;
//     if(edge->dest->visited == false) {
//         dynarr_pushBack(((void**)args)[ARGS_QUEUE], &edge->dest);
//         graph_private_addVisited(((void**)args)[ARGS_VISITED], edge->dest);
//     }
//     return NULL;
// }

// static void* graph_private_BFS(graph_node*** queue, void** args) {
//     graph_node* currentNode = NULL;
//     void* test = NULL;
//     graph_todo_fn todo_fn = args[ARGS_FN];
//     while(dynarr_size(*queue)) {
//         currentNode = *(graph_node**)dynarr_popFront(queue);
//         test = todo_fn(currentNode->value, args[ARGS_ARGS]);
//         if(test != NULL) return test;
//         dynarr_forEach(currentNode->edges, graph_private_BFS_addUnvisited, args);
//     }
//     return NULL;
// }

// void* graph_BFS(graph_node* node, graph_todo_fn todo_fn, void* args) {
//     if(node == NULL) return NULL;
//     graph_node** queue = DYNARR_INIT(sizeof(graph_node*));
//     graph_node** visited = DYNARR_INIT(sizeof(graph_node*));
//     node->visited = true;
//     dynarr_pushBack(&visited, &node);
//     dynarr_pushBack(&queue, &node);
//     void* _args[ARGS_SIZE];
//     _args[ARGS_FN] = todo_fn;
//     _args[ARGS_ARGS] = args;
//     _args[ARGS_VISITED] = &visited;
//     _args[ARGS_QUEUE] = &queue;
//     void* tmp = graph_private_BFS(&queue, _args);
//     UNVISIT(visited);
//     return tmp;
// }


static int graph_private_sortDistWeighted(const void* a, const void* b) {
    const graph_node* ia = a;
    const graph_node* ib = b;
    const double da = ia->distance + ia->heuristic;
    const double db = ib->distance + ib->heuristic;
    return (da > db) - (da < db);
}

static int graph_private_sortDistUnweighted(const void* a, const void* b) {
    const graph_node* ia = a;
    const graph_node* ib = b;
    return (ia->heuristic > ib->heuristic) - (ia->heuristic < ib->heuristic);
}

static void* graph_private_Astar_visitNeighbours(graph_edge* edge, graph_heuristic_fn heuristic_fn, graph_node*** visited, graph_node*** queue, void* goalInfo) {
    graph_node* parent = edge->src;
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

static graph_edge** graph_private_Astar(graph_node*** queue, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, graph_node*** visited, void* goalInfo, bool weighted) {
    graph_node* currentNode = NULL;
    while(dynarr_size(*queue)) {
        currentNode = dynarr_popFront(queue);
        if(isGoal_fn(currentNode->value, goalInfo)) {
            graph_edge** path = DYNARR_INIT(sizeof(graph_edge*));
            while(currentNode != NULL) {
                dynarr_pushFront(&path, &currentNode->pathEdge);
                currentNode = currentNode->pathEdge->src;
            }
            return path;
        }
        size_t size = dynarr_size(currentNode->edges);
        for(uint i = 0; i < size; i++) {
            graph_private_Astar_visitNeighbours(currentNode->edges[i],heuristic_fn,visited,queue,goalInfo);
        }
        if(weighted) {
            dynarr_qsort(queue, graph_private_sortDistWeighted);
        } else if(heuristic_fn != NULL) {
            dynarr_qsort(queue, graph_private_sortDistUnweighted);
        }
    }
    return NULL;
}

graph_edge** graph_Astar(graph_node* node, void* goalInfo, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn, bool weighted) {
    if(node == NULL) return NULL;
    graph_node** queue = DYNARR_INIT(sizeof(graph_node*));
    graph_node** visited = DYNARR_INIT(sizeof(graph_node*));
    node->visited = true;
    dynarr_pushBack(&visited, &node);
    dynarr_pushBack(&queue, &node);
    graph_edge** tmp = graph_private_Astar(&queue, isGoal_fn, heuristic_fn, &visited, goalInfo, weighted);
    UNVISIT(visited);
    dynarr_free(queue);
    return tmp;
}

graph_node** graph_constructAdjency(size_t nvalues, void** values, double*** adjencyMat) {
    if(nvalues == 0) return NULL;
    graph_node** nodes = malloc(sizeof(graph_node*) * nvalues);
    for(size_t i = 0; i < nvalues; i++) {
        nodes[i] = graph_createEmptyNode(values[i]);
    }
    for(size_t i = 0; i < nvalues; i++) {
        for(size_t j = 0; j < nvalues; j++) {
            if(adjencyMat[i][j] != NULL) {
                graph_link(nodes[i], nodes[j], *(adjencyMat[i][j]));
            }
        }
    }
    return nodes;
}