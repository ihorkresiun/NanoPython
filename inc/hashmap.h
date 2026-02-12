#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "vars.h"

#include "stdint.h"

#define HASH_MAX_LOAD_FACTOR 0.75

typedef struct HashNode {
    ObjString* key;
    Value value;
    struct HashNode* next;
} HashNode;

typedef struct HashMap {
    HashNode* nodes;
    int capacity;
    int count;
} HashMap;

void hash_init(HashMap* map, int initial_capacity);
void hash_set(HashMap* map, ObjString* key, Value value);
int hash_get(HashMap* map, ObjString* key, Value* out_value);

uint32_t hash_string(const char* str);

void hash_print(HashMap* map);
void hash_free(HashMap* map);

#endif // __HASHMAP_H__