#include "vm_objects.h"

#include "gc.h"
#include "vm.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

Obj* alloc_object(VM* vm, size_t size, ObjectType type) {
    Obj* object = malloc(size);
    vm->bytes_allocated += size;
    if (vm->bytes_allocated > vm->next_gc) {
        gc_collect(vm);
    }
    object->type = type;
    object->marked = 0;
    object->next = vm->objects;
    vm->objects = object;
    return object;
}

Value vm_make_string(VM* vm, const char* s) {
    ObjString* string = (ObjString*)alloc_object(vm, sizeof(ObjString), OBJ_STRING);
    string->length = strlen(s);
    string->chars = strdup(s);
    vm->bytes_allocated += string->length + 1;  // Track string data
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)string;
    return v;
}

Value vm_make_list(VM* vm, int count) {
    ObjList* list = (ObjList*)alloc_object(vm, sizeof(ObjList), OBJ_LIST);
    list->count = 0;
    int capacity = count > 4 ? count : 4;  // Use count or minimum of 4
    list->capacity = capacity;
    list->items = malloc(sizeof(Value) * capacity);
    vm->bytes_allocated += sizeof(Value) * capacity;
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)list;
    return v;
}

Value vm_make_dict(VM* vm) {
    ObjDict* dict = (ObjDict*)alloc_object(vm, sizeof(ObjDict), OBJ_DICT);
    dict->count = 0;
    dict->capacity = 4;
    dict->map = malloc(sizeof(HashMap));
    hash_init(dict->map, 4);
    vm->bytes_allocated += sizeof(HashMap) + sizeof(HashNode) * 4;
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)dict;
    return v;
}

Value vm_make_tuple(VM* vm) {
    ObjTuple* tuple = (ObjTuple*)alloc_object(vm, sizeof(ObjTuple), OBJ_TUPLE);
    tuple->count = 0;
    tuple->items = NULL;
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)tuple;
    return v;
}

Value vm_make_set(VM* vm) {
    ObjSet* set = (ObjSet*)alloc_object(vm, sizeof(ObjSet), OBJ_SET);
    set->count = 0;
    set->capacity = 4;
    set->map = malloc(sizeof(HashMap));
    hash_init(set->map, 4);
    vm->bytes_allocated += sizeof(HashMap) + sizeof(HashNode) * 4;
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)set;
    return v;
}

Value vm_make_class(VM* vm, const char* name, ObjClass* parent) {
    ObjClass* klass = (ObjClass*)alloc_object(vm, sizeof(ObjClass), OBJ_CLASS);
    klass->name = strdup(name);
    vm->bytes_allocated += strlen(name) + 1;  // Track name string
    klass->methods = malloc(sizeof(HashMap));
    hash_init(klass->methods, 8);
    vm->bytes_allocated += sizeof(HashMap) + sizeof(HashNode) * 8;
    klass->parent = parent;
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)klass;
    return v;
}

Value vm_make_instance(VM* vm, ObjClass* klass) {
    ObjInstance* instance = (ObjInstance*)alloc_object(vm, sizeof(ObjInstance), OBJ_INSTANCE);
    instance->klass = klass;
    instance->fields = malloc(sizeof(HashMap));
    hash_init(instance->fields, 8);
    vm->bytes_allocated += sizeof(HashMap) + sizeof(HashNode) * 8;
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)instance;
    return v;
}
