#include "../headers/dynarr.h"

#define SHIFT(n) (1 << n) // fast 2^n

#define SORT() qsort(darrays, narrays, sizeof(dynarr_arr), dynarr_private_comp_sort)
#define SAVEBUFF(src, byteSize) free(tmpBuff); tmpBuff = malloc(byteSize); memcpy(tmpBuff,src,byteSize);

typedef struct {
    // technically arr is useless, with bytesize, offset and baseArr we can calculate it
    // but for code simplification, we can sacrifice 8 bytes per array
    void* arr;
    void* baseArr; // adress of the allocated array
    size_t baseSize; // log2 of the allocated size for the array
    size_t size; // number of elem in arr
    size_t offset; // discarded element beetween baseAr and arr
    size_t byteSize; // size of 1 element
} dynarr_arr;

// number of arrays
static size_t narrays = 0;
// array of the arrays infos
static dynarr_arr* darrays = NULL;
// temp buff for pop functions
static void* tmpBuff = NULL;


static void dynarr_private_extend(dynarr_arr* arr);
static void dynarr_private_reduce(dynarr_arr* arr);
static void dynarr_private_pushBack(dynarr_arr* arr, void* value);

static int dynarr_private_comp_sort(const void* a, const void* b) {
    const dynarr_arr* da = a;
    const dynarr_arr* db = b;
    return (da->baseArr > db->baseArr) - (da->baseArr < db->baseArr);
}

static size_t dynarr_private_findArr(void* arr) {
    if(arr == NULL) return 0;
    size_t a = 0, b = narrays, mid;
    while(a < b) {
        mid = (a + b) >> 1;
        if(darrays[mid].arr == arr) return mid + 1;
        if(darrays[mid].arr > arr) b = mid;
        else a = mid;
    }
    return 0;
}

static void dynarr_private_init(size_t byteSize, dynarr_arr* arr) {
    arr->baseSize = 0;
    arr->size = arr->offset = 0;
    arr->byteSize = byteSize;
    arr->baseArr = arr->arr = malloc(byteSize);
}

void* dynarr_create(size_t byteSize, size_t size, void* value) {
    if(byteSize == 0) return NULL;
    dynarr_arr* tmp = malloc(sizeof(dynarr_arr) * (narrays + 1));
    memcpy(tmp, darrays, sizeof(dynarr_arr) * narrays);
    free(darrays);
    darrays = tmp;
    dynarr_arr* darr = darrays + narrays;
    dynarr_private_init(byteSize, darr);
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

static void dynarr_private_pushBack(dynarr_arr* arr, void* value) {
    dynarr_private_extend(arr);
    memcpy(arr->arr + (arr->size * arr->byteSize), value, arr->byteSize);
    arr->size++;;
}

void* dynarr_pushBack(void* arrAdd, void* value) {
    if(value == NULL || arrAdd == NULL) return value;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return value;
    dynarr_private_pushBack(darrays + i - 1, value);
    *(void**)arrAdd = darrays[i - 1].arr;
    return value;
}

static void dynarr_private_pushFront(dynarr_arr* arr, void* value) {
    if(arr->offset > 0) {
        arr->offset--;
        arr->arr = arr->baseArr + arr->offset;
        arr->size++;
        memcpy(arr->arr,value, arr->byteSize);
    } else {
        dynarr_private_extend(arr);
        arr->offset = SHIFT(arr->baseSize) - arr->size;
        memmove(arr->baseArr + arr->offset,arr->baseArr,arr->size * arr->byteSize);
        dynarr_private_pushFront(arr,value);
    }
}

void* dynarr_pushFront(void* arrAdd, void* value) {
    if(value == NULL || arrAdd == NULL) return value;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return value;
    dynarr_private_pushFront(darrays + i - 1, value);
    *(void**)arrAdd = darrays[i - 1].arr;
    return value;
}

size_t dynarr_size(void* arr) {
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return 0;
    return darrays[i - 1].size;
}

void dynarr_free(void* arr) {
    if(arr == NULL) return;
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return;
    free(darrays[i - 1].baseArr);
    if(narrays == 1) {
        free(darrays);
        darrays = NULL;
        free(tmpBuff);
        tmpBuff = NULL;
        narrays = 0;
        return;
    }
    dynarr_arr* tmp = malloc(sizeof(dynarr_arr) * (narrays - 1));
    memcpy(tmp, darrays, i * sizeof(dynarr_arr));
    memcpy(tmp, darrays + i + 1, (narrays - i) * sizeof(dynarr_arr));
    free(darrays);
    darrays = tmp;
    narrays--;
}

static void dynarr_private_popBack(dynarr_arr* arr) {
    if(arr->size == 0) return;
    SAVEBUFF(arr->arr + (arr->size - 1) * arr->byteSize, arr->byteSize);
    arr->size--;
    dynarr_private_reduce(arr);
}

void* dynarr_popBack(void* arrAdd) {
    if(arrAdd == NULL) return NULL;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return NULL;
    if(darrays[i - 1].size == 0) return NULL;
    dynarr_private_popBack(darrays + i - 1);
    *(void**)arrAdd = darrays[i - 1].arr;
    return tmpBuff;
}

static void dynarr_private_popFront(dynarr_arr* arr) {
    if(arr->size == 0) return;
    SAVEBUFF(arr->arr, arr->byteSize);
    arr->size--;
    arr->offset++;
    arr->arr = arr->baseArr + (arr->offset * arr->byteSize);
    dynarr_private_reduce(arr);
}

void* dynarr_popFront(void* arrAdd) {
    if(arrAdd == NULL) return NULL;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return NULL;
    if(darrays[i - 1].size == 0) return NULL;
    dynarr_private_popFront(darrays + i - 1);
    *(void**)arrAdd = darrays[i - 1].arr;
    return tmpBuff;
}

void* dynarr_lastDelElem() {
    return tmpBuff;
}






