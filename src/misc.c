#include "../headers/misc.h"

int misc_comp_int(const void* a, const void* b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    return (*ia > *ib) - (*ia < *ib);
}

int misc_comp_uint(const void* a, const void* b) {
    const uint *ia = (const uint *)a;
    const uint *ib = (const uint *)b;
    return (*ia > *ib) - (*ia < *ib);
}

int misc_comp_sizet(const void* a, const void* b) {
    const size_t *ia = (const size_t *)a;
    const size_t *ib = (const size_t *)b;
    return (*ia > *ib) - (*ia < *ib);
}

int misc_comp_float(const void* a, const void* b) {
    const float *ia = (const float *)a;
    const float *ib = (const float *)b;
    return (*ia > *ib) - (*ia < *ib);
}

int misc_comp_int_inv(const void* a, const void* b) {
    return -1 * misc_comp_int(b, a);
}

int misc_comp_uint_inv(const void* a, const void* b) {
    return -1 * misc_comp_uint(b, a);
}

int misc_comp_float_inv(const void* a, const void* b) {
    return -1 * misc_comp_float(b, a);
}

char* misc_createstr(const char* str, size_t sizeMax, void (*error_fn)(char*)) {
    size_t len = strlen(str);
    if(len > sizeMax && sizeMax > 0) len = sizeMax - 1;
    char* output = malloc(sizeof(char) * (len + 1));
    if(output == NULL) {
        if(error_fn != NULL) error_fn(MEMERR);
        return NULL;
    }
    strncpy(output, str, len);
    output[len] = '\0';
    return output;
}

int* misc_createint(int a, void (*error_fn)(char*)) {
    int* b = malloc(sizeof(int));
    if(b == NULL) {
        if(error_fn != NULL) error_fn(MEMERR);
        return NULL;
    }
    *b = a;
    return b;
}

uint* misc_createuint(uint a, void (*error_fn)(char*)) {
    uint* b = malloc(sizeof(uint));
    if(b == NULL) {
        if(error_fn != NULL) error_fn(MEMERR);
        return NULL;
    }
    *b = a;
    return b;
}

void* misc_arrAdd(void* dest, size_t nmemb, size_t bSize, void* src, void (*free_fn)(void*), void (*error_fn)(char*)) {
    void* arr = malloc(bSize * (nmemb + 1));
    if(arr == NULL) {
        if(error_fn != NULL) error_fn(MEMERR);
        return NULL;
    }
    memcpy(arr, dest, bSize * nmemb);
    memcpy(arr + (nmemb * bSize), src, bSize);
    if(free_fn != NULL) free_fn(dest);
    return arr;
}

void* misc_arrRm(void* dest, size_t nmemb, size_t bSize, size_t index, void (*free_fn)(void*), void (*error_fn)(char*)) {
    void* arr = malloc(bSize * (nmemb - 1));
    if(arr == NULL) {
        if(error_fn != NULL) error_fn(MEMERR);
        return NULL;
    }
    if(index > 0) memcpy(arr, dest, bSize * index);
    if(index + 1 < nmemb) memcpy(arr + bSize * index , dest + (bSize * (index + 1)), bSize * (nmemb - index - 1));
    if(free_fn != NULL) free_fn(dest);
    return arr;
}

void misc_printErr(char* str) {
    fprintf(stderr, "error: %s\n", str);
}