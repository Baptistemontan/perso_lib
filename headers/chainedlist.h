#ifndef HEAD_CHAINEDLIST
#define HEAD_CHAINEDLIST

#include <stdio.h>
#include <stdlib.h>

#define CL_INIT NULL

// macro for sorting an entire list
#define cl_qsort(list, comp_fn) cl_qsort_interval(list, NULL, comp_fn)


typedef struct cl_node {
    struct cl_node *next;
    void *val; 
} cl_node;


typedef void (*cl_forEach_fn)(void* value, size_t index);

// free the given ptr
typedef void (*cl_free_fn)(void* ptr);

// compare 2 value
// return 0 if equal
typedef int (*cl_comp_fn)(const void* nodeValue, const void* value);

// add the given value at the top of the chained list
// return the given value
void* cl_add(cl_node** head, void* value);

// add the given value at the end of the chained list
// return the given value
void* cl_addEnd(cl_node** head, void* value);

// remove the first node of the given list and return its value
// return NULL if the list is empty
void* cl_remove(cl_node** head);

// remove the node at the given index and return its value
// return NULL if the given index does'nt exist
void* cl_removeIndex(cl_node** head, size_t index);

// return the value at the given index
// return NULL if the given index does'nt exist
void* cl_get(const cl_node* head, size_t index);

// pass every value in the given list to the given function
void cl_forEach(cl_node* head, cl_forEach_fn todo_fn);

// free the entire list and pass all values to free_fn
// just destroy the list if free_fn is NULL
void cl_free(cl_node* head, cl_free_fn free_fn);

// find the index of the given value with the compare func
// directly compare the values if comp_fn is NULL
// return -1 if the value is not in the list
int cl_findIndex(const cl_node* head, const void* value, cl_comp_fn comp_fn);

// sort the list between start and end
// sort everything from start to the end of the list if end = NULL
// THIS IS NOT A STABLE ALGO
void cl_qsort_interval(cl_node* start, cl_node* end, __compar_fn_t comp_fn);


#endif