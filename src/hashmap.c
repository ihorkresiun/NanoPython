#include "hashmap.h"

#include "stdlib.h"
#include "string.h"
#include "stdint.h"

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

static uint32_t hash_string(const char* str) {
    // DJB2 hash function
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash;
}

void hash_set(HashMap* map, const char* key, Value value) {
    uint32_t index = hash_string(key) % map->capacity;
    HashNode* node = &map->nodes[index];

    // Handle collisions with chaining
    while (node->key != NULL) {
        if (strcmp(node->key, key) == 0) {
            // Key already exists, update value
            node->value = value;
            return;
        }
        if (node->next == NULL) break;
        node = node->next;
    }

    if (node->key == NULL) {
        // New key
        node->key = strdup(key);
        node->value = value;
        map->count++;
    } else {
        // Collision, add new node to the chain
        HashNode* new_node = malloc(sizeof(HashNode));
        new_node->key = strdup(key);
        new_node->value = value;
        new_node->next = NULL;
        node->next = new_node;
        map->count++;
    }
}

int hash_get(HashMap* map, const char* key, Value* out_value) {
    uint32_t index = hash_string(key) % map->capacity;
    HashNode* node = &map->nodes[index];

    while (node != NULL && node->key != NULL) {
        if (strcmp(node->key, key) == 0) {
            *out_value = node->value;
            return 1; // Found
        }
        node = node->next;
    }
    return 0; // Not found
}
