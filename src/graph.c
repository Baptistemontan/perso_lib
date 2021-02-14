#include "../headers/graph.h"

static void* graph_private_DFS(graph_node* node, void** args);
static void* graph_private_reset(void* node, size_t index, size_t offset, void* args);


#define UNVISIT(visited) dynarr_forEach(visited, graph_private_reset, NULL); dynarr_free(visited);

enum {
    ARGS_FN = 0,
    ARGS_ARGS = 1,
    ARGS_VISITED = 2,
    ARGS_QUEUE = 3,
    ARGS_SIZE = 4
};

graph_edge* graph_createEdge(graph_node* src, double weight, graph_node* dest) {
    graph_edge* edge = malloc(sizeof(graph_edge));
    if(edge == NULL) return NULL;
    edge->src = src;
    edge->weight = weight;
    edge->dest = dest;
    return edge;
}

graph_node* graph_createNode(void* value, size_t nb_edges, graph_edge** edges) {
    graph_node* node = malloc(sizeof(graph_node));
    node->edges = DYNARR_INIT;
    for(size_t i = 0; i < nb_edges; i++) {
        dynarr_push(node->edges, edges[i]);
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
    while(dynarr_getSize(node->edges)) {
        free(dynarr_pop(node->edges));
    }
    dynarr_free(node->edges);
    free(node);
    return tmp;
}

void graph_addNEdges(graph_node* node, size_t nb_edges, graph_edge** edges) {
    for(size_t i = 0; i < nb_edges; i++) {
        dynarr_push(node->edges, edges[i]);
    }
}

inline void graph_addEdge(graph_node* node, graph_edge* edge) {
    graph_addNEdges(node, 1, &edge);
}

static void graph_private_addVisited(dynarr_arr* visited, graph_node* node) {
    if(node == NULL || node->visited) return;
    node->visited = true;
    dynarr_push(visited, node);
}

static void* graph_private_DFS_some(void* edge, size_t index, size_t offset, void* args) {
    return graph_private_DFS(((graph_edge*)edge)->dest, args);
}

static void* graph_private_DFS(graph_node* node, void** args) {
    if(node == NULL || node->visited) return NULL;
    graph_private_addVisited(args[ARGS_VISITED], node);
    void* test = dynarr_some(node->edges, graph_private_DFS_some, args);
    if(test != NULL) return test;
    void* (*todo_fn)(void*, void*) = args[ARGS_FN];
    return todo_fn(node->value, args[ARGS_ARGS]);
}

void* graph_DFS(graph_node* node, graph_todo_fn todo_fn, void* args) {
    if(node == NULL) return NULL;
    dynarr_arr* visited = DYNARR_INIT;
    void* _args[ARGS_SIZE];
    _args[ARGS_FN] = todo_fn;
    _args[ARGS_ARGS] = args;
    _args[ARGS_VISITED] = visited;
    void* tmp = graph_private_DFS(node, _args);
    UNVISIT(visited);
    return tmp;
}

static void* graph_private_BFS_addUnvisited(void* _edge, size_t index, size_t offset, void* args) {
    graph_edge* edge = _edge;
    if(edge->dest->visited == false) {
        dynarr_push(((void**)args)[ARGS_QUEUE], edge->dest);
        graph_private_addVisited(((void**)args)[ARGS_VISITED], edge->dest);
    }
    return NULL;
}

static void* graph_private_BFS(dynarr_arr* queue, void** args) {
    graph_node* currentNode = NULL;
    void* test = NULL;
    graph_todo_fn todo_fn = args[ARGS_FN];
    while(dynarr_getSize(queue)) {
        currentNode = dynarr_popFront(queue);
        test = todo_fn(currentNode->value, args[ARGS_ARGS]);
        if(test != NULL) return test;
        dynarr_forEach(currentNode->edges, graph_private_BFS_addUnvisited, args);
    }
    return NULL;
}

void* graph_BFS(graph_node* node, graph_todo_fn todo_fn, void* args) {
    if(node == NULL) return NULL;
    dynarr_arr* queue = DYNARR_INIT;
    dynarr_arr* visited = DYNARR_INIT;
    node->visited = true;
    dynarr_push(visited, node);
    dynarr_push(queue, node);
    void* _args[ARGS_SIZE];
    _args[ARGS_FN] = todo_fn;
    _args[ARGS_ARGS] = args;
    _args[ARGS_VISITED] = visited;
    _args[ARGS_QUEUE] = queue;
    void* tmp = graph_private_BFS(queue, _args);
    UNVISIT(visited);
    return tmp;
}

void graph_freeGraph(graph_node* node, void (*free_fn)(void*)) {
    if(node == NULL || node->visited) return;
    node->visited = true;
    graph_edge* edge = NULL;
    while(dynarr_getSize(node->edges)) {
        edge = dynarr_pop(node->edges);
        graph_freeGraph(edge->dest, free_fn);
        free(edge);
    }
    if(free_fn != NULL) free_fn(node->value);
    graph_freeNode(node);
}

static void* graph_private_reset(void* node, size_t index, size_t offset, void* args) {
    ((graph_node*)node)->pathEdge = NULL;
    ((graph_node*)node)->distance = 0;
    ((graph_node*)node)->visited = false;
    return NULL;
}

static int graph_private_sortDist(const void* a, const void* b) {
    const graph_node* ia = a;
    const graph_node* ib = b;
    const double da = ia->distance + ia->heuristic;
    const double db = ib->distance + ib->heuristic;
    return (da > db) - (da < db);
}

static void* graph_private_Astar_visitNeighbours(void* _edge, size_t index, size_t offset, void* _args) {
    void** args = _args;
    graph_edge* edge = _edge;
    graph_node* parent = edge->src;
    if(parent->distance + edge->weight < edge->dest->distance) {
        edge->dest->pathEdge = edge;
        edge->dest->distance = parent->distance + edge->weight;
    }
    if(edge->dest->visited == false){
        dynarr_push(args[ARGS_QUEUE], edge->dest);
        graph_private_addVisited(args[ARGS_VISITED], edge->dest);
        if(args[ARGS_FN] != NULL) {
            graph_heuristic_fn heuristic_fn = args[ARGS_FN];
            edge->dest->heuristic = heuristic_fn(edge->dest->value, args[ARGS_ARGS]);
        }
    } 
    return NULL;
}

static dynarr_arr* graph_private_Astar(dynarr_arr* queue, graph_isGoal_fn isGoal_fn, void** args) {
    if(dynarr_getSize(queue) == 0) return NULL;
    graph_node* currentNode = dynarr_popFront(queue);
    if(isGoal_fn(currentNode->value, args[ARGS_ARGS])) {
        dynarr_arr* path = DYNARR_INIT;
        while(currentNode != NULL) {
            dynarr_pushFront(path, currentNode->pathEdge);
            currentNode = currentNode->pathEdge->dest;
        }
        return path;
    }
    dynarr_forEach(currentNode->edges,graph_private_Astar_visitNeighbours,args);
    dynarr_qsort(queue, graph_private_sortDist);
    return graph_private_Astar(queue, isGoal_fn, args);
}

dynarr_arr* graph_Astar(graph_node* node, void* goalInfo, graph_isGoal_fn isGoal_fn, graph_heuristic_fn heuristic_fn) {
    if(node == NULL) return NULL;
    dynarr_arr* queue = DYNARR_INIT;
    dynarr_arr* visited = DYNARR_INIT;
    node->visited = true;
    dynarr_push(visited, node);
    dynarr_push(queue, node);
    void* args[ARGS_SIZE];
    args[ARGS_FN] = heuristic_fn;
    args[ARGS_ARGS] = goalInfo;
    args[ARGS_VISITED] = visited;
    dynarr_arr* tmp = graph_private_Astar(queue, isGoal_fn, args);
    UNVISIT(visited);
    dynarr_free(queue);
    return tmp;
}