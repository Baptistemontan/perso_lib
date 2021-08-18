#include "../headers/dynarr_t.h"

#include <string.h>

#define SHIFT(n) (1 << n) // fast 2^n
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))

#include <stdio.h>

typedef struct {
    void* baseArr; // adress of the allocated array
    size_t baseSize; // log2 of the allocated size for the array
    size_t size; // number of elem in arr
    size_t offset; // discarded element beetween baseAr and arr
    size_t byteSize; // size of 1 element
} dynarr_t;

static dynarr_t* dynarr_init(size_t byteSize, size_t size) {
    dynarr_t* arr = malloc(sizeof(dynarr_t));
    arr->size = size;
    arr->baseSize = LOG2(size ? size : 1) + 1;
    void* baseArr = malloc((byteSize * SHIFT(arr->baseSize)) + sizeof(dynarr_t*));
    arr->baseArr = baseArr + sizeof(dynarr_t*);
    memcpy(baseArr, &arr, sizeof(dynarr_t*));
    arr->offset = 0;
    arr->byteSize = byteSize;
    return arr;
}

static void dynarr_resize(dynarr_t* arr, size_t newBaseSize) {
    void* newArr = malloc(arr->byteSize * SHIFT(newBaseSize) + sizeof(dynarr_t*));
    memcpy(newArr + sizeof(dynarr_t*), arr->baseArr + arr->offset * arr->byteSize, arr->size * arr->byteSize);
    memcpy(newArr, &arr, sizeof(dynarr_t*));
    free(arr->baseArr - sizeof(dynarr_t*));
    arr->baseArr = newArr + sizeof(dynarr_t*);
    arr->baseSize = newBaseSize;
    arr->offset = 0;
}

static void dynarr_extend(dynarr_t* arr) {
    if(arr->size + arr->offset < SHIFT(arr->baseSize)) return;
    size_t newBaseSize = arr->baseSize + 1;
    dynarr_resize(arr, newBaseSize);
}

static void dynarr_reduce(dynarr_t* arr) {
    if(arr->baseSize == 0) return;
    if(arr->size * 2 > SHIFT(arr->baseSize)) return;
    size_t newBaseSize = arr->baseSize - 1;
    dynarr_resize(arr, newBaseSize);
}

void* dynarr_create(size_t byteSize, size_t size) {
    if(byteSize == 0) return NULL;
    dynarr_t* darr = dynarr_init(byteSize, size);
    return darr->baseArr;
}

static dynarr_t* dynarr_getInfo(void* arr) {
    dynarr_t** arrInfo = arr - sizeof(dynarr_t*);
    return *arrInfo;
}

static void dynarr_pushBack(dynarr_t* arr, void* value) {
    dynarr_extend(arr);
    memcpy(arr->baseArr + (arr->offset + arr->size) * arr->byteSize, value, arr->byteSize);
    arr->size++;
}

void _dynarr_priv_pushBack(void** arrPtr, void* value) {
    if(value == NULL || arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    dynarr_pushBack(tmp, value);
    *arrPtr = tmp->baseArr + tmp->offset * tmp->byteSize;
}

static void dynarr_pushFront(dynarr_t* arr, void* value) {
    if(arr->offset > 0) {
        arr->offset--;
        arr->size++;
        memcpy(arr->baseArr + arr->offset * arr->byteSize, value, arr->byteSize);
        memcpy(arr->baseArr + arr->offset * arr->byteSize - sizeof(dynarr_t*), &arr, sizeof(dynarr_t*));
    } else {
        dynarr_extend(arr);
        arr->offset = SHIFT(arr->baseSize) - arr->size;
        memmove(arr->baseArr + arr->offset * arr->byteSize, arr->baseArr, arr->size * arr->byteSize);
        dynarr_pushFront(arr,value);
    }
}

void _dynarr_priv_pushFront(void** arrPtr, void* value) {
    if(value == NULL || arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    dynarr_pushFront(tmp, value);
    *arrPtr = tmp->baseArr + tmp->offset * tmp->byteSize;
}

size_t dynarr_size(void* arr) {
    if(arr == NULL) return 0;
    dynarr_t* tmp = dynarr_getInfo(arr);
    return tmp->size;
}

void dynarr_free(void* arr) {
    if(arr == NULL) return;
    dynarr_t* arrInfo = *(dynarr_t**)(arr - sizeof(dynarr_t*));
    free(arrInfo->baseArr - sizeof(dynarr_t*));
    free(arrInfo);
}


static void dynarr_popBack(dynarr_t* arr, void* buff) {
    if(arr->size == 0) return;
    memcpy(buff, arr->baseArr + (arr->offset + arr->size - 1) * arr->byteSize, arr->byteSize);
    arr->size--;
    dynarr_reduce(arr);
}

void _dynarr_priv_popBack(void** arrPtr, void* buff) {
    if(arrPtr == NULL || *arrPtr == NULL || buff == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    if(tmp->size == 0) return;
    dynarr_popBack(tmp, buff);
    *arrPtr = tmp->baseArr + tmp->offset * tmp->byteSize;
}

static void dynarr_popFront(dynarr_t* arr, void* buff) {
    if(arr->size == 0) return;
    memcpy(buff, arr->baseArr + arr->offset * arr->byteSize, arr->byteSize);
    arr->size--;
    arr->offset++;
    memcpy(arr->baseArr + arr->offset * arr->byteSize - sizeof(dynarr_t*), &arr, sizeof(dynarr_t*));
    dynarr_reduce(arr);
}

void _dynarr_priv_popFront(void** arrPtr, void* buff) {
    if(arrPtr == NULL || *arrPtr == NULL || buff == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    if(tmp->size == 0) return;
    dynarr_popFront(tmp, buff);
    *arrPtr = tmp->baseArr + tmp->offset * tmp->byteSize;
}

void dynarr_qsort(void* arr, __compar_fn_t compar) {
    if(arr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(arr);
    qsort(arr, tmp->size, tmp->byteSize, compar);
}