#include "disasm.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "stdint.h"

void store_disasm(Bytecode* bytecode, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Failed to open file for disassembly: %s\n", filename);
        return;
    }

    uint8_t* jump_addresses = malloc(sizeof(uint8_t) * bytecode->count);
    memset(jump_addresses, 0, sizeof(uint8_t) * bytecode->count);
    // Mark entry point
    jump_addresses[0] = 1;
    // Find jumps addresses
    for (int i = 0; i < bytecode->count; i++) {
        Instruction instr = bytecode->instructions[i];
        if (instr.opcode == OP_JUMP || instr.opcode == OP_JUMP_IF_ZERO) {
            jump_addresses[instr.operand] = 1;
        }
    }


    for (int i = 0; i < bytecode->count; i++) {
        Instruction instr = bytecode->instructions[i];
        if (jump_addresses[i]) {
            fprintf(file, "LABEL_%04d:\n", i);
        }
        fprintf(file, "\t%04d ", i);
        switch (instr.opcode) {
            case OP_NOP:         fprintf(file, "NOP\n"); break;
            case OP_ADD:         fprintf(file, "ADD\n"); break;
            case OP_SUB:         fprintf(file, "SUB\n"); break;
            case OP_MUL:         fprintf(file, "MUL\n"); break;
            case OP_DIV:         fprintf(file, "DIV\n"); break;
            case OP_EQ:          fprintf(file, "EQ\n"); break;
            case OP_LT:          fprintf(file, "LT\n"); break;
            case OP_GT:          fprintf(file, "GT\n"); break;
            case OP_JUMP:        fprintf(file, "JUMP LABEL_%04d\n", instr.operand); break;
            case OP_JUMP_IF_ZERO:fprintf(file, "JUMP_IF_ZERO LABEL_%04d\n", instr.operand); break;
            case OP_CONST:      {
                Value constant = bytecode->constants[instr.operand];
                if (constant.type == VAL_INT) {
                    fprintf(file, "CONST [%d]=(INT)%d\n", instr.operand, (int)constant.as.integer);
                } else if (constant.type == VAL_FLOAT) {
                    fprintf(file, "CONST [%d]=(FLT)%f\n", instr.operand, constant.as.floating);
                } else if (constant.type == VAL_BOOL) {
                    fprintf(file, "CONST [%d]=(BOOL)%s\n", instr.operand, constant.as.boolean ? "True" : "False");
                } else if (constant.type == VAL_NONE) {
                    fprintf(file, "CONST None\n");
                } else if (constant.type == VAL_OBJ) {
                    fprintf(file, "CONST [%d]=(OBJ->", instr.operand);
                    if (constant.as.object->type == OBJ_FUNCTION) {
                        fprintf(file, "Func)\n");
                    } else if (constant.as.object->type == OBJ_STRING) {
                        fprintf(file, "Str)\"%s\"\n", ((ObjString*)constant.as.object)->chars);
                    } else if (constant.as.object->type == OBJ_LIST) {
                        fprintf(file, "List)\n");
                    } else {
                        fprintf(file, "->Unknown Object Type %d\n", constant.as.object->type);
                    }                }
                break;
            }
            case OP_POP:        fprintf(file, "POP\n"); break;
            case OP_STORE: {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_OBJ && name.as.object->type == OBJ_STRING) {
                    fprintf(file, "STORE [%d]=(OBJ->Str)\"%s\"\n", instr.operand, ((ObjString*)name.as.object)->chars);
                } else {
                    fprintf(file, "STORE %d\n", instr.operand);
                }
                break;
            }
            case OP_LOAD: {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_OBJ && name.as.object->type == OBJ_STRING) {
                    fprintf(file, "LOAD [%d]=(OBJ->Str)\"%s\"\n", instr.operand, ((ObjString*)name.as.object)->chars);
                } else {
                    fprintf(file, "LOAD %d\n", instr.operand);
                }
                break;
            }
            case OP_HALT:        fprintf(file, "HALT\n"); break;
            case OP_CALL:        fprintf(file, "CALL %d\n", instr.operand); break;
            case OP_RET:         fprintf(file, "RET\n"); break;

            case OP_MAKE_LIST:   fprintf(file, "MAKE_LIST %d\n", instr.operand); break;
            case OP_MAKE_TUPLE:  fprintf(file, "MAKE_TUPLE %d\n", instr.operand); break;
            case OP_MAKE_SET:    fprintf(file, "MAKE_SET %d\n", instr.operand); break;

            case OP_IDX_GET:     fprintf(file, "INDEX_GET\n"); break;
            case OP_IDX_SET:     fprintf(file, "INDEX_SET\n"); break;
            
            case OP_MAKE_CLASS:  {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_OBJ && name.as.object->type == OBJ_STRING) {
                    fprintf(file, "MAKE_CLASS [%d]=(OBJ->Str)\"%s\"\n", instr.operand, ((ObjString*)name.as.object)->chars);
                } else {
                    fprintf(file, "MAKE_CLASS %d\n", instr.operand);
                }
                break;
            }
            case OP_MAKE_INSTANCE: fprintf(file, "MAKE_INSTANCE\n"); break;
            case OP_GET_ATTR:    {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_OBJ && name.as.object->type == OBJ_STRING) {
                    fprintf(file, "GET_ATTR [%d]=(OBJ->Str)\"%s\"\n", instr.operand, ((ObjString*)name.as.object)->chars);
                } else {
                    fprintf(file, "GET_ATTR %d\n", instr.operand);
                }
                break;
            }
            case OP_SET_ATTR:    {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_OBJ && name.as.object->type == OBJ_STRING) {
                    fprintf(file, "SET_ATTR [%d]=(OBJ->Str)\"%s\"\n", instr.operand, ((ObjString*)name.as.object)->chars);
                } else {
                    fprintf(file, "SET_ATTR %d\n", instr.operand);
                }
                break;
            }
            case OP_CALL_METHOD: {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_OBJ && name.as.object->type == OBJ_STRING) {
                    fprintf(file, "CALL_METHOD [%d]=(OBJ->Str)\"%s\"\n", instr.operand, ((ObjString*)name.as.object)->chars);
                } else {
                    fprintf(file, "CALL_METHOD %d\n", instr.operand);
                }
                break;
            }
            default:             fprintf(file, "UNKNOWN OPCODE %d\n", instr.opcode); break;
        }
    }

    fprintf(file, "\nConstants:\n");
    for (int i = 0; i < bytecode->const_count; i++) {
        Value constant = bytecode->constants[i];
        fprintf(file, "%04d: ", i);
        if (constant.type == VAL_INT) {
            fprintf(file, "INT %d\n", (int)constant.as.integer);
        } else if (constant.type == VAL_FLOAT) {
            fprintf(file, "FLOAT %f\n", constant.as.floating);
        } else if (constant.type == VAL_BOOL) {
            fprintf(file, "BOOL %s\n", constant.as.boolean ? "True" : "False");
        } else if (constant.type == VAL_NONE) {
            fprintf(file, "NONE\n");
        } else if (constant.type == VAL_OBJ) {
            fprintf(file, "OBJ ");;
            if (constant.as.object->type == OBJ_FUNCTION) {
                fprintf(file, "Function\n");
            } else if (constant.as.object->type == OBJ_STRING) {
                fprintf(file, "String: \"%s\"\n", ((ObjString*)constant.as.object)->chars);
            } else if (constant.as.object->type == OBJ_LIST) {
                fprintf(file, "List\n");
            } else {
                fprintf(file, "Unknown Object Type %d\n", constant.as.object->type);
            }
        } else {
            fprintf(file, "Unknown Constant Type %d\n", constant.type);
        }
    }

    fclose(file);
    free(jump_addresses);
}