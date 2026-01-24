#include "compiler.h"

#include "stdlib.h"
#include "stdio.h"
#include "string.h"

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
static void patch_jump(Compiler* compiler, int jump_pos, int jump_target) {
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

static void push_loop(Compiler* compiler, int loop_start) {
    if (compiler->loop_count >= 16) {
        printf("Loop stack overflow\n");
        exit(1);
    }

    LoopContext* loop = &compiler->loop_stack[compiler->loop_count++];
    loop->loop_start = loop_start;
    loop->break_jumps = NULL;
    loop->break_count = 0;
    loop->break_capacity = 0;
}

void pop_loop(Compiler* compiler) {
    if (compiler->loop_count <= 0) {
        printf("Loop stack underflow\n");
        exit(1);
    }

    LoopContext* loop = &compiler->loop_stack[--compiler->loop_count];
    free(loop->break_jumps);
}

LoopContext* current_loop(Compiler* compiler) {
    if (compiler->loop_count <= 0) {
        return NULL;
    }
    return &compiler->loop_stack[compiler->loop_count - 1];
}

void add_break_jump(Compiler* compiler, int jump_pos) {
    LoopContext* loop = current_loop(compiler);
    if (!loop) {
        printf("No active loop for break statement\n");
        exit(1);
    }

    if (loop->break_count >= loop->break_capacity) {
        loop->break_capacity = loop->break_capacity == 0 ? 4 : loop->break_capacity * 2;
        loop->break_jumps = realloc(
            loop->break_jumps,
            sizeof(int) * loop->break_capacity
        );
    }

    loop->break_jumps[loop->break_count++] = jump_pos;
}

void patch_break_jumps(Compiler* compiler, int break_target) {
    LoopContext* loop = current_loop(compiler);
    if (!loop) {
        printf("No active loop for patching break statements\n");
        exit(1);
    }

    for (int i = 0; i < loop->break_count; i++) {
        patch_jump(compiler, loop->break_jumps[i], break_target);
    }
}

static void compile_node(Compiler* compiler, Ast* node) {
    switch (node->type) {
        case AST_NUMBER: {
            int idx = add_constant(compiler, make_number_int(node->NumberInt.value));
            emit(compiler, OP_CONST, idx);
        }
        break;

        case AST_FLOAT: {
            int idx = add_constant(compiler, make_number_float(node->NumberFloat.value));
            emit(compiler, OP_CONST, idx);
        }
        break;

        case AST_STRING: {
            int idx = add_constant(compiler, make_string(node->String.value));
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

            if (node->If.else_branch) {
                int jump_over_else = emit_jump(compiler, OP_JUMP);
                patch_jump(compiler, jump_if_false, compiler->bytecode->count);
                compile_node(compiler, node->If.else_branch);
                patch_jump(compiler, jump_over_else, compiler->bytecode->count);
            } else {
                patch_jump(compiler, jump_if_false, compiler->bytecode->count);
            }
        }
        break;

        case AST_WHILE: {
            int loop_start = compiler->bytecode->count;
            push_loop(compiler, loop_start);

            compile_node(compiler, node->While.condition);
            int exit_jump = emit_jump(compiler, OP_JUMP_IF_ZERO);

            compile_node(compiler, node->While.body);
            emit(compiler, OP_JUMP, loop_start);

            int loop_end = compiler->bytecode->count;
            patch_jump(compiler, exit_jump, loop_end);
            patch_break_jumps(compiler, loop_end);
            pop_loop(compiler);
        }
        break;

        case AST_BREAK: {
            int break_jump = emit_jump(compiler, OP_JUMP);
            add_break_jump(compiler, break_jump);
        }
        break;

        case AST_CONTINUE: {
            LoopContext* loop = current_loop(compiler);
            if (!loop) {
                printf("No active loop for continue statement\n");
                exit(1);
            }
            emit(compiler, OP_JUMP, loop->loop_start);
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

        case AST_FUNCDEF: {
            // 1. Put function object in constants
            // 2. Store function object in global scope
            // 3  Jump over function body
            // 4. Function body starts here
            int fn_addr = compiler->bytecode->count + 3; 

            Function* fn = malloc(sizeof(Function));
            fn->addr = fn_addr;
            fn->name = strdup(node->FuncDef.name);
            fn->params = node->FuncDef.args;
            fn->param_count = node->FuncDef.argc;
            fn->body = node->FuncDef.body;
            fn->scope = NULL; // Closure scope will be set during execution

            Value v = {.type = VAL_FUNCTION, .value.function = fn};
            int fn_idx = add_constant(compiler, v);
            emit(compiler, OP_CONST, fn_idx);

            int fn_idx_name = add_constant(compiler, make_string(node->FuncDef.name));
            emit(compiler, OP_STORE_GLOBAL, fn_idx_name);

            int jump_over_func = emit_jump(compiler, OP_JUMP);
            
            compile_node(compiler, node->FuncDef.body);

            patch_jump(compiler, jump_over_func, compiler->bytecode->count);
        }
        break;

        case AST_CALL: {
            for (int i = 0; i < node->Call.argc; i++) {
                compile_node(compiler, node->Call.args[i]);
            }
            int fn_idx_name = add_constant(compiler, make_string(node->Call.name));
            emit(compiler, OP_LOAD_GLOBAL, fn_idx_name);
            emit(compiler, OP_CALL, 0);
        }
        break;

        case AST_RETURN: {
            if (node->Return.value) {
                compile_node(compiler, node->Return.value);
            } else {
                int none_idx = add_constant(compiler, (Value){.type = VAL_NONE});
                emit(compiler, OP_CONST, none_idx);
            }
            emit(compiler, OP_RET, 0);
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