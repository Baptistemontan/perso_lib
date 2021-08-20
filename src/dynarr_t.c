#include "../headers/dynarr_t.h"

#include <string.h>
#include <stdio.h>

#define SHIFT(n) (1 << n) // fast 2^n
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1)) // this come from stackoverflow, I don't know how it works, but it works
#define dynarr_getInfo(arr) (*(dynarr_t**)((arr) - sizeof(dynarr_t*)))
#define dynarr_front(arr) ((arr)->baseArr + ((arr)->offset * (arr)->memSize))
#define dynarr_back(arr) ((arr)->baseArr + (((arr)->offset + (arr)->size) * (arr)->memSize))
#define dynarr_index(arr, i) ((arr)->baseArr + (((arr)->offset + (i)) * (arr)->memSize))
#define dynarr_indexFromBack(arr, i) ((arr)->baseArr + (((arr)->offset + (arr)->size - (i)) * (arr)->memSize))

static void*(*allocator)(size_t) = malloc;
static void(*deallocator)(void*) = free;

// size_t for everything is kinda overkill but meh,
// struct is 40 bytes and storing the adress is 8 more
// thats a total of 48 bytes per array to store info
// thats the size of an array of 12 int.
typedef struct {
    void* baseArr; // adress of the allocated array
    size_t baseSize; // log2 of the allocated size for the array
    size_t size; // number of elem in arr
    size_t offset; // discarded element beetween baseAr and arr
    size_t memSize; // size of 1 element
} dynarr_t;

static dynarr_t* dynarr_init(size_t memSize, size_t size) {
    dynarr_t* arr = allocator(sizeof(dynarr_t));
    if(arr == NULL) {
        fprintf(stderr, "dynarr_init: malloc failed, requested size: %zu\n", sizeof(dynarr_t));
        return NULL;
    }
    arr->size = size;
    arr->baseSize = LOG2(size ? size : 1) + 1;
    void* baseArr = allocator((memSize * SHIFT(arr->baseSize)) + sizeof(dynarr_t*));
    if(baseArr == NULL) {
        fprintf(stderr, "dynarr_init: malloc failed, requested size: %zu\n", (memSize * SHIFT(arr->baseSize)) + sizeof(dynarr_t*));
        deallocator(arr);
        return NULL;
    }
    arr->baseArr = baseArr + sizeof(dynarr_t*);
    memcpy(baseArr, &arr, sizeof(dynarr_t*));
    arr->offset = 0;
    arr->memSize = memSize;
    return arr;
}

