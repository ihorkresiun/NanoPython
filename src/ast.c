#include "ast.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

static Ast* ast_new_node() {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }
    return node;
}

Ast* ast_new_number(int value) {
    Ast* node = ast_new_node();
    node->type = AST_NUMBER;
    node->NumberInt.value = value;
    return node;
}

Ast* ast_new_number_float(double value) {
    Ast* node = ast_new_node();
    node->type = AST_FLOAT;
    node->NumberFloat.value = value;
    return node;
}

Ast* ast_new_string(const char* value) {
    Ast* node = ast_new_node();
    node->type = AST_STRING;
    node->String.value = strdup(value);
    return node;
}

Ast* ast_new_binary_expr(TokenType type, Ast* left, Ast* right) {
    Ast* node = ast_new_node();
    node->type = AST_BINARY;
    node->Binary.left = left;
    node->Binary.right = right;
    node->Binary.op = type;
    return node;
}

Ast* ast_new_unary_expr(TokenType type, Ast* value) {
    Ast* node = ast_new_node();
    node->type = AST_UNARY;
    node->Unary.value = value;
    node->Unary.op = type;
    return node;
}

Ast* ast_new_var(const char* name) {
    Ast* node = ast_new_node();
    node->type = AST_VAR;
    node->Variable.name = strdup(name);
    return node;
}

Ast* ast_new_assign(const char* name, Ast* value) {
    Ast* node = ast_new_node();
    node->type = AST_ASSIGN;
    node->Assign.name = strdup(name);
    node->Assign.value = value;
    return node;
}

Ast* ast_new_if(Ast* condition, Ast* then_block, Ast* else_block) {
    Ast* node = ast_new_node();
    node->type = AST_IF;
    node->If.condition = condition;
    node->If.then_branch = then_block;
    node->If.else_branch = else_block;
    return node;
}

Ast* ast_new_while(Ast* condition, Ast* body) {
    Ast* node = ast_new_node();
    node->type = AST_WHILE;
    node->While.condition = condition;
    node->While.body = body;
    return node;
}

Ast* ast_new_for(const char* var, Ast* iterable, Ast* body) {
    Ast* node = ast_new_node();
    node->type = AST_FOR;
    node->For.var = strdup(var);
    node->For.iterable = iterable;
    node->For.body = body;
    return node;
}

Ast* ast_new_block(Ast** statements, int count) {
    Ast* node = ast_new_node();
    node->type = AST_BLOCK;
    node->Block.statements = statements;
    node->Block.count = count;
    return node;
}

Ast* ast_new_list(Ast** elements, int count) {
    Ast* node = ast_new_node();
    node->type = AST_LIST;
    node->List.elements = elements;
    node->List.count = count;
    return node;
}

Ast* ast_new_dict(Ast** keys, Ast** values, int count) {
    Ast* node = ast_new_node();
    node->type = AST_DICT;
    node->Dict.keys = keys;
    node->Dict.values = values;
    node->Dict.count = count;
    return node;
}

Ast* ast_new_index(Ast* target, Ast* index) {
    Ast* node = ast_new_node();
    node->type = AST_INDEX;
    node->Index.target = target;
    node->Index.index = index;
    return node;
}

Ast* ast_new_assign_index(Ast* target, Ast* index, Ast* value) {
    Ast* node = ast_new_node();
    node->type = AST_ASSIGN_INDEX;
    node->AssignIndex.target = target;
    node->AssignIndex.index = index;
    node->AssignIndex.value = value;
    return node;
}

Ast* ast_new_funcdef(const char* name, char** args, int argc, Ast* body) {
    Ast* node = ast_new_node();
    node->type = AST_FUNCDEF;
    node->FuncDef.name = strdup(name);
    node->FuncDef.args = args;
    node->FuncDef.argc = argc;
    node->FuncDef.body = body;
    return node;
}

Ast* ast_new_call(const char* name, Ast** args, int argc) {
    Ast* node = ast_new_node();
    node->type = AST_CALL;
    node->Call.name = strdup(name);
    node->Call.args = args;
    node->Call.argc = argc;
    return node;
}

Ast* ast_new_return(Ast* value) {
    Ast* node = ast_new_node();
    node->type = AST_RETURN;
    node->Return.value = value;
    return node;
}

Ast* ast_new_break() {
    Ast* node = ast_new_node();
    node->type = AST_BREAK;
    return node;
}

Ast* ast_new_continue() {
    Ast* node = ast_new_node();
    node->type = AST_CONTINUE;
    return node;
}

Ast* ast_new_classdef(const char* name, const char* parent, Ast** methods, int method_count) {
    Ast* node = ast_new_node();
    node->type = AST_CLASSDEF;
    node->ClassDef.name = strdup(name);
    node->ClassDef.parent = parent ? strdup(parent) : NULL;
    node->ClassDef.methods = methods;
    node->ClassDef.method_count = method_count;
    return node;
}

