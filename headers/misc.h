#ifndef HEAD_MISC
#define HEAD_MISC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// compare 2 int
int misc_comp_int(const void* a, const void* b);

// compare 2 uint
int misc_comp_uint(const void* a, const void* b);

// compare 2 float
int misc_comp_float(const void* a, const void* b);

// compare 2 size_t
int misc_comp_sizet(const void* a, const void* b);

// compare 2 int but inverse the output
int misc_comp_int_inv(const void* a, const void* b);

// compare 2 uint but inverse the output
int misc_comp_uint_inv(const void* a, const void* b);


// compare 2 float but inverse the output
int misc_comp_float_inv(const void* a, const void* b);

// return a malloced copy of the input string
// len of output string is <= sizeMax
// if sizeMax = 0 then no restriction is done on the output string
// returned adress must be free
// if an error occured will call error_fn() with the error message if error_fn is not NULL and return NULL
char* misc_createstr(const char* str, size_t sizeMax, void (*error_fn)(char*));

// return a malloced copy of the input int
// returned adress must be free
// if an error occured will call error_fn() with the error message if error_fn is not NULL and return NULL
int* misc_createint(int a, void (*error_fn)(char*));

// return a malloced copy of the input uint
// returned adress must be free
// if an error occured will call error_fn() with the error message if error_fn is not NULL and return NULL
uint* misc_createuint(uint a, void (*error_fn)(char*));

// return a malloced arr which have been copied nmemb elements of bSize bytes from dest
// and with at the last element src
// will pass dest to the free_fn() if free_fn is not NULL
// if an error occured will call error_fn() with the error message if error_fn is not NULL and return NULL
void* misc_arrAdd(void* dest, size_t nmemb, size_t bSize, void* src, void (*free_fn)(void*), void (*error_fn)(char*));

// return a malloced arr which is all elements of bSize bytes from dest
// exept the one at the given index
// will pass dest to the free_fn() if free_fn is not NULL
// if an error occured will call error_fn() with the error message if error_fn is not NULL and return NULL
void* misc_arrRm(void* dest, size_t nmemb, size_t bSize, size_t index, void (*free_fn)(void*), void (*error_fn)(char*));

// print to stderr the given string
void misc_printErr(const char* str);

#endif