#include "../headers/dynarr.h"

#define SHIFT(n) (1 << n) // fast 2^n
#define SORT() qsort(darray_info->arr, darray_info->size, sizeof(dynarr_arr*), dynarr_private_comp_sort)
#define SAVEBUFF(src, byteSize) free(tmpBuff); tmpBuff = malloc(byteSize); memcpy(tmpBuff,src,byteSize);

typedef struct {
    // technically arr is useless, with bytesize, offset and baseArr we can calculate it
    // but for code simplification and speed, we can sacrifice 8 bytes per array
    void* arr;
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
// info on the arr of arr info
static dynarr_arr* darray_info = NULL;
// arr of info arr
static dynarr_arr** darray = NULL;


static void dynarr_private_extend(dynarr_arr* arr);
static void dynarr_private_reduce(dynarr_arr* arr);
static void dynarr_private_pushBack(dynarr_arr* arr, void* value);

static int dynarr_private_comp_sort(const void* a, const void* b) {
    const dynarr_arr* da = *(dynarr_arr**)a;
    const dynarr_arr* db = *(dynarr_arr**)b;
    return (da->baseArr > db->baseArr) - (da->baseArr < db->baseArr);
}

static size_t dynarr_private_findArr(void* arr) {
    if(arr == NULL) return 0;
    size_t a = 0, b = darray_info->size - 1, mid;
    while(a <= b && b < darray_info->size) {
        mid = (a + b) >> 1;
        if(darray[mid]->arr == arr) return mid + 1;
        if(darray[mid]->arr > arr) {
            b = mid - 1;
        } else {
            a = mid + 1;
        }
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

void* dynarr_create(size_t byteSize, size_t size, void* value) {
    if(byteSize == 0) return NULL;
    if(darray_info == NULL) {
        darray_info = dynarr_private_init(sizeof(dynarr_arr*));
        darray = darray_info->arr;
    }
    dynarr_arr* darr = dynarr_private_init(byteSize);
    dynarr_private_pushBack(darray_info, &darr);
    darray = darray_info->arr;
    for(size_t i = 0; i < size; i++) {
        dynarr_private_pushBack(darr, value);
    }
    SORT();
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
    arr->size++;
}

void* dynarr_pushBack(void* arrAdd, void* value) {
    if(value == NULL || arrAdd == NULL) return NULL;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return NULL;
    dynarr_arr* tmp = darray[i - 1];
    dynarr_private_pushBack(tmp, value);
    *(void**)arrAdd = tmp->arr;
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
    if(value == NULL || arrAdd == NULL) return NULL;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return NULL;
    dynarr_arr* tmp = darray[i - 1];
    dynarr_private_pushFront(tmp, value);
    *(void**)arrAdd = tmp->arr;
    return value;
}

size_t dynarr_getSize(void* arr) {
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return 0;
    return darray[i - 1]->size;
}

void dynarr_free(void* arr) {
    if(arr == NULL) return;
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return;
    dynarr_arr* tmp = darray[i - 1];
    darray[i - 1] = darray[darray_info->size - 1];
    darray[darray_info->size - 1] = tmp;
    dynarr_private_popBack(darray_info);
    darray = darray_info->arr;
    tmp = *(dynarr_arr**)tmpBuff;
    free(tmp->baseArr);
    free(tmp);
    SORT();
    if(darray_info->size == 0) {
        free(tmpBuff);
        tmpBuff = NULL;
        free(darray_info->baseArr);
        free(darray_info);
        darray_info = NULL;
        darray = NULL;
        // printf("no more bitch\n");
    }
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
    dynarr_arr* tmp = darray[i - 1];
    if(tmp->size == 0) return NULL;
    dynarr_private_popBack(tmp);
    *(void**)arrAdd = tmp->arr;
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
    dynarr_arr* tmp = darray[i - 1];
    if(tmp->size == 0) return NULL;
    dynarr_private_popFront(tmp);
    *(void**)arrAdd = tmp->arr;
    return tmpBuff;
}

void* dynarr_lastDelElem() {
    return tmpBuff;
}

void dynarr_qsort(void* arr, __compar_fn_t compar) {
    if(arr == NULL) return;
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return;
    qsort(arr,darray[i - 1]->size, darray[i - 1]->byteSize, compar);
}