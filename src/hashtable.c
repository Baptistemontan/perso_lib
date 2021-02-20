#include "../headers/all.h"

static size_t ht_private_defaultHash(size_t size, char* key) {
    size_t len = strlen(key), sum1 = -1 , sum2 = 0, sum3 = sum1 / (2 * len);
    sum1 -= len * (size + 1);
    for(size_t i = 0; i < len; i++) {
        sum1 += sum2 + i + key[i];
        sum3 += sum1 - sum2 + len;
        sum2 += (sum1 + 1) * key[len - i - 1] - i * len + sum3 * (1 - i);
        sum1 ^= sum2;
        sum2 ^= sum3;
    }
    return (sum1 + sum2) % size;
}

static struct ht_pair* ht_private_createPair(char* key, void* value) {
    struct ht_pair* pair = malloc(sizeof(struct ht_pair));
    size_t keysize = strlen(key);
    pair->key = malloc(sizeof(char) * (keysize + 1));
    strcpy(pair->key,key);
    pair->value = value;
    return pair;
}

static void* ht_private_freePair(struct ht_pair** pair) {
    if(*pair == NULL) return NULL;
    struct ht_pair* tmp = *pair;
    *pair = tmp->next;
    void* value = tmp->value;
    free(tmp->key);
    free(tmp);
    return value;
}

ht_htable* ht_createTable(size_t size, ht_hash_fn hash_fn) {
    ht_htable* table = malloc(sizeof(ht_htable));
    table->table = malloc(sizeof(struct ht_pair*) * size);
    table->size = size;
    table->hash_fn = hash_fn != NULL ? hash_fn : ht_private_defaultHash;
    for(size_t i = 0; i < size; i++) {
        *(table->table + i) = NULL;
    }
    return table;
}

void ht_freeTable(ht_htable* table) {
    for(size_t i = 0; i < table->size; i++) {
        while(table->table[i] != NULL) {
            ht_private_freePair(table->table + i);
        }
    }
    free(table->table);
    free(table);
}

void* ht_setPair(ht_htable* table, char* key, void* value) {
    size_t slot = table->hash_fn(table->size,key);
    struct ht_pair* pair = table->table[slot];
    while (pair != NULL)
    {
        if(strcasecmp(pair->key,key) == 0) {
            void* tmp = pair->value;
            pair->value = value;
            return tmp;
        }
        pair = pair->next;
    }
    pair = ht_private_createPair(key,value);
    pair->next = table->table[slot];
    table->table[slot] = pair;
    return value;
}

void* ht_getPair(ht_htable* table, char* key) {
    size_t slot = table->hash_fn(table->size,key);
    struct ht_pair* pair = table->table[slot];
    while(pair != NULL) {
        if(strcasecmp(pair->key,key) == 0) {
            return pair->value;
        }
        pair = pair->next;
    }
    return NULL;
}

void* ht_removePair(ht_htable* table, char* key) {
    size_t slot = table->hash_fn(table->size,key);
    struct ht_pair** pair = table->table + slot;
    while(*pair != NULL) {
        if(strcasecmp((*pair)->key,key) == 0) {
            return ht_private_freePair(pair);
        }
        pair = &((*pair)->next);
    }
    return NULL;
}

void ht_map(ht_htable* src, ht_htable* dest) {
    struct ht_pair* pair = NULL;
    for(size_t i = 0; i < src->size; i++) {
        pair = src->table[i];
        while (pair != NULL) {
            ht_setPair(dest,pair->key,pair->value);
            pair = pair->next;
        }
    }
}

void ht_print(ht_htable* table) {
    struct ht_pair** arr = table->table;
    struct ht_pair* pair = NULL;
    printf("--------------------------------\n");
    for(size_t i = 0; i < table->size; i++) {
        pair = arr[i];
        while(pair != NULL) {
            printf("slot: %4lu\tkey: %-10s\n",i,pair->key);
            pair = pair->next;
        }
    }
    printf("--------------------------------\n");
}

void ht_save(ht_htable* table, char* path, ht_write_fn write_fn) {
    const size_t len = strlen(path);
    char newPath[len + 4];
    strcpy(newPath,path);
    strcpy(newPath + len, ".ht\0");
    FILE* savefile = fopen(newPath,"w+");
    if(savefile == NULL) {
        fprintf(stderr,"can't open %s.\n",newPath);
        exit(EXIT_FAILURE);
    }
    char buff[SIZEBUFF];
    struct ht_pair* pair = NULL;
    for(size_t i = 0; i < table->size; i++) {
        pair = table->table[i];
        while(pair != NULL) {
            write_fn(pair->value,pair->key,buff);
            fprintf(savefile,"%s:%s:",pair->key,buff);
            pair = pair->next;
        }
    }
    fclose(savefile);
}

void ht_load(ht_htable* table, char* path, ht_load_fn load_fn) {
    const size_t len = strlen(path);
    char newPath[len + 4];
    strcpy(newPath,path);
    strcpy(newPath + len, ".ht\0");
    FILE* savefile = fopen(newPath,"r");
    if(savefile == NULL) {
        fprintf(stderr,"can't open %s.\n",newPath);
        exit(EXIT_FAILURE);
    }
    char buff[SIZEBUFF];
    char key[MAXKEYSIZE];
    struct ht_pair* pair;
    while(fscanf(savefile,"%s:%s:",key,buff) != EOF) {
        ht_setPair(table,key,load_fn(key,buff));
    }
    fclose(savefile);
}


