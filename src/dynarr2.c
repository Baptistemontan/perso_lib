#include "../headers/dynarr2.h"

#define SHIFT(n) (1 << n)

#define SORT() qsort(darrays, narrays, sizeof(dynarr_arr*), dynarr_private_comp_sort)

typedef struct {
    void* arr;
    void* baseArr;
    size_t baseSize;
    size_t size;
    size_t offset;
    size_t byteSize;
} dynarr_arr;

static size_t narrays = 0;
static dynarr_arr** darrays = NULL;

static void dynarr_private_extend(dynarr_arr* arr);
static void dynarr_private_reduce(dynarr_arr* arr);
static void* dynarr_private_pushBack(dynarr_arr* arr, void* value);

static int dynarr_private_comp_sort(const void* a, const void* b) {
    const dynarr_arr* da = *(dynarr_arr**)a;
    const dynarr_arr* db = *(dynarr_arr**)b;
    return (da->arr > db->arr) - (da->arr < db->arr);
}

static size_t dynarr_private_findArr(void* arr) {
    if(arr == NULL) return 0;
    size_t a = 0, b = narrays, mid;
    while(a < b) {
        mid = (a + b) >> 1;
        if(darrays[mid]->arr == arr) return mid + 1;
        if(darrays[mid]->arr > arr) b = mid;
        else a = mid;
    }
    return 0;
}

static dynarr_arr* dynarr_private_init(size_t byteSize) {
    dynarr_arr* arr = malloc(sizeof(dynarr_arr));
    arr->baseSize = 0;
    arr->size = arr->offset = 0;
    arr->byteSize = byteSize;
    arr->baseArr = arr->arr = malloc(byteSize);
    return arr;
}

static void dynarr_private_free(dynarr_arr* arr) {
    free(arr->baseArr);
    free(arr);
}

void* dynarr_create(size_t byteSize, size_t size, void* value) {
    if(byteSize == 0) return NULL;
    dynarr_arr* darr = dynarr_private_init(byteSize);
    dynarr_arr** tmp = malloc(sizeof(dynarr_arr*) * (narrays + 1));
    memcpy(tmp, darrays, sizeof(dynarr_arr*) * narrays);
    free(darrays);
    darrays = tmp;
    darrays[narrays] = darr;
    narrays++;
    for(size_t i = 0; i < size; i++) {
        dynarr_private_pushBack(darr, value);
    }
    return darr->arr;
}

static void dynarr_private_extend(dynarr_arr* arr) {
    if(arr->size + arr->offset < SHIFT(arr->baseSize)) return;
    size_t newBaseSize = arr->baseSize + 1;
    void* newArr = malloc(arr->byteSize * SHIFT(newBaseSize));
    memcpy(newArr,arr->arr,arr->size * arr->byteSize);
    free(arr->baseArr);
    arr->baseArr = newArr;
    arr->arr = newArr;
    arr->baseSize = newBaseSize;
    arr->offset = 0;
    SORT();
}

static void dynarr_private_reduce(dynarr_arr* arr) {
    if(arr->size * 2 > SHIFT(arr->baseSize)) return;
    if(arr->baseSize == 0) return;
    size_t newBaseSize = arr->baseSize - 1;
    void* newArr = malloc(arr->byteSize * SHIFT(newBaseSize));
    memcpy(newArr,arr->arr,arr->size * arr->byteSize);
    free(arr->baseArr);
    arr->baseArr = newArr;
    arr->arr = newArr;
    arr->baseSize = newBaseSize;
    arr->offset = 0;
    SORT();
}

static void* dynarr_private_pushBack(dynarr_arr* arr, void* value) {
    dynarr_private_extend(arr);
    memcpy(arr->arr + (arr->size * arr->byteSize), value, arr->byteSize);
    arr->size++;
    return arr->arr;
}

void* dynarr_pushBack(void* arrAdd, void* value) {
    if(value == NULL || arrAdd == NULL) return value;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return value;
    *(void**)arrAdd = dynarr_private_pushBack(darrays[i - 1], value);
    return value;
}

static void* dynarr_private_pushFront(dynarr_arr* arr, void* value) {
    if(arr->offset > 0) {
        arr->offset--;
        arr->arr = arr->baseArr + arr->offset;
        arr->size++;
        SORT();
        memcpy(arr->arr,value, arr->byteSize);
    } else {
        dynarr_private_extend(arr);
        arr->offset = SHIFT(arr->baseSize) - arr->size;
        memmove(arr->baseArr + arr->offset,arr->baseArr,arr->size * arr->byteSize);
        dynarr_private_pushFront(arr,value);
    }
    return arr->arr;
}

void* dynarr_pushFront(void* arrAdd, void* value) {
    if(value == NULL || arrAdd == NULL) return value;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return value;
    *(void**)arrAdd = dynarr_private_pushFront(darrays[i - 1], value);
    return value;
}

size_t dynarr_size(void* arr) {
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return 0;
    return darrays[i - 1]->size;
}

void dynarr_free(void* arr) {
    if(arr == NULL) return;
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return;
    dynarr_private_free(darrays[i - 1]);
    if(narrays == 1) {
        free(darrays);
        darrays = NULL;
        narrays = 0;
        return;
    }
    dynarr_arr** tmp = malloc(sizeof(dynarr_arr*) * (narrays - 1));
    memcpy(tmp, darrays, i * sizeof(dynarr_arr*));
    memcpy(tmp, darrays + i + 1, (narrays - i) * sizeof(dynarr_arr*));
    free(darrays);
    darrays = tmp;
    narrays--;
}