static void dynarr_resize(dynarr_t* arr, size_t newBaseSize) {
    void* newArr = allocator(arr->memSize * SHIFT(newBaseSize) + sizeof(dynarr_t*));
    if(newArr == NULL) {
        fprintf(stderr, "dynarr_resize: malloc failed, requested size: %zu\n", arr->memSize * SHIFT(newBaseSize) + sizeof(dynarr_t*));
        return;
    }
    memcpy(newArr + sizeof(dynarr_t*), dynarr_front(arr), arr->size * arr->memSize);
    memcpy(newArr, &arr, sizeof(dynarr_t*));
    deallocator(arr->baseArr - sizeof(dynarr_t*));
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

void* dynarr_create(size_t memSize, size_t size) {
    if(memSize == 0) return NULL;
    dynarr_t* darr = dynarr_init(memSize, size);
    if(darr == NULL) return NULL;
    return darr->baseArr;
}

static void dynarr_pushBack(dynarr_t*__restrict__ arr, void*__restrict__ value) {
    dynarr_extend(arr);
    memcpy(dynarr_back(arr), value, arr->memSize);
    arr->size++;
}

void _dynarr_priv_pushBack(void** arrPtr, void* value) {
    if(value == NULL || arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    dynarr_pushBack(tmp, value);
    *arrPtr = dynarr_front(tmp);
}

static void dynarr_pushFront(dynarr_t* arr, void*__restrict__ value) {
    if(arr->offset > 0) {
        arr->offset--;
        arr->size++;
        memcpy(dynarr_front(arr), value, arr->memSize);
        memcpy(dynarr_front(arr) - sizeof(dynarr_t*), &arr, sizeof(dynarr_t*));
    } else {
        // create room if needed
        dynarr_extend(arr);
        // shift everything to back of the array
        arr->offset = SHIFT(arr->baseSize) - arr->size;
        // if no offset (should not be possible) do nothing,
        // this is to avoid infinite recursive calls
        if(arr->offset == 0) return;
        // need memmove here because everything is moved over itself
        memmove(dynarr_front(arr), arr->baseArr, arr->size * arr->memSize);
        // retry to push the value
        dynarr_pushFront(arr,value);
    }
}

void _dynarr_priv_pushFront(void** arrPtr, void* value) {
    if(value == NULL || arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    dynarr_pushFront(tmp, value);
    *arrPtr = dynarr_front(tmp);
}

size_t dynarr_size(const void* arr) {
    if(arr == NULL) return 0;
    const dynarr_t* tmp = dynarr_getInfo(arr);
    return tmp->size;
}

void dynarr_free(void* arr) {
    if(arr == NULL) return;
    dynarr_t* arrInfo = *(dynarr_t**)(arr - sizeof(dynarr_t*));
    deallocator(arrInfo->baseArr - sizeof(dynarr_t*));
    deallocator(arrInfo);
}


static void dynarr_popBack(dynarr_t*__restrict__ arr, void*__restrict__ buff) {
    if(arr->size == 0) return;
    if(buff != NULL) memcpy(buff, dynarr_indexFromBack(arr, 1), arr->memSize);
    arr->size--;
    dynarr_reduce(arr);
}

void _dynarr_priv_popBack(void** arrPtr, void* buff) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    if(tmp->size == 0) return;
    dynarr_popBack(tmp, buff);
    *arrPtr = dynarr_front(tmp);
}

static void dynarr_popFront(dynarr_t* arr, void*__restrict__ buff) {
    if(arr->size == 0) return;
    if(buff != NULL) memcpy(buff, arr->baseArr + arr->offset * arr->memSize, arr->memSize);
    arr->size--;
    arr->offset++;
    memcpy(dynarr_front(arr) - sizeof(dynarr_t*), &arr, sizeof(dynarr_t*));
    dynarr_reduce(arr);
}

void _dynarr_priv_popFront(void** arrPtr, void* buff) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    if(tmp->size == 0) return;
    dynarr_popFront(tmp, buff);
    *arrPtr = dynarr_front(tmp);
}

void dynarr_qsort(void* arr, int (*compar_fn) (const void *, const void *)) {
    if(arr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(arr);
    qsort(arr, tmp->size, tmp->memSize, compar_fn);
}

void* _dynarr_priv_slice(void* arr, size_t start, size_t end) {
    if(arr == NULL) return NULL;
    dynarr_t* tmp = dynarr_getInfo(arr);
    if(start >= tmp->size) return dynarr_create(tmp->memSize, 0);
    if(end > tmp->size) end = tmp->size;
    void* newArr = dynarr_create(tmp->memSize, end - start);
    memcpy(newArr, dynarr_index(tmp, start), (end - start) * tmp->memSize);
    return newArr;
}

static void dynarr_insert(dynarr_t* arr, size_t index, void* value) {
    if(index > arr->size) return;
    dynarr_extend(arr);
    // need memmove here because everything is moved over itself by one element
    memmove(dynarr_index(arr, index + 1), dynarr_index(arr, index), (arr->size - index) * arr->memSize);
    memcpy(dynarr_index(arr, index), value, arr->memSize);
    arr->size++;
}

void _dynarr_priv_insert(void** arrPtr, size_t index, void* value) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    if(index >= tmp->size) {
        dynarr_pushBack(tmp, value);
        return;
    }
    if(index == 0) {
        dynarr_pushFront(tmp, value);
        return;
    }
    dynarr_insert(tmp, index, value);
    *arrPtr = dynarr_front(tmp);
}

void _dynarr_priv_remove(void** arrPtr, size_t index, void*__restrict__ buff) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    if(index >= tmp->size) return;
    if(buff != NULL) memcpy(buff, dynarr_index(tmp, index), tmp->memSize);
    // need memmove here because everything is moved over itself by one element
    memmove(dynarr_index(tmp, index), dynarr_index(tmp, index + 1), (tmp->size - index - 1) * tmp->memSize);
    tmp->size--;
    dynarr_reduce(tmp);
    *arrPtr = dynarr_front(tmp);
}

