#include "../headers/vector.h"

#include <string.h>
#include <stdio.h>

#define SHIFT(n) (1 << n) // fast 2^n
#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1)) // this come from stackoverflow, I don't know how it works, but it works
#define vec_getInfo(arr) (*(vec_t**)((arr) - sizeof(vec_t*)))
#define vec_front(arr) ((arr)->baseArr + ((arr)->offset * (arr)->memSize))
#define vec_back(arr) ((arr)->baseArr + (((arr)->offset + (arr)->size) * (arr)->memSize))
#define vec_index(arr, i) ((arr)->baseArr + (((arr)->offset + (i)) * (arr)->memSize))
#define vec_indexFromBack(arr, i) ((arr)->baseArr + (((arr)->offset + (arr)->size - (i)) * (arr)->memSize))

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
} vec_t;

static vec_t* vec_init(size_t memSize, size_t size) {
    vec_t* arr = allocator(sizeof(vec_t));
    if(arr == NULL) {
        fprintf(stderr, "vec_init: malloc failed, requested size: %zu\n", sizeof(vec_t));
        return NULL;
    }
    arr->size = size;
    arr->baseSize = LOG2(size ? size : 1) + 1;
    void* baseArr = allocator((memSize * SHIFT(arr->baseSize)) + sizeof(vec_t*));
    if(baseArr == NULL) {
        fprintf(stderr, "vec_init: malloc failed, requested size: %zu\n", (memSize * SHIFT(arr->baseSize)) + sizeof(vec_t*));
        deallocator(arr);
        return NULL;
    }
    arr->baseArr = baseArr + sizeof(vec_t*);
    memcpy(baseArr, &arr, sizeof(vec_t*));
    arr->offset = 0;
    arr->memSize = memSize;
    return arr;
}

static void vec_resize(vec_t* arr, size_t newBaseSize) {
    void* newArr = allocator(arr->memSize * SHIFT(newBaseSize) + sizeof(vec_t*));
    if(newArr == NULL) {
        fprintf(stderr, "vec_resize: malloc failed, requested size: %zu\n", arr->memSize * SHIFT(newBaseSize) + sizeof(vec_t*));
        return;
    }
    memcpy(newArr + sizeof(vec_t*), vec_front(arr), arr->size * arr->memSize);
    memcpy(newArr, &arr, sizeof(vec_t*));
    deallocator(arr->baseArr - sizeof(vec_t*));
    arr->baseArr = newArr + sizeof(vec_t*);
    arr->baseSize = newBaseSize;
    arr->offset = 0;
}

static void vec_extend(vec_t* arr) {
    if(arr->size + arr->offset < SHIFT(arr->baseSize)) return;
    size_t newBaseSize = arr->baseSize + 1;
    vec_resize(arr, newBaseSize);
}

static void vec_reduce(vec_t* arr) {
    if(arr->baseSize == 0) return;
    if(arr->size * 2 > SHIFT(arr->baseSize)) return;
    size_t newBaseSize = arr->baseSize - 1;
    vec_resize(arr, newBaseSize);
}

void* vec_create(size_t memSize, size_t size) {
    if(memSize == 0) return NULL;
    vec_t* darr = vec_init(memSize, size);
    if(darr == NULL) return NULL;
    return darr->baseArr;
}

static void vec_pushBack(vec_t*__restrict__ arr, void*__restrict__ value) {
    vec_extend(arr);
    memcpy(vec_back(arr), value, arr->memSize);
    arr->size++;
}

void _vec_priv_pushBack(void** arrPtr, void* value) {
    if(value == NULL || arrPtr == NULL || *arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*arrPtr);
    vec_pushBack(tmp, value);
    *arrPtr = vec_front(tmp);
}

static void vec_pushFront(vec_t* arr, void*__restrict__ value) {
    if(arr->offset > 0) {
        arr->offset--;
        arr->size++;
        memcpy(vec_front(arr), value, arr->memSize);
        memcpy(vec_front(arr) - sizeof(vec_t*), &arr, sizeof(vec_t*));
    } else {
        // create room if needed
        vec_extend(arr);
        // shift everything to back of the array
        arr->offset = SHIFT(arr->baseSize) - arr->size;
        // if no offset (should not be possible) do nothing,
        // this is to avoid infinite recursive calls
        if(arr->offset == 0) return;
        // need memmove here because everything is moved over itself
        memmove(vec_front(arr), arr->baseArr, arr->size * arr->memSize);
        // retry to push the value
        vec_pushFront(arr,value);
    }
}

