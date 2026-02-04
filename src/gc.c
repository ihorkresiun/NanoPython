#include "gc.h"

#include "vm.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

void gc_free_object(VM* vm, Obj* obj);


void mark_hashmap(VM* vm, HashMap* map) {
    if (map == NULL) return;
    for (int i = 0; i < map->capacity; i++) {
        HashNode* node = &map->nodes[i];
        if (node->key != NULL) {  // Check if bucket has data
            gc_mark(vm, (Value){.type=VAL_OBJ, .as.object=(Obj*)node->key});
            gc_mark(vm, node->value);
        }
        // Follow chain
        node = node->next;
        while (node) {
            if (node->key != NULL) {
                gc_mark(vm, (Value){.type=VAL_OBJ, .as.object=(Obj*)node->key});
                gc_mark(vm, node->value);
            }
            node = node->next;
        }
    }
}

void gc_mark(VM* vm, Value value) {
    if (value.type != VAL_OBJ) return;
    if (value.as.object == NULL) return;

    Obj* obj = value.as.object;
    if (obj->marked) return; // Already marked
    
    obj->marked = 1;

    // Recursively mark referenced objects
    switch (obj->type) {
        case OBJ_STRING:
        case OBJ_NATIVE_FUNCTION:
            // No child objects to mark
            break;
        case OBJ_LIST: {
            ObjList* list = (ObjList*)obj;
            for (int i = 0; i < list->count; i++) {
                gc_mark(vm, list->items[i]);
            }
            break;
        }
        case OBJ_DICT: {
            ObjDict* dict = (ObjDict*)obj;
            mark_hashmap(vm, dict->map);
            break;
        }
        case OBJ_TUPLE: {
            ObjTuple* tuple = (ObjTuple*)obj;
            for (int i = 0; i < tuple->count; i++) {
                gc_mark(vm, tuple->items[i]);
            }
            break;
        }
        case OBJ_SET: {
            ObjSet* set = (ObjSet*)obj;
            mark_hashmap(vm, set->map);
            break;
        }

        case OBJ_FUNCTION: {
            ObjFunction* func = (ObjFunction*)obj;
            if (func->scope) {  // Add null check
                mark_hashmap(vm, func->scope->vars);
            }
            break;
        }

        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)obj;
            // Don't mark klass->name - it's a char*, not an Obj*
            mark_hashmap(vm, klass->methods);
            if (klass->parent) {
                gc_mark(vm, (Value){.type=VAL_OBJ, .as.object=(Obj*)klass->parent});
            }
            break;
        }

        case OBJ_INSTANCE: {
            ObjInstance* inst = (ObjInstance*)value.as.object;
            // Mark class
            gc_mark(vm, (Value){.type=VAL_OBJ, .as.object=(Obj*)inst->klass});
            // Mark fields in hashmap
            mark_hashmap(vm, inst->fields);
            break;
        }
    }
}

void gc_mark_roots(VM* vm) {
    // Mark stack values
    for (int i = 0; i < vm->sp; i++) {
        gc_mark(vm, vm->stack[i]);
    }

    // Mark global variables in scope
    Scope* scope = vm->scope;
    while (scope) {
        mark_hashmap(vm, scope->vars);
        scope = scope->parent;
    }

    // Mark call frame scopes
    for (int i = 0; i < vm->frame_count; i++) {
        CallFrame* frame = &vm->call_stack[i];
        if (frame->scope) {
            Scope* s = frame->scope;
            while (s) {
                mark_hashmap(vm, s->vars);
                s = s->parent;
            }
        }
    }

    // Mark constants
    for (int i = 0; i < vm->bytecode->const_count; i++) {
        gc_mark(vm, vm->bytecode->constants[i]);
    }

    // Mark interned strings
    mark_hashmap(vm, &vm->strings);
}

