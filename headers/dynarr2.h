#ifndef HEAD_DYNARR2
#define HEAD_DYNARR2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DYNARR_INIT(byteSize) dynarr_create(byteSize, 0, NULL)

void* dynarr_create(size_t byteSize, size_t size, void* value);
void* dynarr_pushBack(void* arrAdd, void* value);
void* dynarr_pushFront(void* arrAdd, void* value);
size_t dynarr_size(void* arr);
void dynarr_free(void* arr);


#endif