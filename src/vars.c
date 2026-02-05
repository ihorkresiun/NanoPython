#include "vars.h"

#include "hashmap.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"

Scope* new_scope(const char* name, Scope* parent) {
    Scope* scope = malloc(sizeof(Scope));
    scope->name = name;
    scope->vars = malloc(sizeof(HashMap));
    hash_init(scope->vars, 16);
    scope->parent = parent;
    scope->return_value = make_none();
    return scope;
}

Value scope_find(Scope* scope, ObjString* name) {
    while (scope) {
        Value ret;
        if (hash_get(scope->vars, name, &ret)) {
            return ret;
        }

        scope = scope->parent;
    }

    return (Value){.type = VAL_NONE};
}

void scope_set(Scope* scope, ObjString* name, Value value) {
    hash_set(scope->vars, name, value);
}

int is_true(Value v) {
    switch (v.type) {
        case VAL_BOOL: return v.as.boolean;
        case VAL_FLOAT: return v.as.floating != 0;
        case VAL_NONE: return 0;
        default: return 1;
    }
}

Value make_number_int(int x) {
    return (Value){.type = VAL_INT, .as.integer = x};
}

Value make_number_float(double x) {
    return (Value){.type = VAL_FLOAT, .as.floating = x};
}

Value make_bool(int b) {
    return (Value){.type = VAL_BOOL, .as.boolean = b};
}

int is_obj_type(Value v, ObjectType type) {
    return v.type == VAL_OBJ && v.as.object->type == type;
}

ObjString* as_string(Value v) {
    return (ObjString*)v.as.object;
}

Value make_const_string(const char* s) {
    Value v;
    v.type = VAL_OBJ;

    ObjString* string = malloc(sizeof(ObjString));
    string->obj.type = OBJ_STRING;
    string->length = strlen(s);
    string->chars = strdup(s);

    v.as.object = (Obj*)string;
    return v;
}

Value make_function(ObjFunction* fn) {
    Value v;
    v.type = VAL_OBJ;
    v.as.object = (Obj*)fn;
    return v;
}

Value make_native_function(const char* name, NativeFn function) {
    ObjNativeFunction* native_fn = malloc(sizeof(ObjNativeFunction));
    native_fn->obj.type = OBJ_NATIVE_FUNCTION;
    native_fn->function = function;
    native_fn->name = strdup(name);
    Value v = {0};
    v.type = VAL_OBJ;
    v.as.object = (Obj*)native_fn;
    return v;
}

Value make_none() {
    return (Value){.type = VAL_NONE};
}

static void print_node(HashNode* node) {
    if (node == NULL) return;
    print_value((Value){.type=VAL_OBJ, .as.object=(Obj*)node->key});
    printf(": ");
    print_value(node->value);
    printf(", ");
}

static void print_dict(ObjDict* dict) {
    printf("{");
    for (int i = 0; i < dict->capacity; i++) {
        HashNode* node = &dict->map->nodes[i];
        if (node->key != NULL) {  // Check if bucket has data
            print_node(node);
        }
        // Follow chain
        node = node->next;
        while (node) {
            if (node->key != NULL) {
                print_node(node);
            }
            node = node->next;
        }
    }
    printf("}");
}

void print_value(Value v) {
    switch (v.type) {
        case VAL_INT:
            printf("%ld", v.as.integer);
        break;
        case VAL_FLOAT:
            printf("%g", v.as.floating);
        break;
        case VAL_BOOL:
            printf("%s", v.as.boolean ? "True" : "False");
        break;
        case VAL_NONE:
            printf("None");
        break;
        case VAL_OBJ:
            if (v.as.object->type == OBJ_STRING) {
                ObjString* str = (ObjString*)v.as.object;
                printf("%s", str->chars);
                break;
            } else if (v.as.object->type == OBJ_LIST) {
                ObjList* list = (ObjList*)v.as.object;
                printf("[");
                for (int i = 0; i < list->count; i++) {
                    print_value(list->items[i]);
                    if (i < list->count - 1) {
                        printf(", ");
                    }
                }
                printf("]");
            } else if (v.as.object->type == OBJ_DICT) {
                ObjDict* dict = (ObjDict*)v.as.object;
                print_dict(dict);
            } else if (v.as.object->type == OBJ_TUPLE) {
                ObjTuple* tuple = (ObjTuple*)v.as.object;
                printf("(");
                for (int i = 0; i < tuple->count; i++) {
                    print_value(tuple->items[i]);
                    if (i < tuple->count - 1) {
                        printf(", ");
                    }
                }
                // Special case for single element tuple
                if (tuple->count == 1) {
                    printf(",");
                }
                printf(")");
            } else if (v.as.object->type == OBJ_SET) {
                ObjSet* set = (ObjSet*)v.as.object;
                if (set->count == 0) {
                    printf("set()");
                } else {
                    printf("{");
                    int printed = 0;
                    // Iterate through set's hashmap
                    if (set->map) {
                        for (int i = 0; i < set->map->capacity; i++) {
                            HashNode* node = &set->map->nodes[i];
                            if (node->key != NULL) {
                                if (printed > 0) printf(", ");
                                print_value(node->value);
                                printed++;
                                
                                // Follow the chain
                                HashNode* next = node->next;
                                while (next != NULL) {
                                    if (printed > 0) printf(", ");
                                    print_value(next->value);
                                    printed++;
                                    next = next->next;
                                }
                            }
                        }
                    }
                    printf("}");
                }
            } else if (v.as.object->type == OBJ_NATIVE_FUNCTION) {
                ObjNativeFunction* native_fn = (ObjNativeFunction*)v.as.object;
                printf("<native function %s>", native_fn->name);
            } else if (v.as.object->type == OBJ_FUNCTION) {
                printf("<function>");
            } else if (v.as.object->type == OBJ_CLASS) {
                ObjClass* klass = (ObjClass*)v.as.object;
                printf("<class '%s'>", klass->name);
            } else if (v.as.object->type == OBJ_INSTANCE) {
                ObjInstance* instance = (ObjInstance*)v.as.object;
                printf("<%s instance>", instance->klass->name);
            } else {
                printf("<Unknown object type %d>", v.as.object->type);
            }

        break;
        
        default:
            printf("<unknown>");
    }
}

int value_equals(Value* a, Value* b) {
    if (a->type != b->type) return 0;

    switch (a->type) {
        case VAL_INT:
            return a->as.integer == b->as.integer;
        case VAL_FLOAT:
            return a->as.floating == b->as.floating;
        case VAL_BOOL:
            return a->as.boolean == b->as.boolean;
        case VAL_NONE:
            return 1; // Both are none
        case VAL_OBJ: {
            if (a->as.object->type != b->as.object->type) return 0;

            if (a->as.object->type == OBJ_STRING) {
                ObjString* str_a = (ObjString*)a->as.object;
                ObjString* str_b = (ObjString*)b->as.object;
                return strcmp(str_a->chars, str_b->chars) == 0;
            }

            return 0;

        }

        default:
            return 0;
    }
}
