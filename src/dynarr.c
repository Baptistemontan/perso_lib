#include "../headers/dynarr.h"

dynarr_arr* dynarr_new() {
    dynarr_arr* array = malloc(sizeof(dynarr_arr));
    array->size = 0;
    array->offset = 0;
    array->baseSize = 1;
    array->baseArr = malloc(sizeof(void*));
    array->arr = array->baseArr;
    return array;
}

void dynarr_free(dynarr_arr* arr, void (*free_fn)(void*)) {
    if(arr == NULL) return;
    if(free_fn != NULL) {
        for(size_t i = 0; i < arr->size; i++) {
            free_fn(arr->arr[i]);
        }
    }
    free(arr->baseArr);
    free(arr);
}

static void dynarr_private_extend(dynarr_arr* arr) {
    if(arr->size + arr->offset < arr->baseSize) return;
    size_t newBaseSize = arr->baseSize << 1;
    void** newArr = malloc(sizeof(void*) * newBaseSize);
    memcpy(newArr,arr->arr,arr->size * sizeof(void*));
    free(arr->baseArr);
    arr->baseArr = newArr;
    arr->arr = newArr;
    arr->baseSize = newBaseSize;
    arr->offset = 0;
}

static void dynarr_private_reduce(dynarr_arr* arr) {
    if(arr->size * 2 > arr->baseSize) return;
    if(arr->baseSize == 1) return;
    size_t newBaseSize = arr->baseSize >> 1;
    void** newArr = malloc(sizeof(void*) * newBaseSize);
    memcpy(newArr,arr->arr,arr->size * sizeof(void*));
    free(arr->baseArr);
    arr->baseArr = newArr;
    arr->arr = newArr;
    arr->baseSize = newBaseSize;
    arr->offset = 0;
}

void* dynarr_push(dynarr_arr* arr, void* value) {
    if(arr == NULL) return NULL;
    dynarr_private_extend(arr);
    arr->arr[arr->size] = value;
    arr->size++;
    return value;
}

void* dynarr_pop(dynarr_arr* arr) {
    if(arr == NULL) return NULL;
    if(arr->size == 0) return NULL;
    void* x = arr->arr[arr->size - 1];
    arr->size--;
    dynarr_private_reduce(arr);
    return x;
}

void* dynarr_pushFront(dynarr_arr* arr, void* value) {
    if(arr == NULL) return value;
    if(arr->offset > 0) {
        arr->offset--;
        arr->arr = arr->baseArr + arr->offset;
        arr->size++;
        arr->arr[0] = value;
    } else {
        dynarr_private_extend(arr);
        arr->offset = arr->baseSize - arr->size;
        memmove(arr->baseArr + arr->offset,arr->baseArr,arr->size * sizeof(void*));
        dynarr_pushFront(arr,value);
    }
    return value;
}

void* dynarr_popFront(dynarr_arr* arr) {
    if(arr == NULL) return NULL;
    if(arr->size == 0) return NULL;
    void* x = arr->arr[0];
    arr->size--;
    arr->offset++;
    arr->arr = arr->baseArr + arr->offset;
    dynarr_private_reduce(arr);
    return x;
}

void* dynarr_get(dynarr_arr* arr, size_t index) {
    if(arr == NULL) return NULL;
    if(index >= arr->size) return NULL;
    return arr->arr[index];
}

void* dynarr_set(dynarr_arr* arr, size_t index, void* value) {
    if(arr == NULL) return NULL;
    if(index >= arr->size) return NULL;
    return arr->arr[index] = value;
}

void dynarr_qsort_interval(dynarr_arr* arr, size_t start, size_t nmemb, __compar_fn_t compare) {
    if(arr == NULL) return;
    if(start >= arr->size) return;
    if(nmemb == 0 || nmemb + start > arr->size) nmemb = arr->size - start;
    qsort(arr->arr + start, nmemb, sizeof(void*), compare);
}

void dynarr_forEach_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_forEach_fn todo_fn, void* args) {
    if(arr == NULL) return;
    if(start >= arr->size) return;
    if(nmemb == 0 || nmemb + start > arr->size) nmemb = arr->size - start;
    for(size_t i = start; i < start + nmemb; i++) {
        todo_fn(arr->arr[i], i, start, args);
    }
}

dynarr_arr* dynarr_map_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_map_fn map_fn) {
    if(arr == NULL) return NULL;
    if(start >= arr->size) return NULL;
    dynarr_arr* newArr = dynarr_new();
    void* res;
    if(nmemb == 0 || nmemb + start > arr->size) nmemb = arr->size - start;
    for(size_t i = start; i < start + nmemb; i++) {
        res = map_fn(arr->arr[i], i, start);
        if(res != DYNARR_MAP_NOT) dynarr_push(newArr, res);
    }
    return newArr;
}

void* dynarr_every_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_forEach_fn test_fn, void* args) {
    if(arr == NULL) return NULL;
    if(start >= arr->size) return NULL;
    if(nmemb == 0 || nmemb + start > arr->size) nmemb = arr->size - start;
    void* test;
    for(size_t i = start; i < start + nmemb; i++) {
        test = test_fn(arr->arr[i], i, start, args);
        if(test == NULL) return NULL;
    }
    return test;
}

void* dynarr_some_interval(dynarr_arr* arr, size_t start, size_t nmemb, dynarr_forEach_fn test_fn, void* args) {
    if(arr == NULL) return NULL;
    if(start >= arr->size) return NULL;
    if(nmemb == 0 || nmemb + start > arr->size) nmemb = arr->size - start;
    void* test;
    for(size_t i = start; i < start + nmemb; i++) {
        test = test_fn(arr->arr[i], i, start, args);
        if(test != NULL) return test;
    }
    return NULL;
}

void dynarr_fill_interval(dynarr_arr* arr, size_t start, size_t nmemb, void* value) {
    if(arr == NULL) return;
    if(start > arr->size) return;
    if(nmemb == 0) nmemb = arr->size - start;
    size_t second = 0;
    if(nmemb + start > arr->size) {
        second =  nmemb + start - arr->size;
        nmemb = arr->size - start;
    }
    for(size_t i = start; i < start + nmemb; i++) {
        arr->arr[i] = value;
    }
    for(size_t i = start + nmemb; i < start + nmemb + second; i++) {
        dynarr_push(arr, value);
    }
}

inline size_t dynarr_getSize(dynarr_arr* arr) {
    if(arr == NULL) return 0;
    return arr->size;
}

long dynarr_bsearch_interval(dynarr_arr* arr, size_t start, size_t nmemb, void* value, __compar_fn_t compare) {
    if(arr == NULL) return -1;
    if(start >= arr->size) return -1;
    if(nmemb == 0) nmemb = arr->size - start;
    size_t end = start + nmemb - 1;
    long mid;
    int comp;
    while(end >= start) {
        mid = (start + end) / 2;
        comp = compare(arr->arr[mid],value);
        if(comp == 0) {
            mid--;
            while(mid >= 0 && compare(arr->arr[mid],value) == 0) {
                mid--;
            }
            return mid + 1;
        } else if(comp < 0) {
            start = mid + 1;
        } else {
            end = mid - 1;
        }
    }
    return -1;
}