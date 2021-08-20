#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 10

void cqsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));

void shuffle(void* arr, size_t nmemb, size_t size) {
    char buff[size];
    int randn;
    for(size_t i = 0; i < nmemb; i++) {
        randn = rand() % nmemb;
        if(randn == i) continue;
        memcpy(buff, arr + i * size, size);
        memcpy(arr + i * size, arr + randn * size, size);
        memcpy(arr + randn * size, buff, size);
    }
}

void dispArr(int* arr, size_t size) {
    printf("arr : ");
    for(size_t i = 0; i < size; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n\n");
}

int comp_int(const void* a, const void* b) {
    const int *ia = (const int *)a;
    const int *ib = (const int *)b;
    return (*ia > *ib) - (*ia < *ib);
}

int main(int argc, char const *argv[])
{
    srand(time(NULL));
    int arr[SIZE] = {1,3,6,6,6,9,18,18,23,56}, arr2[SIZE];
    shuffle(arr, SIZE, sizeof(int));
    memcpy(arr2, arr, SIZE * sizeof(int));
    dispArr(arr, SIZE);
    qsort(arr, SIZE, sizeof(int), comp_int);
    printf("std qsort : \n");
    dispArr(arr, SIZE);
    cqsort(arr2, SIZE, sizeof(int), comp_int);
    printf("custom qsort : \n");
    dispArr(arr2, SIZE);
    
    for(size_t i = 0; i < SIZE; i++) {
        if(arr[i] != arr2[i]) {
            printf("different result.\n");
            goto exit;// oé je sais les goto c degueux mais flm
        }
    }
    printf("same result.\n");
    exit: 

    return 0;
}

void cqsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*)) {
}