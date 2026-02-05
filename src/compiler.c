#include "compiler.h"

#include "lexer.h"
#include "parser.h"
#include "intern_string.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

void compiler_init(Compiler* compiler) {
    const int code_cap = 1024;
    const int const_cap = 256;
    compiler->bytecode = malloc(sizeof(Bytecode));
    compiler->bytecode->instructions = malloc(sizeof(Instruction) * code_cap);
    compiler->bytecode->count = 0;
    compiler->bytecode->capacity = code_cap;
    compiler->bytecode->constants = malloc(sizeof(Value) * const_cap);
    compiler->bytecode->const_count = 0;
    compiler->loop_count = 0;
    hash_init(&compiler->imported_modules, 16);
}

static void emit(Compiler* compiler, Opcode op, int arg) {
    if (compiler->bytecode->count >= compiler->bytecode->capacity) {
        compiler->bytecode->capacity *= 2;
        compiler->bytecode->instructions = realloc(
            compiler->bytecode->instructions,
            sizeof(Instruction) * compiler->bytecode->capacity
        );
    }

    compiler->bytecode->instructions[compiler->bytecode->count++] = (Instruction){op, arg};
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

    // Check for existing constant
    for (int i = 0; i < bytecode->const_count; i++) {
        Value constant = bytecode->constants[i];
        if (value_equals(&constant, &value)) {
            return i;
        }
    }

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
            emit(compiler, OP_STORE, idx);
        }
        break;

        case AST_VAR: {
            int idx = add_constant(compiler, make_string(node->Variable.name));
            emit(compiler, OP_LOAD, idx);
        }
        break;

        case AST_BLOCK: {
            for (int i = 0; i < node->Block.count; i++) {
                compile_node(compiler, node->Block.statements[i]);
            }
        }
        break;

        case AST_FUNCDEF: {
            // 1. Put function object in constants
            // 2. Store function object in global scope
            // 3  Jump over function body
            // 4. Function body starts here
            int fn_addr = compiler->bytecode->count + 3;

            ObjFunction* fn = malloc(sizeof(ObjFunction));
            fn->addr = fn_addr;
            fn->name = strdup(node->FuncDef.name);
            fn->param_count = node->FuncDef.argc;
            
            // Deep copy parameters
            fn->params = malloc(sizeof(char*) * fn->param_count);
            for (int i = 0; i < fn->param_count; i++) {
                fn->params[i] = strdup(node->FuncDef.args[i]);
            }
            
            fn->scope = NULL; // Closure scope will be set during execution

            Obj * obj_fn = (Obj*)fn;
            obj_fn->type = OBJ_FUNCTION;
            Value v = {.type = VAL_OBJ, .as.object = obj_fn};
            int fn_idx = add_constant(compiler, v);
            emit(compiler, OP_CONST, fn_idx);

            int fn_idx_name = add_constant(compiler, make_string(node->FuncDef.name));
            emit(compiler, OP_STORE, fn_idx_name);

            int jump_over_func = emit_jump(compiler, OP_JUMP);
            
            compile_node(compiler, node->FuncDef.body);

            // Ensure function ends with a return
            emit(compiler, OP_CONST, add_constant(compiler, (Value){.type = VAL_NONE}));
            emit(compiler, OP_RET, 0);

            patch_jump(compiler, jump_over_func, compiler->bytecode->count);
        }
        break;

        case AST_CALL: {
            for (int i = 0; i < node->Call.argc; i++) {
                compile_node(compiler, node->Call.args[i]);
            }
            int fn_idx_name = add_constant(compiler, make_string(node->Call.name));
            emit(compiler, OP_LOAD, fn_idx_name);
            emit(compiler, OP_CALL, node->Call.argc);
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

        case AST_LIST: {
            for (int i = 0; i < node->List.count; i++) {
                compile_node(compiler, node->List.elements[i]);
            }
            emit(compiler, OP_MAKE_LIST, node->List.count);
        }
        break;

        case AST_TUPLE: {
            for (int i = 0; i < node->Tuple.count; i++) {
                compile_node(compiler, node->Tuple.elements[i]);
            }
            emit(compiler, OP_MAKE_TUPLE, node->Tuple.count);
        }
        break;

        case AST_SET: {
            for (int i = 0; i < node->Set.count; i++) {
                compile_node(compiler, node->Set.elements[i]);
            }
            emit(compiler, OP_MAKE_SET, node->Set.count);
        }
        break;

        case AST_DICT: {
            for (int i = 0; i < node->Dict.count; i++) {
                compile_node(compiler, node->Dict.keys[i]);
                compile_node(compiler, node->Dict.values[i]);
            }
            // Call native function to create dict
            int fn_idx_name = add_constant(compiler, make_string("native_make_dict"));
            emit(compiler, OP_LOAD, fn_idx_name);
            emit(compiler, OP_CALL, node->Dict.count);
        }
        break;

        case AST_INDEX: {
            compile_node(compiler, node->Index.target);
            compile_node(compiler, node->Index.index);
            emit(compiler, OP_IDX_GET, 0);}
        break;

        case AST_ASSIGN_INDEX: {
            compile_node(compiler, node->AssignIndex.target);
            compile_node(compiler, node->AssignIndex.index);
            compile_node(compiler, node->AssignIndex.value);
            emit(compiler, OP_IDX_SET, 0);
        }
        break;

        case AST_CLASSDEF: {
            // Create class object with name and parent
            int class_name_idx = add_constant(compiler, make_string(node->ClassDef.name));
            
            // Load parent class if specified
            if (node->ClassDef.parent) {
                int parent_idx = add_constant(compiler, make_string(node->ClassDef.parent));
                emit(compiler, OP_LOAD, parent_idx);
            } else {
                // No parent, push None
                int none_idx = add_constant(compiler, make_none());
                emit(compiler, OP_CONST, none_idx);
            }
            
            // Create the class object - it's now on the stack
            emit(compiler, OP_MAKE_CLASS, class_name_idx);
            
            // Store the class so we can reload it
            emit(compiler, OP_STORE, class_name_idx);
            
            // For each method: load class, push method, set attribute
            for (int i = 0; i < node->ClassDef.method_count; i++) {
                Ast* method = node->ClassDef.methods[i];
                
                // Load the class
                emit(compiler, OP_LOAD, class_name_idx);
                
                // Compile the method as a function
                int method_addr = compiler->bytecode->count + 2;
                
                ObjFunction* fn = malloc(sizeof(ObjFunction));
                fn->addr = method_addr;
                fn->name = strdup(method->FuncDef.name);
                fn->param_count = method->FuncDef.argc;
                
                // Deep copy parameters
                fn->params = malloc(sizeof(char*) * fn->param_count);
                for (int i = 0; i < fn->param_count; i++) {
                    fn->params[i] = strdup(method->FuncDef.args[i]);
                }
                
                fn->scope = NULL;
                fn->obj.type = OBJ_FUNCTION;
                
                Value v = {.type = VAL_OBJ, .as.object = (Obj*)fn};
                int fn_idx = add_constant(compiler, v);
                emit(compiler, OP_CONST, fn_idx);
                
                int jump_over_method = emit_jump(compiler, OP_JUMP);
                
                // Compile method body
                compile_node(compiler, method->FuncDef.body);
                emit(compiler, OP_CONST, add_constant(compiler, make_none()));
                emit(compiler, OP_RET, 0);
                
                patch_jump(compiler, jump_over_method, compiler->bytecode->count);
                
                // Stack: [class, method_function]
                // Set method on class (this pops both)
                int method_name_idx = add_constant(compiler, make_string(method->FuncDef.name));
                emit(compiler, OP_SET_ATTR, method_name_idx);
            }
        }
        break;

        case AST_METHOD_CALL: {
            // Push the object (this will become 'self')
            compile_node(compiler, node->MethodCall.object);
            
            // Compile arguments
            for (int i = 0; i < node->MethodCall.argc; i++) {
                compile_node(compiler, node->MethodCall.args[i]);
            }
            
            // Method call: stack = [object, arg1, arg2, ...]
            // First operand: method name index
            // Second operand: argc (NOT including self)
            int method_name_idx = add_constant(compiler, make_string(node->MethodCall.method_name));
            emit(compiler, OP_CALL_METHOD, method_name_idx);
            emit(compiler, OP_NOP, node->MethodCall.argc); // Store argc for method call
        }
        break;

        case AST_ATTR_ACCESS: {
            compile_node(compiler, node->AttrAccess.object);
            int attr_name_idx = add_constant(compiler, make_string(node->AttrAccess.attr_name));
            emit(compiler, OP_GET_ATTR, attr_name_idx);
        }
        break;

        case AST_ATTR_ASSIGN: {
            compile_node(compiler, node->AttrAssign.object);
            compile_node(compiler, node->AttrAssign.value);
            int attr_name_idx = add_constant(compiler, make_string(node->AttrAssign.attr_name));
            emit(compiler, OP_SET_ATTR, attr_name_idx);
        }
        break;

        case AST_IMPORT: {
            // Check if module already imported
            ObjString* module_name = malloc(sizeof(ObjString));
            module_name->obj.type = OBJ_STRING;
            module_name->chars = strdup(node->Import.module_name);
            module_name->length = strlen(node->Import.module_name);
            
            Value cached;
            if (hash_get(&compiler->imported_modules, module_name, &cached)) {
                // Already imported, skip
                free(module_name->chars);
                free(module_name);
                break;
            }
            
            // Compile-time import: load, parse, and compile the module
            char filename[256];
            snprintf(filename, sizeof(filename), "%s.py", node->Import.module_name);
            
            // Read the module file
            FILE* file = fopen(filename, "r");
            if (!file) {
                printf("Cannot import module '%s': file '%s' not found\n", 
                       node->Import.module_name, filename);
                exit(1);
            }
            
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            
            char* source = malloc(file_size + 1);
            fread(source, 1, file_size, file);
            source[file_size] = '\0';
            fclose(file);
            
            // Parse the module
            Lexer* lexer = malloc(sizeof(Lexer));
            lexer_init(lexer, source);
            
            Parser parser;
            parser.lexer = lexer;
            parser_init(&parser, source);
            Ast* module_ast = parse_program(&parser);
            
            // Mark as imported
            hash_set(&compiler->imported_modules, module_name, make_number_int(1));
            
            // Compile the module into the current bytecode
            // (Just compile its statements, don't add HALT)
            if (module_ast->type == AST_BLOCK) {
                for (int i = 0; i < module_ast->Block.count; i++) {
                    compile_node(compiler, module_ast->Block.statements[i]);
                }
            } else {
                compile_node(compiler, module_ast);
            }
            
            // Cleanup
            free(lexer);
            free(source);
            ast_free(module_ast);
        }
        break;

        default:
            printf("Unsupported AST node in compiler: %d\n", node->type);
            exit(1);
            break;
    }
}

Bytecode* compile(Compiler* compiler, Ast* node) 
{
    compile_node(compiler, node);
    emit(compiler, OP_HALT, 0);
    return compiler->bytecode;
}