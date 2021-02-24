#ifndef HEAD_DYNARR
#define HEAD_DYNARR

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DYNARR_INIT(type) dynarr_create(sizeof(type), 0, NULL)

void* dynarr_create(size_t byteSize, size_t size, void* value);
void* dynarr_pushBack(void* arrAdd, void* value);
void* dynarr_pushFront(void* arrAdd, void* value);
size_t dynarr_size(void* arr);
void dynarr_free(void* arr);
void* dynarr_popBack(void* arrAdd);
void* dynarr_popFront(void* arrAdd);
void dynarr_qsort(void* arr, __compar_fn_t compar);


#endif