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

// Emit a jump instruction and return its position for later patching
static int emit_jump(Compiler* compiler, Opcode op) {
    emit(compiler, op, 0);
    return compiler->bytecode->count - 1;
}

// Patch a previously emitted jump instruction to jump to the current bytecode position
static void patch_jump(Compiler* compiler, int jump_pos) {
    int jump_target = compiler->bytecode->count;
    compiler->bytecode->instructions[jump_pos].operand = jump_target;
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
                case TOKEN_EQ:
                    emit(compiler, OP_EQ, 0);
                    break;
                case TOKEN_LT:
                    emit(compiler, OP_LT, 0);
                    break;
                case TOKEN_GT:
                    emit(compiler, OP_GT, 0);
                    break;
                default:
                    printf("Unsupported binary operator in compiler: %d\n", node->Binary.op);
                    exit(1);
                break;
            }
        }
        break;

        case AST_IF: {
            compile_node(compiler, node->If.condition);
            int jump_if_false = emit_jump(compiler, OP_JUMP_IF_ZERO);

            compile_node(compiler, node->If.then_branch);
            int jump_end = emit_jump(compiler, OP_JUMP);

            patch_jump(compiler, jump_if_false);
            if (node->If.else_branch) {
                compile_node(compiler, node->If.else_branch);
            }
            patch_jump(compiler, jump_end);
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