void _vec_priv_pushFront(void** arrPtr, void* value) {
    if(value == NULL || arrPtr == NULL || *arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*arrPtr);
    vec_pushFront(tmp, value);
    *arrPtr = vec_front(tmp);
}

size_t vec_size(const void* arr) {
    if(arr == NULL) return 0;
    const vec_t* tmp = vec_getInfo(arr);
    return tmp->size;
}

void vec_free(void* arr) {
    if(arr == NULL) return;
    vec_t* arrInfo = *(vec_t**)(arr - sizeof(vec_t*));
    deallocator(arrInfo->baseArr - sizeof(vec_t*));
    deallocator(arrInfo);
}


static void vec_popBack(vec_t*__restrict__ arr, void*__restrict__ buff) {
    if(arr->size == 0) return;
    if(buff != NULL) memcpy(buff, vec_indexFromBack(arr, 1), arr->memSize);
    arr->size--;
    vec_reduce(arr);
}

void _vec_priv_popBack(void** arrPtr, void* buff) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*arrPtr);
    if(tmp->size == 0) return;
    vec_popBack(tmp, buff);
    *arrPtr = vec_front(tmp);
}

static void vec_popFront(vec_t* arr, void*__restrict__ buff) {
    if(arr->size == 0) return;
    if(buff != NULL) memcpy(buff, arr->baseArr + arr->offset * arr->memSize, arr->memSize);
    arr->size--;
    arr->offset++;
    memcpy(vec_front(arr) - sizeof(vec_t*), &arr, sizeof(vec_t*));
    vec_reduce(arr);
}

void _vec_priv_popFront(void** arrPtr, void* buff) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*arrPtr);
    if(tmp->size == 0) return;
    vec_popFront(tmp, buff);
    *arrPtr = vec_front(tmp);
}

void vec_qsort(void* arr, int (*compar_fn) (const void *, const void *)) {
    if(arr == NULL) return;
    vec_t* tmp = vec_getInfo(arr);
    qsort(arr, tmp->size, tmp->memSize, compar_fn);
}

void* _vec_priv_slice(void* arr, size_t start, size_t end) {
    if(arr == NULL) return NULL;
    vec_t* tmp = vec_getInfo(arr);
    if(start >= tmp->size) return vec_create(tmp->memSize, 0);
    if(end > tmp->size) end = tmp->size;
    void* newArr = vec_create(tmp->memSize, end - start);
    memcpy(newArr, vec_index(tmp, start), (end - start) * tmp->memSize);
    return newArr;
}

static void vec_insert(vec_t* arr, size_t index, void* value) {
    if(index > arr->size) return;
    vec_extend(arr);
    // need memmove here because everything is moved over itself by one element
    memmove(vec_index(arr, index + 1), vec_index(arr, index), (arr->size - index) * arr->memSize);
    memcpy(vec_index(arr, index), value, arr->memSize);
    arr->size++;
}

void _vec_priv_insert(void** arrPtr, size_t index, void* value) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*arrPtr);
    if(index >= tmp->size) {
        vec_pushBack(tmp, value);
        return;
    }
    if(index == 0) {
        vec_pushFront(tmp, value);
        return;
    }
    vec_insert(tmp, index, value);
    *arrPtr = vec_front(tmp);
}

void _vec_priv_remove(void** arrPtr, size_t index, void*__restrict__ buff) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*arrPtr);
    if(index >= tmp->size) return;
    if(buff != NULL) memcpy(buff, vec_index(tmp, index), tmp->memSize);
    // need memmove here because everything is moved over itself by one element
    memmove(vec_index(tmp, index), vec_index(tmp, index + 1), (tmp->size - index - 1) * tmp->memSize);
    tmp->size--;
    vec_reduce(tmp);
    *arrPtr = vec_front(tmp);
}

