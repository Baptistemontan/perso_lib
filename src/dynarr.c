#include "../headers/dynarr.h"

#define SHIFT(n) (1 << n) // fast 2^n
#define SAVEBUFF(src, byteSize) free(tmpBuff); tmpBuff = malloc(byteSize); memcpy(tmpBuff,src,byteSize);

typedef struct {
    void* baseArr; // adress of the allocated array
    size_t baseSize; // log2 of the allocated size for the array
    size_t size; // number of elem in arr
    size_t offset; // discarded element beetween baseAr and arr
    size_t byteSize; // size of 1 element
} dynarr_arr;

static dynarr_arr* dynarr_private_init(size_t byteSize);
static void dynarr_private_popBack(dynarr_arr* arr);

// temp buff for pop functions
static void* tmpBuff = NULL;

static void dynarr_private_extend(dynarr_arr* arr);
static void dynarr_private_reduce(dynarr_arr* arr);
static void dynarr_private_pushBack(dynarr_arr* arr, void* value);

static dynarr_arr* dynarr_private_getInfo(void* arr) {
    dynarr_arr** arrInfo = arr - sizeof(dynarr_arr*);
    return *arrInfo;
}

static dynarr_arr* dynarr_private_init(size_t byteSize) {
    dynarr_arr* arr = malloc(sizeof(dynarr_arr));
    arr->baseSize = 0;
    arr->size = arr->offset = 0;
    arr->byteSize = byteSize;
    void* baseArr = malloc(byteSize + sizeof(dynarr_arr*));
    arr->baseArr = baseArr + sizeof(dynarr_arr*);
    memcpy(baseArr, &arr, sizeof(dynarr_arr*));
    return arr;
}

void* dynarr_create(size_t byteSize) {
    if(byteSize == 0) return NULL;
    dynarr_arr* darr = dynarr_private_init(byteSize);
    return darr->baseArr;
}

static void dynarr_private_resize(dynarr_arr* arr, size_t newBaseSize) {
    void* newArr = malloc(arr->byteSize * SHIFT(newBaseSize) + sizeof(dynarr_arr*));
    memcpy(newArr + sizeof(dynarr_arr*), arr->baseArr + arr->offset * arr->byteSize, arr->size * arr->byteSize);
    memcpy(newArr, &arr, sizeof(dynarr_arr*));
    free(arr->baseArr - sizeof(dynarr_arr*));
    arr->baseArr = newArr + sizeof(dynarr_arr*);
    arr->baseSize = newBaseSize;
    arr->offset = 0;
}

static void dynarr_private_extend(dynarr_arr* arr) {
    if(arr->size + arr->offset < SHIFT(arr->baseSize)) return;
    size_t newBaseSize = arr->baseSize + 1;
    dynarr_private_resize(arr, newBaseSize);
}

static void dynarr_private_reduce(dynarr_arr* arr) {
    if(arr->size * 2 > SHIFT(arr->baseSize)) return;
    if(arr->baseSize == 0) return;
    size_t newBaseSize = arr->baseSize - 1;
    dynarr_private_resize(arr, newBaseSize);
}

static void dynarr_private_pushBack(dynarr_arr* arr, void* value) {
    dynarr_private_extend(arr);
    memcpy(arr->baseArr + (arr->offset + arr->size) * arr->byteSize, value, arr->byteSize);
    arr->size++;
}

void* dynarr_pushBack(void* arrAdd, void* value) {
    if(value == NULL || arrAdd == NULL) return NULL;
    dynarr_arr* tmp = dynarr_private_getInfo(*(void**)arrAdd);
    dynarr_private_pushBack(tmp, value);
    *(void**)arrAdd = tmp->baseArr + tmp->offset * tmp->byteSize;
    return value;
}

static void dynarr_private_pushFront(dynarr_arr* arr, void* value) {
    if(arr->offset > 0) {
        arr->offset--;
        arr->size++;
        memcpy(arr->baseArr + arr->offset * arr->byteSize, value, arr->byteSize);
        memcpy(arr->baseArr + arr->offset * arr->byteSize - sizeof(dynarr_arr*), &arr, sizeof(dynarr_arr*));
    } else {
        dynarr_private_extend(arr);
        arr->offset = SHIFT(arr->baseSize) - arr->size;
        memmove(arr->baseArr + arr->offset * arr->byteSize, arr->baseArr, arr->size * arr->byteSize);
        dynarr_private_pushFront(arr,value);
    }
}

void* dynarr_pushFront(void* arrAdd, void* value) {
    if(value == NULL || arrAdd == NULL) return NULL;
    dynarr_arr* tmp = dynarr_private_getInfo(*(void**)arrAdd);
    dynarr_private_pushFront(tmp, value);
    *(void**)arrAdd = tmp->baseArr + tmp->offset * tmp->byteSize;
    return value;
}

size_t dynarr_size(void* arr) {
    dynarr_arr* tmp = dynarr_private_getInfo(arr);
    return tmp->size;
}

void dynarr_free(void* arr) {
    if(arr == NULL) return;
    dynarr_arr* arrInfo = *(dynarr_arr**)(arr - sizeof(dynarr_arr*));
    free(arrInfo->baseArr - sizeof(dynarr_arr*));
    free(arrInfo);
}

static void dynarr_private_popBack(dynarr_arr* arr) {
    if(arr->size == 0) return;
    SAVEBUFF(arr->baseArr + (arr->offset + arr->size - 1) * arr->byteSize, arr->byteSize);
    arr->size--;
    dynarr_private_reduce(arr);
}

void* dynarr_popBack(void* arrAdd) {
    if(arrAdd == NULL) return NULL;
    dynarr_arr* tmp = dynarr_private_getInfo(*(void**)arrAdd);
    if(tmp->size == 0) return NULL;
    dynarr_private_popBack(tmp);
    *(void**)arrAdd = tmp->baseArr + tmp->offset * tmp->byteSize;
    return tmpBuff;
}

static void dynarr_private_popFront(dynarr_arr* arr) {
    if(arr->size == 0) return;
    SAVEBUFF(arr->baseArr + arr->offset * arr->byteSize, arr->byteSize);
    arr->size--;
    arr->offset++;
    memcpy(arr->baseArr + arr->offset * arr->byteSize - sizeof(dynarr_arr*), &arr, sizeof(dynarr_arr*));
    dynarr_private_reduce(arr);
}

void* dynarr_popFront(void* arrAdd) {
    if(arrAdd == NULL) return NULL;
    dynarr_arr* tmp = dynarr_private_getInfo(*(void**)arrAdd);
    if(tmp->size == 0) return NULL;
    dynarr_private_popFront(tmp);
    *(void**)arrAdd = tmp->baseArr + tmp->offset * tmp->byteSize;
    return tmpBuff;
}

void* dynarr_lastDelElem() {
    return tmpBuff;
}

void dynarr_qsort(void* arr, __compar_fn_t compar) {
    if(arr == NULL) return;
    dynarr_arr* tmp = dynarr_private_getInfo(arr);
    qsort(arr, tmp->size, tmp->byteSize, compar);
}