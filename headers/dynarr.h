#ifndef HEAD_DYNARR
#define HEAD_DYNARR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// return an address of an array
// must be free with dynarr_free()
// memSize is the size of one element like sizeof(int)
void* dynarr_create(size_t memSize);

// push the value of the ptr to the back of the given array
// take the adress of the array ptr in arguments
// return value_ptr for chaining
// if arr_ptr or value_ptr is not valid, just return NULL
void* dynarr_pushBack(void* arr_ptr, void* value_ptr);

// push the value of the ptr to the front of the given array
// take the adress of the array ptr in arguments
// return value_ptr for chaining
// if arr_ptr or value_ptr is not valid, just return NULL
void* dynarr_pushFront(void* arr_ptr, void* value_ptr);

// return the size of the given arr
// if the given adress is not valid return 0
size_t dynarr_size(void* arr);

// free the given arr
void dynarr_free(void* arr);

// pop the value at the back of the given array
// take the adress of the array ptr in arguments
// return an adress to a buffer containing the popped value
// can be casted and used
// the adress is temporary and can be destroyed at any time
// the returned adress MUST NOT be free
// if arr_ptr is not valid, just return NULL
void* dynarr_popBack(void* arr_ptr);

// pop the value at the front of the given array
// take the adress of the array ptr in arguments
// return an adress to a buffer containing the popped value
// can be casted and used, but the returned adress is temporary
// if arr_ptr is not valid, just return NULL
void* dynarr_popFront(void* arr_ptr);

// return the adress of the buffer pop Buffer
// the adress is temporary and can be destroyed at any time
// the returned adress MUST NOT be free
void* dynarr_lastDelElem();

// sort the entire array with the compare fonction
// just like a normal qsort but the memSize, nmemb, ect is handled by the lib
void dynarr_qsort(void* arr, __compar_fn_t compar);


#endif