void dynarr_swap(void* arr, size_t index1, size_t index2) {
    if(arr == NULL || index1 == index2) return;
    dynarr_t* tmp = dynarr_getInfo(arr);
    if(index1 >= tmp->size || index2 >= tmp->size) return;
    // no need for memmove here as index1 and index2 are always different
    // so dest and src can't overlap

    // the two possibility here avoid the need of a temp buffer
    // by writing the values directly to unused memory of the array
    if(tmp->offset != 0) {
        memcpy(tmp->baseArr, dynarr_index(tmp, index1), tmp->memSize);
        memcpy(dynarr_index(tmp, index1), dynarr_index(tmp, index2), tmp->memSize);
        memcpy(dynarr_index(tmp, index2), tmp->baseArr, tmp->memSize);
        return;
    } else if(SHIFT(tmp->baseSize) > tmp->size) {
        memcpy(dynarr_back(tmp), dynarr_index(tmp, index1), tmp->memSize);
        memcpy(dynarr_index(tmp, index1), dynarr_index(tmp, index2), tmp->memSize);
        memcpy(dynarr_index(tmp, index2), dynarr_back(tmp), tmp->memSize);
        return;
    }

    void* buff = allocator(tmp->memSize);
    if(buff == NULL) {
        fprintf(stderr, "dynarr_swap: buffer malloc failed, requested size: %zu\n", tmp->memSize);
        return;
    }
    memcpy(buff, dynarr_index(tmp, index1), tmp->memSize);
    memcpy(dynarr_index(tmp, index1), dynarr_index(tmp, index2), tmp->memSize);
    memcpy(dynarr_index(tmp, index2), buff, tmp->memSize);
    deallocator(buff);
}

void _dynarr_priv_clear(void** arrPtr) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*arrPtr);
    tmp->size = 0;
    dynarr_resize(tmp, 0);
    *arrPtr = dynarr_front(tmp);
}


void _dynarr_debug_print(void* arr, FILE* stream) {
    if(arr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(arr);
    fprintf(stream, "size: %lu, offset: %lu, memSize: %lu, baseSize: %lu\n", tmp->size, tmp->offset, tmp->memSize, tmp->baseSize);
    fprintf(stream, "effective memsize: %lu\n", SHIFT(tmp->baseSize) * tmp->memSize + sizeof(dynarr_t) + sizeof(dynarr_t*));
}

static void dynarr_preReserve(dynarr_t* arr, size_t newSize) {
    if(newSize <= arr->baseSize) return;
    size_t newBaseSize = LOG2(newSize ? newSize : 1) + 1;
    if(newBaseSize > arr->baseSize) {
        dynarr_resize(arr, newBaseSize);
    }
}


void dynarr_allocate(void* arrPtr, size_t newSize, int resize) {
    if(arrPtr == NULL || *(void**)arrPtr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(*(void**)arrPtr);
    dynarr_preReserve(tmp, newSize);
    if(resize) tmp->size = newSize;
    *(void**)arrPtr = dynarr_front(tmp);
}

void dynarr_reverse(void* arr) {
    if(arr == NULL) return;
    dynarr_t* tmp = dynarr_getInfo(arr);
    // no need for memmove here as i is strictly inferior to j,
    // so i always different from j
    // so dest and src can't overlap

    // the two possibility here avoid the need of a temp buffer
    // by writing the values directly to the array
    if(tmp->offset != 0) {
        for(size_t i = 0, j = tmp->size - 1; i < j; i++, j--) {
            memcpy(tmp->baseArr, dynarr_index(tmp, i), tmp->memSize);
            memcpy(dynarr_index(tmp, i), dynarr_index(tmp, j), tmp->memSize);
            memcpy(dynarr_index(tmp, j), tmp->baseArr, tmp->memSize);
        }
        return;
    } else if(SHIFT(tmp->baseSize) > tmp->size) {
        for(size_t i = 0, j = tmp->size - 1; i < j; i++, j--) {
            memcpy(dynarr_back(tmp), dynarr_index(tmp, i), tmp->memSize);
            memcpy(dynarr_index(tmp, i), dynarr_index(tmp, j), tmp->memSize);
            memcpy(dynarr_index(tmp, j), dynarr_back(tmp), tmp->memSize);
        }
        return;
    }
    void* buff = allocator(tmp->memSize);
    if(buff == NULL) {
        fprintf(stderr, "dynarr_reverse: buffer malloc failed, requested size: %zu\n", tmp->memSize);
        return;
    }
    for(size_t i = 0, j = tmp->size - 1; i < j; i++, j--) {
        memcpy(buff, dynarr_index(tmp, i), tmp->memSize);
        memcpy(dynarr_index(tmp, i), dynarr_index(tmp, j), tmp->memSize);
        memcpy(dynarr_index(tmp, j), buff, tmp->memSize);
    }
    deallocator(buff);
}

void dynarr_set_allocator(void* (*_allocator)(size_t)) {
    allocator = _allocator;
}

void dynarr_set_deallocator(void (*_deallocator)(void*)) {
    deallocator = _deallocator;
}


