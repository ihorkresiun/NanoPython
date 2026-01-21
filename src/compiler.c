#include "compiler.h"

#include "stdlib.h"
#include "stdio.h"

static void emit(Compiler* compiler, Opcode op, int arg) {
    if (compiler->bytecode->count >= compiler->bytecode->capacity) {
        compiler->bytecode->capacity *= 2;
        compiler->bytecode->instructions = realloc(
            compiler->bytecode->instructions,
            sizeof(Instruction) * compiler->bytecode->capacity
        );
    }

    Bytecode* bytecode = compiler->bytecode;
    bytecode->instructions[bytecode->count++] = (Instruction){op, arg};
}

static int add_constant(Compiler* compiler, Value value) {
    Bytecode* bytecode = compiler->bytecode;
    if (bytecode->const_count >= bytecode->capacity) {
        bytecode->capacity *= 2;
        bytecode->constants = realloc(
            bytecode->constants,
            sizeof(Value) * bytecode->capacity
        );
    }

    bytecode->constants[bytecode->const_count] = value;
    return bytecode->const_count++;
}

static void compile_node(Compiler* compiler, Ast* node) {
    switch (node->type) {
        case AST_NUMBER: {
            int idx = add_constant(compiler, make_number(node->Number.value));
            emit(compiler, OP_CONST, idx);
        }
        break;

        case AST_BINARY: {
            compile_node(compiler, node->Binary.left);
            compile_node(compiler, node->Binary.right);
            switch (node->Binary.op) {
                case TOKEN_PLUS:
                    emit(compiler, OP_ADD, 0);
                    break;
                case TOKEN_MINUS:
                    emit(compiler, OP_SUB, 0);
                    break;
                // Add cases for other binary operations as needed
            }
        }
        break;

        case AST_PRINT: {
            compile_node(compiler, node->Print.expr);
            emit(compiler, OP_PRINT, 0);
        }
        break;

        // Handle other AST node types as needed

        default:
            printf("Unsupported AST node in compiler: %d\n", node->type);
            exit(1);
            break;
    }
}

void compile(Compiler* compiler, Ast* node) 
{
    compile_node(compiler, node);
    emit(compiler, OP_HALT, 0);
}