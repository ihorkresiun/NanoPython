#include "disasm.h"

#include "stdio.h"

void store_disasm(Bytecode* bytecode, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Failed to open file for disassembly: %s\n", filename);
        return;
    }

    for (int i = 0; i < bytecode->count; i++) {
        Instruction instr = bytecode->instructions[i];
        fprintf(file, "%04d: ", i);
        switch (instr.opcode) {
            case OP_NOP:          fprintf(file, "NOP\n"); break;
            case OP_LOAD:        fprintf(file, "LOAD %d\n", instr.operand); break;
            case OP_STORE:       fprintf(file, "STORE %d\n", instr.operand); break;
            case OP_ADD:         fprintf(file, "ADD\n"); break;
            case OP_SUB:         fprintf(file, "SUB\n"); break;
            case OP_MUL:         fprintf(file, "MUL\n"); break;
            case OP_DIV:         fprintf(file, "DIV\n"); break;
            case OP_EQ:          fprintf(file, "EQ\n"); break;
            case OP_LT:          fprintf(file, "LT\n"); break;
            case OP_GT:          fprintf(file, "GT\n"); break;
            case OP_JUMP:        fprintf(file, "JUMP %d\n", instr.operand); break;
            case OP_JUMP_IF_ZERO: fprintf(file, "JUMP_IF_ZERO %d\n", instr.operand); break;
            case OP_CONST:      {
                Value constant = bytecode->constants[instr.operand];
                if (constant.type == VAL_INT) {
                    fprintf(file, "CONST %d (INT)\n", (int)constant.value.i);
                } else if (constant.type == VAL_FLOAT) {
                    fprintf(file, "CONST %f (FLOAT)\n", constant.value.f);
                } else if (constant.type == VAL_STRING) {
                    fprintf(file, "CONST \"%s\" (STRING)\n", constant.value.string);
                } else {
                    fprintf(file, "CONST (TYPE %d)\n", constant.type);
                }
                break;
            }
            case OP_POP:         fprintf(file, "POP\n"); break;
            case OP_PRINT:      fprintf(file, "PRINT\n"); break;
            case OP_STORE_GLOBAL: {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_STRING) {
                    fprintf(file, "STORE_GLOBAL %s\n", name.value.string);
                } else {
                    fprintf(file, "STORE_GLOBAL %d\n", instr.operand);
                }
                break;
            }
            case OP_LOAD_GLOBAL: {
                Value name = bytecode->constants[instr.operand];
                if (name.type == VAL_STRING) {
                    fprintf(file, "LOAD_GLOBAL %s\n", name.value.string);
                } else {
                    fprintf(file, "LOAD_GLOBAL %d\n", instr.operand);
                }
                break;
            }
            case OP_HALT:        fprintf(file, "HALT\n"); break;
            default:             fprintf(file, "UNKNOWN OPCODE %d\n", instr.opcode); break;
        }
    }

    fclose(file);
}