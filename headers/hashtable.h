#ifndef HEAD_HT_HASHTABLE
#define HEAD_HT_HASHTABLE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// macro for creating a new hashtable with default hash fonction
#define HASHTABLE_INIT(size) ht_createTable(size, NULL)

#define SIZEBUFF 500
#define MAXKEYSIZE 100

// hash fonction, should return an integer in [0,size[ based on the given key
typedef size_t (*ht_hash_fn)(size_t size, char* key);

// buff is size SIZEBUFF
typedef void (*ht_write_fn)(void* value, char* key, char* buff);

// buff is size SIZEBUFF
typedef void* (*ht_load_fn)(char* key, char* buff);

// stock info on a given pair, you should not have to use that
struct ht_pair {
    char* key;
    void* value;
    struct ht_pair *next;
};

// stock the hashtable, should be initialized with ht_createTable()
typedef struct {
    struct ht_pair** table;
    size_t size;
    ht_hash_fn hash_fn;
} ht_htable;

// initialize a ht_htable, should be free with ht_freeTable()
// use a default hash fonction if hash_fn is NULL
ht_htable* ht_createTable(size_t size, ht_hash_fn hash_fn);

// free the entire table
void ht_freeTable(ht_htable* table);

// if no pair with this key exist, create one and return the given value,
// if one already exist, return the old value and overwrite it with the new one
void* ht_setPair(ht_htable* table, char* key, void* value);

// return the value of the pair with the given key,
// return NULL if no pair with this key exist
void* ht_getPair(ht_htable* table, char* key);

// remove the pair with the given key,
// return the value of the pair if the pair exist,
// return NULL if the pair does'nt exist
void* ht_removePair(ht_htable* table, char* key);

// copy all the pair in src to dest at the right place
// src and dest can be with different size / hash fonction
void ht_map(ht_htable* src, ht_htable* dest);



// private
void ht_print(ht_htable* table);


#endif