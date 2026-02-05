#include "hashmap.h"

#include "stdlib.h"
#include "string.h"
#include "stdint.h"
#include "stdio.h"

void hash_init(HashMap* map, int initial_capacity) {
    map->capacity = initial_capacity;
    map->count = 0;
    map->nodes = malloc(sizeof(HashNode) * map->capacity);
    for (int i = 0; i < map->capacity; i++) {
        map->nodes[i].key = NULL;
        map->nodes[i].value = (Value){0};
        map->nodes[i].next = NULL;
    }
}

uint32_t hash_string(const char* str){
    // DJB2 hash function
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

void hash_set(HashMap* map, ObjString* key, Value value) {
    uint32_t index = hash_string(key->chars) & (map->capacity - 1);
    HashNode* node = &map->nodes[index];

    // Handle collisions with chaining
    while (node->key != NULL) {
        if (strcmp(node->key->chars, key->chars) == 0) {
            // Key already exists, update value
            node->value = value;
            return;
        }
        if (node->next == NULL) break;
        node = node->next;
    }

    if (node->key == NULL) {
        // New key
        node->key = key;
        node->value = value;
        map->count++;
    } else {
        // Collision, add new node to the chain
        HashNode* new_node = malloc(sizeof(HashNode));
        new_node->key = key;
        new_node->value = value;
        new_node->next = NULL;
        node->next = new_node;
        map->count++;
    }
}

int hash_get(HashMap* map, ObjString* key, Value* out_value) {
    uint32_t index = hash_string(key->chars) & (map->capacity - 1);
    HashNode* node = &map->nodes[index];

    while (node != NULL && node->key != NULL) {
        if (node->key->length == key->length && strcmp(node->key->chars, key->chars) == 0) {
            *out_value = node->value;
            return 1; // Found
        }
        node = node->next;
    }
    return 0; // Not found
}

void hash_debug_print(HashMap* map) {
    for (int i = 0; i < map->capacity; i++) {
        HashNode* node = &map->nodes[i];
        if (node->key != NULL) {
            printf("Bucket %d: ", i);
            while (node != NULL) {
                printf("[Key: %s, Value Type: %d] -> ", node->key->chars, node->value.type);
                node = node->next;
            }
            printf("NULL\n");
        }
    }
}