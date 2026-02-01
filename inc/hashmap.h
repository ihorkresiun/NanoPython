#ifndef __HASHMAP_H__
#define __HASHMAP_H__

#include "vars.h"

typedef struct HashNode {
    const char* key;
    Value value;
    struct HashNode* next;
} HashNode;

typedef struct {
    HashNode* nodes;
    int capacity;
    int count;
} HashMap;

void hash_init(HashMap* map, int initial_capacity);
void hash_set(HashMap* map, const char* key, Value value);
int hash_get(HashMap* map, const char* key, Value* out_value);

#endif // __HASHMAP_H__