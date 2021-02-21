#ifndef HEAD_DYNARR
#define HEAD_DYNARR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// create a new dynamic array
// must be free with dynarr_free()
#define DYNARR_INIT dynarr_new()
#define dynarr_qsort(arr, comp_fn) dynarr_qsort_interval(arr,0,0,comp_fn) 
#define dynarr_forEach(arr, todo_fn, args) dynarr_forEach_interval(arr,0,0,todo_fn, args)
#define dynarr_map(arr, map_fn) dynarr_map_interval(arr,0,0,map_fn)
#define dynarr_every(arr, test_fn, args) dynarr_every_interval(arr,0,0,test_fn, args)
#define dynarr_some(arr, test_fn, args) dynarr_some_interval(arr,0,0,test_fn, args)
#define dynarr_fill(arr, value) dynarr_fill_interval(arr,0,0,value)
#define dynarr_bsearch(arr, value, comp_fn) dynarr_bsearch_interval(arr,0,0,value,comp_fn)

#define DYNARR_MAP_NOT ((void*)-1)


typedef struct {
    void** arr;
    void** baseArr;
    size_t baseSize;
    size_t size;
    size_t offset;
} dynarr_arr;

typedef void* (*dynarr_forEach_fn)(void* value, size_t index, size_t offset, void* args);

typedef void* (*dynarr_map_fn)(const void* value, size_t index, size_t offset);

//initialize a dynamic array
dynarr_arr* dynarr_new();

// free the given dynamic arr
void dynarr_free(dynarr_arr* arr, void (*free_fn)(void*));

// push at the end of the array a new value
// return the value
void* dynarr_push(dynarr_arr* arr, void* value);

// pop the value at the end of the array
// return the value of the popped element
void* dynarr_pop(dynarr_arr* arr);

// push at value at the front of the array
// return the value
void* dynarr_pushFront(dynarr_arr* arr, void* value);

// pop the value at the front of the array
// return the value
void* dynarr_popFront(dynarr_arr* arr);

// return the value at the given index
// return NULL if the given index don't exist
void* dynarr_get(dynarr_arr* arr, size_t index);

// set the value at the given index to the given value
// return NULL if the given index don't exist
void* dynarr_set(dynarr_arr* arr, size_t index, void* value);

// sort the array for nmemb element from the index start
// sort all the element after start if nmemb == 0
// do nothing if start is greater than the last index
void dynarr_qsort_interval(dynarr_arr* arr, size_t start, size_t nmemb, __compar_fn_t comp_fn);

// pass to the func fonction nmemb value from the index start in the array and their respective index 
// pass all the element after start if nmemb == 0
// do nothing if start is greater than the last index
void dynarr_forEach_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_forEach_fn todo_fn, void* args);

// return a new dynamic array which nmemb elements of arr from the start index
// has been pass to func and the returned value pushed into the new array
// if nmemb == 0 every element from start and after are mapped
// if start is greater than the last index return NULL
// returned arr must be free with dynarr_free()
dynarr_arr* dynarr_map_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_map_fn map_fn);

// pass all nmemb element of arr from start to func
// if func return NULL for one element, the function return NULL
// else return the last returned adress of func
// pass all the element after start if nmemb == 0
// return NULL if start is greater than the last index
void* dynarr_every_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_forEach_fn test_fn, void* args);

// pass all nmemb element of arr from start to func
// if func return an adress != NULL one time, return the adress
// else return NULL
// pass all the element after start if nmemb == 0
// return NULL if start is greater than the last index
void* dynarr_some_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_forEach_fn test_fn, void* args);

// fill nmemb element of arr from start with the value value
// pass all the element after start if nmemb == 0
// does nothing if start is greater than the last index
// if start + nmemb is greater than the last index, the extra elements are created
void dynarr_fill_interval(dynarr_arr* arr, size_t start, size_t nmemb, void* value);

// return the size of the given arr
size_t dynarr_getSize(dynarr_arr* arr);

// execute a binary search in the array using comp_fn
// return the index of the first element in the array that match the value
// assume that the array is already sorted
// pass all the element after start if nmemb == 0
// return -1 if the searched element is not in the interval
long dynarr_bsearch_interval(dynarr_arr* arr, size_t start, size_t nmemb, void* value, __compar_fn_t comp_fn);

#endif