// Helper to free HashMap without recursively freeing contained objects
void gc_hash_free(VM* vm, HashMap* map) {
    if (!map) return;
    
    // Free chain nodes (not the base nodes array itself)
    for (int i = 0; i < map->capacity; i++) {
        HashNode* node = map->nodes[i].next;
        while (node != NULL) {
            HashNode* next = node->next;
            free(node);
            vm->bytes_allocated -= sizeof(HashNode);
            node = next;
        }
    }
    
    // Free the base array and HashMap struct
    free(map->nodes);
    vm->bytes_allocated -= sizeof(HashNode) * map->capacity;
    free(map);
    vm->bytes_allocated -= sizeof(HashMap);
}

void gc_free_object(VM* vm, Obj* obj) {
    switch (obj->type) {
        case OBJ_STRING: {
            ObjString* str = (ObjString*)obj;
            free(str->chars);
            vm->bytes_allocated -= str->length + 1;
            free(str);
            vm->bytes_allocated -= sizeof(ObjString);
            break;
        }
        case OBJ_LIST: {
            ObjList* list = (ObjList*)obj;
            free(list->items);
            vm->bytes_allocated -= sizeof(Value) * list->capacity;
            free(list);
            vm->bytes_allocated -= sizeof(ObjList);
            break;
        }
        case OBJ_DICT: {
            ObjDict* dict = (ObjDict*)obj;
            if (dict->map) {
                gc_hash_free(vm, dict->map);
            }
            free(dict);
            vm->bytes_allocated -= sizeof(ObjDict);
            break;
        }
        case OBJ_TUPLE: {
            ObjTuple* tuple = (ObjTuple*)obj;
            // Don't recursively free items - sweep will handle them
            if (tuple->items) {
                free(tuple->items);
                vm->bytes_allocated -= sizeof(Value) * tuple->count;
            }
            free(tuple);
            vm->bytes_allocated -= sizeof(ObjTuple);
            break;
        }
        case OBJ_SET: {
            ObjSet* set = (ObjSet*)obj;
            if (set->map) {
                gc_hash_free(vm, set->map);
            }
            free(set);
            vm->bytes_allocated -= sizeof(ObjSet);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* func = (ObjFunction*)obj;
            if (func->name) free(func->name);
            if (func->params) {
                for (int i = 0; i < func->param_count; i++) {
                    free(func->params[i]);
                }
                free(func->params);
            }
            free(func);
            vm->bytes_allocated -= sizeof(ObjFunction);
            break;
        }
        case OBJ_CLASS: {
            ObjClass* klass = (ObjClass*)obj;
            if (klass->name) {
                free(klass->name);
                vm->bytes_allocated -= strlen(klass->name) + 1;
            }
            if (klass->methods) {
                gc_hash_free(vm, klass->methods);
            }
            free(klass);
            vm->bytes_allocated -= sizeof(ObjClass);
            break;
        }
        case OBJ_INSTANCE: {
            ObjInstance* inst = (ObjInstance*)obj;
            if (inst->fields) {
                gc_hash_free(vm, inst->fields);
            }
            free(inst);
            vm->bytes_allocated -= sizeof(ObjInstance);
            break;
        }
        case OBJ_NATIVE_FUNCTION: {
            ObjNativeFunction* native = (ObjNativeFunction*)obj;
            free(native);
            vm->bytes_allocated -= sizeof(ObjNativeFunction);
            break;
        }
    }
}

void gc_sweep(VM* vm) {
    Obj** obj = &vm->objects;
    while (*obj) {
        if (!(*obj)->marked) {
            // This object wasn't reached, so free it
            Obj* unreached = *obj;
            *obj = unreached->next; // Remove from list

            gc_free_object(vm, unreached);
        } else {
            // This object was reached, unmark it for the next GC cycle
            (*obj)->marked = 0;
            obj = &(*obj)->next; // Move to next object
        }
    }
}

void gc_collect(VM* vm) {
    int before = vm->bytes_allocated;

    gc_mark_roots(vm);
    gc_sweep(vm);

    int after = vm->bytes_allocated;
    vm->next_gc = after * 2; // Set next GC threshold
    printf("GC collected %d bytes, %d remaining\n", before - after, after);
}