Ast* ast_new_method_call(Ast* object, const char* method_name, Ast** args, int argc) {
    Ast* node = ast_new_node();
    node->type = AST_METHOD_CALL;
    node->MethodCall.object = object;
    node->MethodCall.method_name = strdup(method_name);
    node->MethodCall.args = args;
    node->MethodCall.argc = argc;
    return node;
}

Ast* ast_new_attr_access(Ast* object, const char* attr_name) {
    Ast* node = ast_new_node();
    node->type = AST_ATTR_ACCESS;
    node->AttrAccess.object = object;
    node->AttrAccess.attr_name = strdup(attr_name);
    return node;
}

Ast* ast_new_attr_assign(Ast* object, const char* attr_name, Ast* value) {
    Ast* node = ast_new_node();
    node->type = AST_ATTR_ASSIGN;
    node->AttrAssign.object = object;
    node->AttrAssign.attr_name = strdup(attr_name);
    node->AttrAssign.value = value;
    return node;
}

Ast* ast_new_import(const char* module_name) {
    Ast* node = ast_new_node();
    node->type = AST_IMPORT;
    node->Import.module_name = strdup(module_name);
    return node;
}

void ast_free(Ast* node) {
    if (node == NULL) return;

    switch (node->type)
    {
        case AST_NUMBER:
            // Nothing to free
        break;
        case AST_STRING:
            free(node->String.value);
        break;
        case AST_BINARY:
            ast_free(node->Binary.left);
            ast_free(node->Binary.right);
        break;
        case AST_UNARY:
            ast_free(node->Unary.value);
        break;
        case AST_VAR:
            free(node->Variable.name);
        break;
        case AST_ASSIGN:
            free(node->Assign.name);
            ast_free(node->Assign.value);
        break;
        case AST_IF:
            ast_free(node->If.condition);
            ast_free(node->If.then_branch);
            ast_free(node->If.else_branch);
        break;
        case AST_ELSE:
            // Handled in AST_IF
        break;
        case AST_WHILE:
            ast_free(node->While.condition);
            ast_free(node->While.body);
        break;
        case AST_FOR:
            free((char*)node->For.var);
            ast_free(node->For.iterable);
            ast_free(node->For.body);
        break;
        case AST_BLOCK:
            for (int i = 0; i < node->Block.count; i++) {
                ast_free(node->Block.statements[i]);
            }
            free(node->Block.statements);
        break;
        case AST_LIST:
            for (int i = 0; i < node->List.count; i++) {
                ast_free(node->List.elements[i]);
            }
            free(node->List.elements);
        break;
        case AST_DICT:
            for (int i = 0; i < node->Dict.count; i++) {
                ast_free(node->Dict.keys[i]);
                ast_free(node->Dict.values[i]);
            }
            free(node->Dict.keys);
            free(node->Dict.values);
        break;
        case AST_INDEX:
            ast_free(node->Index.target);
            ast_free(node->Index.index);
        break;
        case AST_ASSIGN_INDEX:
            ast_free(node->AssignIndex.target);
            ast_free(node->AssignIndex.index);
            ast_free(node->AssignIndex.value);
        break;
        case AST_FUNCDEF:
            free(node->FuncDef.name);
            for (int i = 0; i < node->FuncDef.argc; i++) {
                free(node->FuncDef.args[i]);
            }
            free(node->FuncDef.args);
            ast_free(node->FuncDef.body);
        break;
        case AST_CALL:
            free(node->Call.name);
            for (int i = 0; i < node->Call.argc; i++) {
                ast_free(node->Call.args[i]);
            }
            free(node->Call.args);
        break;
        case AST_RETURN:
            ast_free(node->Return.value);
        break;
        case AST_BREAK:
        case AST_CONTINUE:
            // Nothing to free
        break;
        case AST_CLASSDEF:
            free(node->ClassDef.name);
            if (node->ClassDef.parent) free(node->ClassDef.parent);
            for (int i = 0; i < node->ClassDef.method_count; i++) {
                ast_free(node->ClassDef.methods[i]);
            }
            free(node->ClassDef.methods);
        break;
        case AST_METHOD_CALL:
            ast_free(node->MethodCall.object);
            free(node->MethodCall.method_name);
            for (int i = 0; i < node->MethodCall.argc; i++) {
                ast_free(node->MethodCall.args[i]);
            }
            free(node->MethodCall.args);
        break;
        case AST_ATTR_ACCESS:
            ast_free(node->AttrAccess.object);
            free(node->AttrAccess.attr_name);
        break;
        case AST_ATTR_ASSIGN:
            ast_free(node->AttrAssign.object);
            free(node->AttrAssign.attr_name);
            ast_free(node->AttrAssign.value);
        break;
        case AST_IMPORT:
            free(node->Import.module_name);
        break;

        default:
            printf("Unknown AST node type in ast_free: %d\n", node->type);
            exit(1);
        break;
        
    }
    free(node);
}