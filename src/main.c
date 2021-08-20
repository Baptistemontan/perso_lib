#include "../headers/all.h"

VEC_DEF_ALL(int, int);

int main(int argc, char const *argv[])
{
    int* test = vec_create_int(0);
    for(int i = 0; i < 5; i++) {
        vec_pushBack_int(&test, i);
    }
    vec_insert_int(&test, 2, 10);
    for(int i = 0; i < vec_size(test); i++) {
        printf("i = %d, v = %d\n", i, test[i]);
    }
    while(vec_size(test)) {
        printf("poped %d\n", vec_popFront_int(&test));
    }

    vec_free(test);


    return EXIT_SUCCESS;
}
