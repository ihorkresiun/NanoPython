#include "ast.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"


Ast* ast_new_number(double value) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }
    node->type = AST_NUMBER;
    node->Number.value = value;
    return node;
}

Ast* ast_new_unary(TokenType type, Ast* value) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }
    node->type = AST_UNARY;
    node->Unary.value = value;
    node->Unary.op = type;
    return node;
}

Ast* ast_new_expr(TokenType type, Ast* left, Ast* right) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }
    node->type = AST_BINARY;
    node->Binary.left = left;
    node->Binary.right = right;
    node->Binary.op = type;
    return node;
}

Ast* ast_new_var(const char* name) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }
    node->type = AST_VAR;
    node->Variable.name = strdup(name);
    return node;
}

Ast* ast_new_assign(const char* name, Ast* value) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }
    node->type = AST_ASSIGN;
    node->Assign.name = strdup(name);
    node->Assign.value = value;
    return node;
}

Ast* ast_new_block(Ast** statements, int count) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) { 
        printf("Out of memory!\node"); 
        exit(1); 
    }
    node->type = AST_BLOCK;
    node->Block.statements = statements;
    node->Block.count = count;
    return node;
}

Ast* ast_new_if(Ast* condition, Ast* then_block, Ast* else_block) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }

    node->type = AST_IF;
    node->If.condition = condition;
    node->If.then_branch = then_block;
    node->If.else_branch = else_block;

    return node;
}

Ast* ast_new_while(Ast* condition, Ast* body) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }

    node->type = AST_WHILE;
    node->While.condition = condition;
    node->While.body = body;

    return node;
}

Ast* ast_new_funcdef(const char* name, char** args, int argc, Ast* body) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }

    node->type = AST_FUNCDEF;
    node->FuncDef.name = strdup(name);
    node->FuncDef.args = args;
    node->FuncDef.argc = argc;
    node->FuncDef.body = body;

    return node;
}

Ast* ast_new_call(const char* name, Ast** args, int argc) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }

    node->type = AST_CALL;
    node->Call.name = strdup(name);
    node->Call.args = args;
    node->Call.argc = argc;

    return node;
}

Ast* ast_new_return(Ast* value) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }

    node->type = AST_RETURN;
    node->Return.value = value;

    return node;
}

Ast* ast_new_print(Ast* expr) {
    Ast* node = malloc(sizeof(Ast));
    if (!node) {
        printf("Out of memory!\n");
        exit(1);
    }

    node->type = AST_PRINT;
    node->Print.expr = expr;

    return node;
}

void ast_free(Ast* node) {
    if (node == NULL) return;

    if (node->type == AST_BINARY) {
        ast_free(node->Binary.left);
        ast_free(node->Binary.right);
    }
    if (node->type == AST_ASSIGN) {
        free(node->Assign.name);
        ast_free(node->Assign.value);
    }
    if (node->type == AST_VAR) {
        free(node->Variable.name);
    }
    if (node->type == AST_BLOCK) {
        for (int i = 0; i < node->Block.count; i++) {
            ast_free(node->Block.statements[i]);
        }
        free(node->Block.statements);
    }
    if (node->type == AST_IF) {
        ast_free(node->If.condition);
        ast_free(node->If.then_branch);
        ast_free(node->If.else_branch);
    }
    if (node->type == AST_WHILE) {
        ast_free(node->While.condition);
        ast_free(node->While.body);
    }

    free(node);
}