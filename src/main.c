#include "../headers/all.h"

DYNARR_DEF_ALL(int, int);

int main(int argc, char const *argv[])
{
    int* test = dynarr_create_int(0);
    for(int i = 0; i < 5; i++) {
        dynarr_pushBack_int(&test, i);
    }
    dynarr_insert_int(&test, 2, 10);
    for(int i = 0; i < dynarr_size(test); i++) {
        printf("i = %d, v = %d\n", i, test[i]);
    }
    while(dynarr_size(test)) {
        printf("poped %d\n", dynarr_popFront_int(&test));
    }

    dynarr_free(test);


    return EXIT_SUCCESS;
}
