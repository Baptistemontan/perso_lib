#include "../headers/all.h"

#define SIZE 5

int main(int argc, char const *argv[])
{
    uint* test = DYNARR_INIT(sizeof(uint));
    uint i = 34;
    for(uint j = 0; j < SIZE; j++) {
        i += j;
        printf("%u\n",*(uint*)dynarr_pushBack(&test, &i));
    }
    printf("\n");
    while (dynarr_size(test))
    {
        printf("%u\n",*(uint*)dynarr_popFront(&test));
    }
    
    dynarr_free(test);
    return EXIT_SUCCESS;
}