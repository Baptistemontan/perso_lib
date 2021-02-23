#include "../headers/dynarr.h"

#define SHIFT(n) (1 << n) // fast 2^n

#define SORT() qsort(darrays, narrays, sizeof(dynarr_arr*), dynarr_private_comp_sort)
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
static dynarr_arr** darrays = NULL;
// temp buff for pop functions
static void* tmpBuff = NULL;


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
    // for(uint i = 0; i < narrays; i++) {
    //     printf("%p\n",darrays[i]->arr);
    // }
    // printf("r = %p\n", arr);
    size_t a = 0, b = narrays - 1, mid;
    while(a <= b && b < narrays) {
        mid = (a + b) >> 1;
        if(darrays[mid]->arr == arr) return mid + 1;
        if(darrays[mid]->arr > arr) {
            b = mid - 1;
        } else {
            a = mid + 1;
        }
    }
    printf("erf\n");
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

void* dynarr_create(size_t byteSize, size_t size, void* valuePtr) {
    if(byteSize == 0) return NULL;
    dynarr_arr** tmp = malloc(sizeof(dynarr_arr*) * (narrays + 1));
    for(size_t i = 0; i < narrays; i++) {
        tmp[i] = darrays[i];
    }
    free(darrays);
    darrays = tmp;
    dynarr_arr* darr = dynarr_private_init(byteSize);
    // size_t i = dynarr_private_findArr(darr->arr);
    // if(i != 0) printf("hoho\n");
    darrays[narrays] = darr;
    narrays++;
    // printf("create\n");
    // dynarr_private_findArr(darr->arr);
    for(size_t i = 0; i < size; i++) {
        dynarr_private_pushBack(darr, valuePtr);
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

void* dynarr_pushBack(void* arrAdd, void* valuePtr) {
    if(valuePtr == NULL || arrAdd == NULL) return valuePtr;
    size_t i = dynarr_private_findArr(*(void**)arrAdd);
    if(i == 0) return valuePtr;
    dynarr_arr* tmp = darrays[i - 1];
    dynarr_private_pushBack(tmp, valuePtr);
    *(void**)arrAdd = tmp->arr;
    return valuePtr;
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
    dynarr_arr* tmp = darrays[i - 1];
    dynarr_private_pushFront(tmp, value);
    *(void**)arrAdd = tmp->arr;
    return value;
}

size_t dynarr_size(void* arr) {
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return 0;
    return darrays[i - 1]->size;
}

void dynarr_free(void* arr) {
    if(arr == NULL) return;
    // printf("%lu\n",narrays);
    size_t i = dynarr_private_findArr(arr);
    if(i == 0) return;
    free(darrays[i - 1]->baseArr);
    free(darrays[i - 1]);
    if(narrays == 1) {
        free(darrays);
        darrays = NULL;
        free(tmpBuff);
        tmpBuff = NULL;
        narrays = 0;
        printf("all gone\n");
        return;
    }
    dynarr_arr** tmp = malloc(sizeof(dynarr_arr*) * (narrays - 1));
    // for(size_t j = 0; j < i - 1; j++) {
    //     tmp[j] = darrays[j];
    // }
    // for(size_t j = i; j < narrays; j++) {
    //     tmp[j - 1] = darrays[j];
    // }
    memcpy(tmp, darrays, (i - 1) * sizeof(dynarr_arr*));
    memcpy(tmp + i - 1, darrays + i, (narrays - i) * sizeof(dynarr_arr*));
    free(darrays);
    darrays = tmp;
    narrays--;
    // dynarr_private_findArr(arr);
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
    dynarr_arr* tmp = darrays[i - 1];
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
    dynarr_arr* tmp = darrays[i - 1];
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
    qsort(arr,darrays[i - 1]->size, darrays[i - 1]->byteSize, compar);
}