void vec_swap(void* arr, size_t index1, size_t index2) {
    if(arr == NULL || index1 == index2) return;
    vec_t* tmp = vec_getInfo(arr);
    if(index1 >= tmp->size || index2 >= tmp->size) return;
    // no need for memmove here as index1 and index2 are always different
    // so dest and src can't overlap

    // the two possibility here avoid the need of a temp buffer
    // by writing the values directly to unused memory of the array
    if(tmp->offset != 0) {
        memcpy(tmp->baseArr, vec_index(tmp, index1), tmp->memSize);
        memcpy(vec_index(tmp, index1), vec_index(tmp, index2), tmp->memSize);
        memcpy(vec_index(tmp, index2), tmp->baseArr, tmp->memSize);
        return;
    } else if(SHIFT(tmp->baseSize) > tmp->size) {
        memcpy(vec_back(tmp), vec_index(tmp, index1), tmp->memSize);
        memcpy(vec_index(tmp, index1), vec_index(tmp, index2), tmp->memSize);
        memcpy(vec_index(tmp, index2), vec_back(tmp), tmp->memSize);
        return;
    }

    void* buff = allocator(tmp->memSize);
    if(buff == NULL) {
        fprintf(stderr, "vec_swap: buffer malloc failed, requested size: %zu\n", tmp->memSize);
        return;
    }
    memcpy(buff, vec_index(tmp, index1), tmp->memSize);
    memcpy(vec_index(tmp, index1), vec_index(tmp, index2), tmp->memSize);
    memcpy(vec_index(tmp, index2), buff, tmp->memSize);
    deallocator(buff);
}

void _vec_priv_clear(void** arrPtr) {
    if(arrPtr == NULL || *arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*arrPtr);
    tmp->size = 0;
    vec_resize(tmp, 0);
    *arrPtr = vec_front(tmp);
}


void _vec_debug_print(void* arr, FILE* stream) {
    if(arr == NULL) return;
    vec_t* tmp = vec_getInfo(arr);
    fprintf(stream, "size: %lu, offset: %lu, memSize: %lu, baseSize: %lu\n", tmp->size, tmp->offset, tmp->memSize, tmp->baseSize);
    fprintf(stream, "effective memsize: %lu\n", SHIFT(tmp->baseSize) * tmp->memSize + sizeof(vec_t) + sizeof(vec_t*));
}

static void vec_preReserve(vec_t* arr, size_t newSize) {
    if(newSize <= arr->baseSize) return;
    size_t newBaseSize = LOG2(newSize ? newSize : 1) + 1;
    if(newBaseSize > arr->baseSize) {
        vec_resize(arr, newBaseSize);
    }
}


void vec_allocate(void* arrPtr, size_t newSize, int resize) {
    if(arrPtr == NULL || *(void**)arrPtr == NULL) return;
    vec_t* tmp = vec_getInfo(*(void**)arrPtr);
    vec_preReserve(tmp, newSize);
    if(resize) tmp->size = newSize;
    *(void**)arrPtr = vec_front(tmp);
}

void vec_reverse(void* arr) {
    if(arr == NULL) return;
    vec_t* tmp = vec_getInfo(arr);
    // no need for memmove here as i is strictly inferior to j,
    // so i always different from j
    // so dest and src can't overlap

    // the two possibility here avoid the need of a temp buffer
    // by writing the values directly to the array
    if(tmp->offset != 0) {
        for(size_t i = 0, j = tmp->size - 1; i < j; i++, j--) {
            memcpy(tmp->baseArr, vec_index(tmp, i), tmp->memSize);
            memcpy(vec_index(tmp, i), vec_index(tmp, j), tmp->memSize);
            memcpy(vec_index(tmp, j), tmp->baseArr, tmp->memSize);
        }
        return;
    } else if(SHIFT(tmp->baseSize) > tmp->size) {
        for(size_t i = 0, j = tmp->size - 1; i < j; i++, j--) {
            memcpy(vec_back(tmp), vec_index(tmp, i), tmp->memSize);
            memcpy(vec_index(tmp, i), vec_index(tmp, j), tmp->memSize);
            memcpy(vec_index(tmp, j), vec_back(tmp), tmp->memSize);
        }
        return;
    }
    void* buff = allocator(tmp->memSize);
    if(buff == NULL) {
        fprintf(stderr, "vec_reverse: buffer malloc failed, requested size: %zu\n", tmp->memSize);
        return;
    }
    for(size_t i = 0, j = tmp->size - 1; i < j; i++, j--) {
        memcpy(buff, vec_index(tmp, i), tmp->memSize);
        memcpy(vec_index(tmp, i), vec_index(tmp, j), tmp->memSize);
        memcpy(vec_index(tmp, j), buff, tmp->memSize);
    }
    deallocator(buff);
}

void vec_set_allocator(void* (*_allocator)(size_t)) {
    allocator = _allocator;
}

void vec_set_deallocator(void (*_deallocator)(void*)) {
    deallocator = _deallocator;
}


