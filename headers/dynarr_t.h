#ifndef HEAD_DYNARR_T
#define HEAD_DYNARR_T

#include <stdlib.h>

void* dynarr_create(size_t byteSize, size_t size);
void _dynarr_priv_pushBack(void** arrPtr, void* value);
void _dynarr_priv_pushFront(void** arrPtr, void* value);
void _dynarr_priv_popBack(void** arrPtr, void* buff);
void _dynarr_priv_popFront(void** arrPtr, void* buff);
size_t dynarr_size(void* arr);
void dynarr_free(void* arr);
void dynarr_qsort(void* arr, int (*compar_fn) (const void *, const void *));

#define DYNARR_DEF_H_CREATE(type) \
    type* dynarr_create_##type(size_t size)

#define DYNARR_DEF_CREATE(type) \
    DYNARR_DEF_H_CREATE(type) { \
        return (type*)dynarr_create(sizeof(type), size); \
    } 

#define DYNARR_DEF_H_PUSHBACK(type) \
    type dynarr_pushBack_##type(type** arrPtr, type value)

#define DYNARR_DEF_PUSHBACK(type) \
    DYNARR_DEF_H_PUSHBACK(type) { \
        _dynarr_priv_pushBack((void**)arrPtr, &value); \
        return value; \
    }

#define DYNARR_DEF_H_PUSHFRONT(type) \
    type dynarr_pushFront_##type(type** arrPtr, type value)

#define DYNARR_DEF_PUSHFRONT(type) \
    DYNARR_DEF_H_PUSHFRONT(type) { \
        _dynarr_priv_pushFront((void**)arrPtr, &value); \
        return value; \
    }

#define DYNARR_DEF_H_POPBACK(type) \
    type dynarr_popBack_##type(type** arrPtr)

#define DYNARR_DEF_POPBACK(type) \
    DYNARR_DEF_H_POPBACK(type) { \
        type buff; \
        _dynarr_priv_popBack((void**)arrPtr, &buff); \
        return buff; \
    }

#define DYNARR_DEF_H_POPFRONT(type) \
    type dynarr_popFront_##type(type** arrPtr)

#define DYNARR_DEF_POPFRONT(type) \
    DYNARR_DEF_H_POPFRONT(type) { \
        type buff; \
        _dynarr_priv_popFront((void**)arrPtr, &buff); \
        return buff; \
    }

#define DYNARR_DEF_H_MAP(fromType, toType) \
    toType* dynarr_map_##fromType_to_##toType(fromType* arr, toType (*map_fn)(fromType, size_t))

#define DYNARR_DEF_MAP(fromType, toType) \
    DYNARR_DEF_H_MAP(fromType, toType) { \
        size_t arrSize = dynarr_size(arr); \
        toType* newArr = dynarr_create(sizeof(toType), arrSize); \
        for(size_t i = 0; i < arrSize; i++) { \
            newArr[i] = map_fn(arr[i], i); \
        } \
        return newArr; \
    }

#define DYNARR_DEF_ALL(type) \
    DYNARR_DEF_CREATE(type) \
    DYNARR_DEF_PUSHBACK(type) \
    DYNARR_DEF_PUSHFRONT(type) \
    DYNARR_DEF_POPBACK(type) \
    DYNARR_DEF_POPFRONT(type)

#define DYNARR_DEF_H_ALL(type) \
    DYNARR_DEF_H_CREATE(type) \
    DYNARR_DEF_H_PUSHBACK(type) \
    DYNARR_DEF_H_PUSHFRONT(type) \
    DYNARR_DEF_H_POPBACK(type) \
    DYNARR_DEF_H_POPFRONT(type)
    


#endif