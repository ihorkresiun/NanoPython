#include "intern_string.h"

#include "hashmap.h"

#include "string.h"
#include "stdlib.h"
#include "stdint.h"

static ObjString* find_string(HashMap* map, const char* chars, int length) {
    if (map->count == 0 || length == 0) return NULL;
    uint32_t hash = hash_string(chars);
    uint32_t index = hash & (map->capacity - 1);
    HashNode* node = &map->nodes[index];

    while (node != NULL && node->key != NULL && node->key->length == length) {
        if (memcmp(node->key->chars, chars, length) == 0) {
            return node->key;
        }
        node = node->next;
    }
    return NULL;
}

static ObjString* make_obj_string(const char* chars, int length) {
    ObjString* string = malloc(sizeof(ObjString));
    string->obj.type = OBJ_STRING;
    string->length = length;
    string->chars = malloc(length + 1);
    memcpy(string->chars, chars, length);
    string->chars[length] = '\0';
    return string;
}

ObjString* intern_string(VM* map, char* chars, int length) {
    ObjString* interned = find_string(&map->strings, chars, length);
    if (interned) {
        free(chars); // Free the input string since it's not used
        return interned;
    }

    ObjString* new_str = make_obj_string(chars, length);

    hash_set(&map->strings, new_str, make_none());

    return new_str;
}

ObjString* intern_const_string(VM* map, const char* chars, int length) {
    ObjString* interned = find_string(&map->strings, chars, length);
    if (interned) {
        return interned;
    }

    ObjString* new_str = make_obj_string(chars, length);

    hash_set(&map->strings, new_str, make_none());

    return new_str;
}