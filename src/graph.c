#include "../headers/graph.h"

static void* graph_private_DFS(graph_node* node, void** args);


#define UNVISIT(visited) dynarr_forEach(visited, graph_private_unvisit, NULL); dynarr_free(visited);
#define RESET(visited) dynarr_forEach(visited, graph_private_reset, NULL); dynarr_free(visited);

enum {
    ARGS_ARGS_PARENT = 0,
    ARGS_ARGS_GOALINFO = 1,
    ARGS_FN = 0,
    ARGS_ARGS = 1,
    ARGS_VISITED = 2,
    ARGS_QUEUE = 3,
    ARGS_SIZE = 4
};

graph_edge* graph_createEdge(size_t weight, graph_node* dest) {
    graph_edge* edge = malloc(sizeof(graph_edge));
    if(edge == NULL) return NULL;
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
    node->parent = NULL;
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

static void* graph_private_unvisit(void* node, size_t index, size_t offset, void* args) {
    ((graph_node*)node)->visited = false;
    return NULL;
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

void* graph_DFS(graph_node* node, void* (*todo_fn)(void* value, void* args), void* args) {
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

static void* graph_private_BFS_some(void* _node, size_t index, size_t offset, void* args) {
    graph_node* node = _node;
    dynarr_forEach(node->edges, graph_private_BFS_addUnvisited, args);
    void* (*todo_fn)(void*, void*) = ((void**)args)[ARGS_FN];
    return todo_fn(node->value, ((void**)args)[ARGS_ARGS]);
}

static void* graph_private_BFS(dynarr_arr* queue, void** args) {
    if(dynarr_getSize(queue) == 0) {
        dynarr_free(queue);
        return NULL;
    }
    args[ARGS_QUEUE] = DYNARR_INIT;
    void* test = dynarr_some(queue, graph_private_BFS_some, args);
    dynarr_free(queue);
    if(test != NULL) {
        dynarr_free(args[ARGS_QUEUE]);
        return test;
    }
    return graph_private_BFS(args[ARGS_QUEUE], args);
}

void* graph_BFS(graph_node* node, void* (*todo_fn)(void* value, void* args), void* args) {
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
    void* tmp = graph_private_BFS(queue, _args);
    UNVISIT(visited);
    return tmp;
}

void graph_freeGraph(graph_node* node, void (*free_fn)(void*)) {
    if(node == NULL || node->visited) return;
    node->visited = true;
    graph_edge* edge;
    while(dynarr_getSize(node->edges)) {
        edge = dynarr_pop(node->edges);
        graph_freeGraph(edge->dest, free_fn);
        free(edge);
    }
    if(free_fn != NULL) free_fn(node->value);
    graph_freeNode(node);
}

static void* graph_private_reset(void* node, size_t index, size_t offset, void* args) {
    ((graph_node*)node)->parent = NULL;
    ((graph_node*)node)->distance = 0;
    ((graph_node*)node)->visited = false;
    return NULL;
}

static int graph_private_sortDist(const void* a, const void* b) {
    const graph_node* ia = a;
    const graph_node* ib = b;
    const size_t da = ia->distance + ia->heuristic;
    const size_t db = ib->distance + ib->heuristic;
    return (da > db) - (da < db);
}

static void* graph_private_Astar_visitNeighbours(void* _edge, size_t index, size_t offset, void* args) {
    graph_node* parent = ((void***)args)[ARGS_ARGS][ARGS_ARGS_PARENT];
    graph_edge* edge = _edge;
    if(parent->distance + edge->weight < edge->dest->distance) {
        edge->dest->parent = parent;
        edge->dest->distance = parent->distance + edge->weight;
    }
    if(edge->dest->visited == false){
        dynarr_push(((void**)args)[ARGS_QUEUE], edge->dest);
        graph_private_addVisited(((void**)args)[ARGS_VISITED], edge->dest);
        if(((void**)args)[ARGS_FN] != NULL) {
            size_t (*heuristic_fn)(void*, void*) = ((void**)args)[ARGS_FN];
            edge->dest->heuristic = heuristic_fn(edge->dest, ((void***)args)[ARGS_ARGS][ARGS_ARGS_GOALINFO]);
        }
    } 
    return NULL;
}

static dynarr_arr* graph_private_Astar(dynarr_arr* queue, bool (*isGoal_fn)(void* node, void* goalInfo), void** args) {
    if(dynarr_getSize(queue) == 0) return NULL;
    graph_node* currentNode = dynarr_popFront(queue);
    if(isGoal_fn(currentNode, ((void**)(args[ARGS_ARGS]))[ARGS_ARGS_GOALINFO])) {
        dynarr_arr* path = DYNARR_INIT;
        while(currentNode != NULL) {
            dynarr_pushFront(path, currentNode);
            currentNode = currentNode->parent;
        }
        return path;
    }
    ((void**)(args[ARGS_ARGS]))[ARGS_ARGS_PARENT] = currentNode;
    dynarr_forEach(currentNode->edges,graph_private_Astar_visitNeighbours,args);
    dynarr_qsort(queue, graph_private_sortDist);
    return graph_private_Astar(queue, isGoal_fn, args);
}

dynarr_arr* graph_Astar(graph_node* node, void* goalInfo, bool (*isGoal_fn)(void* node, void* goalInfo), size_t (*heuristic_fn)(void* node, void* goalInfo)) {
    if(node == NULL) return NULL;
    dynarr_arr* queue = DYNARR_INIT;
    dynarr_arr* visited = DYNARR_INIT;
    node->visited = true;
    dynarr_push(visited, node);
    dynarr_push(queue, node);
    void* args[ARGS_SIZE];
    args[ARGS_FN] = heuristic_fn;
    void* args_args[2];
    args_args[ARGS_ARGS_GOALINFO] = goalInfo;
    args[ARGS_ARGS] = args_args;
    args[ARGS_VISITED] = visited;
    dynarr_arr* tmp = graph_private_Astar(queue, isGoal_fn, args);
    RESET(visited);
    dynarr_free(queue);
    return tmp;
}