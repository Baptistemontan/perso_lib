#include "../headers/all.h"

#define SIZE 9
#define SQRTSIZE 3

void printMat(uint (*mat)[SIZE]) {
    for(uint i = 0; i < SIZE; i++) {
        for(uint j = 0; j < SIZE; j++) {
            printf("%u ",mat[i][j]);
        }
        printf("\n");
    }
}


void remp(FILE* f, uint (*mat)[SIZE]) {
    for(uint i = 0; i < SIZE; i++) {
        for(uint j = 0; j < SIZE; j++) {
            fscanf(f,"%u ",mat[i] + j);
        }
        fscanf(f,"\n");
    }
}

int test(uint (*mat)[SIZE], uint a, uint b, uint val) {
    uint aa, bb;
    uint oa = a / SQRTSIZE, ob = b / SQRTSIZE;
    for(uint i = 0; i < SIZE; i++) {
        if(i != a && mat[i][b] == val) return 0;
        if(i != b && mat[a][i] == val) return 0;
        bb = (i % SQRTSIZE);
        aa = ((i - bb) / SQRTSIZE);
        if((aa != a || bb != b) && mat[aa + oa * SQRTSIZE][bb + ob * SQRTSIZE] == val) return 0;
    }
    return 1;
}

int solve(uint (*mat)[SIZE], uint ind) {
    if(ind == SIZE * SIZE) return 1;
    uint b = ind % SIZE, a = (ind - b) / SIZE;
    if(mat[a][b]) return solve(mat,ind + 1);
    for(uint i = 1; i <= SIZE; i++) {
        if(test(mat,a,b,i) == 0) continue;
        mat[a][b] = i;
        if(solve(mat, ind + 1)) return 1;
    }
    mat[a][b] = 0;
    return 0;
} 


int main(int argc, char const *argv[])
{
    uint (*mat)[SIZE] = malloc(sizeof(uint) * SIZE * SIZE);

    FILE* f = fopen(ASSETS"/sudoku.txt","r");
    remp(f, mat);
    fclose(f);

    solve(mat, 0);

    
    printMat(mat);
    free(mat);

    return EXIT_SUCCESS;
}