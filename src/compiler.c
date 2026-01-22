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
                case TOKEN_STAR:
                    emit(compiler, OP_MUL, 0);
                    break;
                case TOKEN_SLASH:
                    emit(compiler, OP_DIV, 0);
                    break;
                default:
                    printf("Unsupported binary operator in compiler: %d\n", node->Binary.op);
                    exit(1);
                break;
            }
        }
        break;

        case AST_ASSIGN: {
            compile_node(compiler, node->Assign.value);
            int idx = add_constant(compiler, make_string(node->Assign.name));
            emit(compiler, OP_STORE_GLOBAL, idx);
        }
        break;

        case AST_VAR: {
            int idx = add_constant(compiler, make_string(node->Variable.name));
            emit(compiler, OP_LOAD_GLOBAL, idx);
        }
        break;

        case AST_BLOCK: {
            for (int i = 0; i < node->Block.count; i++) {
                compile_node(compiler, node->Block.statements[i]);
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