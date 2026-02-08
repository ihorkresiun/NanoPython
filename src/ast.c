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

Ast* ast_new_tuple(Ast** elements, int count) {
    Ast* node = ast_new_node();
    node->type = AST_TUPLE;
    node->Tuple.elements = elements;
    node->Tuple.count = count;
    return node;
}

Ast* ast_new_set(Ast** elements, int count) {
    Ast* node = ast_new_node();
    node->type = AST_SET;
    node->Set.elements = elements;
    node->Set.count = count;
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

const TokenName token_type_name[] = {
    {TOKEN_EOF, "EOF"},
    {TOKEN_NUMBER, "NUMBER"},
    {TOKEN_FLOAT, "FLOAT"},
    {TOKEN_STRING, "STRING"},
    {TOKEN_IDENT, "IDENT"},
    {TOKEN_MINUS, "MINUS"},
    {TOKEN_PLUS, "PLUS"},
    {TOKEN_STAR, "STAR"},
    {TOKEN_SLASH, "SLASH"},
    {TOKEN_LPAREN, "LPAREN"},
    {TOKEN_RPAREN, "RPAREN"},
    {TOKEN_LBRACKET, "LBRACKET"},
    {TOKEN_RBRACKET, "RBRACKET"},
    {TOKEN_LBRACE, "LBRACE"},
    {TOKEN_RBRACE, "RBRACE"},
    {TOKEN_COMMA, "COMMA"},
    {TOKEN_CARET, "CARET"},
    {TOKEN_ASSIGN, "ASSIGN"},
    {TOKEN_DOT, "DOT"},
    {TOKEN_IF, "IF"},
    {TOKEN_ELSE, "ELSE"},
    {TOKEN_WHILE, "WHILE"},
    {TOKEN_DEF, "DEF"},
    {TOKEN_CLASS, "CLASS"},
    {TOKEN_RETURN, "RETURN"},
    {TOKEN_BREAK, "BREAK"},
    {TOKEN_CONTINUE, "CONTINUE"},
    {TOKEN_FOR, "FOR"},
    {TOKEN_IN, "IN"},
    {TOKEN_IMPORT, "IMPORT"},
    {TOKEN_FROM, "FROM"},
    {TOKEN_COLON, "COLON"},
    {TOKEN_LT, "LT"},
    {TOKEN_GT, "GT"},
    {TOKEN_LE, "LE"},
    {TOKEN_GE, "GE"},
    {TOKEN_EQ, "EQ"},
    {TOKEN_NE, "NE"},
    {TOKEN_NEWLINE, "NEWLINE"},
    {TOKEN_INDENT, "INDENT"},
    {TOKEN_DEDENT, "DEDENT"},
    {TOKEN_AND, "AND"},
    {TOKEN_OR, "OR"},
    {TOKEN_NOT, "NOT"},
};

static void print_indent(FILE* f, int indent)
{
    for (int i = 0; i < indent; i++) {
        fprintf(f, "  ");
    }
}

static void ast_fprint(Ast* node, FILE* f, int indent) {
    if (node == NULL) return;

    print_indent(f, indent);

    switch (node->type)
    {
        case AST_NUMBER:
            fprintf(f, "Number: %ld\n", node->NumberInt.value);
        break;
        case AST_FLOAT:
            fprintf(f, "Float: %f\n", node->NumberFloat.value);
        break;
        case AST_STRING:
            fprintf(f, "String: \"%s\"\n", node->String.value);
        break;
        case AST_BINARY:
            fprintf(f, "Binary Expression:\n");
            ast_fprint(node->Binary.left, f, indent + 1);
            print_indent(f, indent + 1);
            fprintf(f, "%s", token_type_name[node->Binary.op].name);
            ast_fprint(node->Binary.right, f, indent + 1);
        break;
        case AST_UNARY:
            fprintf(f, "Unary Expression: %s\n", token_type_name[node->Unary.op].name);
            ast_fprint(node->Unary.value, f, indent + 1);
        break;
        case AST_VAR:
            fprintf(f, "Variable: %s\n", node->Variable.name);
        break;
        case AST_ASSIGN:
            fprintf(f, "Assignment: %s\n", node->Assign.name);
            ast_fprint(node->Assign.value, f, indent + 1);
        break;
        case AST_IF:
            fprintf(f, "If Statement:\n");

            fprintf(f, "Condition:\n");
            ast_fprint(node->If.condition, f, indent + 1);
            fprintf(f, "Then:\n");
            ast_fprint(node->If.then_branch, f, indent + 1);
            if (node->If.else_branch) {
                fprintf(f, "Else:\n");
                ast_fprint(node->If.else_branch, f, indent + 1);
            }
        break;
        case AST_WHILE:
            fprintf(f, "While Loop:\n");
            print_indent(f, indent);
            fprintf(f, "Condition:\n");
            ast_fprint(node->While.condition, f, indent + 1);
            print_indent(f, indent);
            fprintf(f, "Body:\n");
            ast_fprint(node->While.body, f, indent + 2);
        break;
        case AST_FOR:
            fprintf(f, "For Loop: %s in\n", node->For.var);
            print_indent(f, indent);
            fprintf(f, "Iterable:\n");
            ast_fprint(node->For.iterable, f, indent + 1);
            print_indent(f, indent);
            fprintf(f, "Body:\n");
            ast_fprint(node->For.body, f, indent + 2);
        break;
        case AST_BLOCK:
            fprintf(f, "Block:\n");
            for (int i = 0; i < node->Block.count; i++) {
                ast_fprint(node->Block.statements[i], f, indent + 1);
            }
        break;
        case AST_LIST:
            fprintf(f, "List:\n");
            for (int i = 0; i < node->List.count; i++) {
                ast_fprint(node->List.elements[i], f, indent + 1);
            }
        break;
        case AST_DICT:
            fprintf(f, "Dict:\n");
            for (int i = 0; i < node->Dict.count; i++) {
                print_indent(f, indent);
                fprintf(f, "Key:\n");
                ast_fprint(node->Dict.keys[i], f, indent + 2);
                print_indent(f, indent);
                fprintf(f, "Value:\n");
                ast_fprint(node->Dict.values[i], f, indent + 2);
            }
        break;
        case AST_SET:
            fprintf(f, "Set:\n");
            for (int i = 0; i < node->Set.count; i++) {
                ast_fprint(node->Set.elements[i], f, indent + 1);
            }
        break;
        case AST_TUPLE:
            fprintf(f, "Tuple:\n");
            for (int i = 0; i < node->Tuple.count; i++) {
                ast_fprint(node->Tuple.elements[i], f, indent + 1);
            }
        break;
        case AST_INDEX:
            fprintf(f, "Indexing:\n");
            print_indent(f, indent);
            fprintf(f, "Target:\n");
            ast_fprint(node->Index.target, f, indent + 1);
            print_indent(f, indent);
            fprintf(f, "Index:\n");
            ast_fprint(node->Index.index, f, indent + 1);
        break;
        case AST_ASSIGN_INDEX:
            fprintf(f, "Index Assignment:\n");
            print_indent(f, indent);
            fprintf(f, "Target:\n");
            ast_fprint(node->AssignIndex.target, f, indent + 1);
            print_indent(f, indent);
            fprintf(f, "Index:\n");
            ast_fprint(node->AssignIndex.index, f, indent + 1);
            print_indent(f, indent);
            fprintf(f, "Value:\n");
            ast_fprint(node->AssignIndex.value, f, indent + 1);
        break;
        case AST_FUNCDEF:
            fprintf(f, "Function Definition: %s\n", node->FuncDef.name);
            print_indent(f, indent);
            fprintf(f, "Arguments:\n");
            for (int i = 0; i < node->FuncDef.argc; i++) {
                print_indent(f, indent + 1);
                fprintf(f, "- %s\n", node->FuncDef.args[i]);
            }
            print_indent(f, indent);
            fprintf(f, "Body:\n");
            ast_fprint(node->FuncDef.body, f, indent + 1);
        break;
        case AST_CALL:
            fprintf(f, "Function Call: %s\n", node->Call.name);
            print_indent(f, indent);
            fprintf(f, "Arguments:\n");
            for (int i = 0; i < node->Call.argc; i++) {
                ast_fprint(node->Call.args[i], f, indent + 1);
            }
        break;
        case AST_RETURN:
            fprintf(f, "Return:\n");
            ast_fprint(node->Return.value, f, indent + 1);
        break;
        case AST_BREAK:
            fprintf(f, "Break\n");
        break;
        case AST_CONTINUE:
            fprintf(f, "Continue\n");
        break;
        case AST_CLASSDEF:
            fprintf(f, "Class Definition: %s\n", node->ClassDef.name);
            if (node->ClassDef.parent) {
                print_indent(f, indent);
                fprintf(f, "Parent Class: %s\n", node->ClassDef.parent);
            }
            print_indent(f, indent);
            fprintf(f, "Methods:\n");
            for (int i = 0; i < node->ClassDef.method_count; i++) {
                ast_fprint(node->ClassDef.methods[i], f, indent + 1);
            }
        break;
        case AST_METHOD_CALL:
            fprintf(f, "Method Call: %s\n", node->MethodCall.method_name);
            print_indent(f, indent);
            fprintf(f, "Object:\n");
            ast_fprint(node->MethodCall.object, f, indent + 1);
            print_indent(f, indent);
            fprintf(f, "Arguments:\n");
            for (int i = 0; i < node->MethodCall.argc; i++) {
                ast_fprint(node->MethodCall.args[i], f, indent + 1);
            }
        break;
        case AST_ATTR_ACCESS:
            fprintf(f, "Attribute Access: %s\n", node->AttrAccess.attr_name);
            print_indent(f, indent);
            fprintf(f, "Object:\n");
            ast_fprint(node->AttrAccess.object, f, indent + 1);
        break;
        case AST_ATTR_ASSIGN:
            fprintf(f, "Attribute Assignment: %s\n", node->AttrAssign.attr_name);
            print_indent(f, indent);
            fprintf(f, "Object:\n");
            ast_fprint(node->AttrAssign.object, f, indent + 1);
            print_indent(f, indent);
            fprintf(f, "Value:\n");
            ast_fprint(node->AttrAssign.value, f, indent + 1);
        break;
        case AST_IMPORT:
            fprintf(f, "Import: %s\n", node->Import.module_name);
        break;

        case AST_ELSE:
            // Handled in AST_IF
        break;
        default:
            fprintf(f, "Unknown AST node type: %d\n", node->type);
        break;
    }
}

void ast_dump(Ast* node, const char* filename) {
    if (node == NULL) return;
    FILE* f = fopen(filename, "w");
    if (!f) {
        printf("Failed to open file for AST dump: %s\n", filename);
        return;
    }
    
    ast_fprint(node, f, 1);
    
    fclose(f);
}

void ast_free(Ast* node) {
    if (node == NULL) return;

    switch (node->type)
    {
        case AST_NUMBER:
        case AST_FLOAT:
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
        case AST_SET:
            for (int i = 0; i < node->Set.count; i++) {
                ast_free(node->Set.elements[i]);
            }
            free(node->Set.elements);
        break;
        case AST_TUPLE:
            for (int i = 0; i < node->Tuple.count; i++) {
                ast_free(node->Tuple.elements[i]);
            }
            free(node->Tuple.elements);
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