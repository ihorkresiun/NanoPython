#include "bytecode.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static void bytecode_init(Bytecode* bytecode) {
    bytecode->instructions = NULL;
    bytecode->count = 0;
    bytecode->capacity = 0;
    bytecode->constants = NULL;
    bytecode->const_count = 0;
    bytecode->capacity = 0;
}

static int serialize_value(char* data, Value val) {
    data[0] = val.type;
    
    switch(val.type) {
        case VAL_NONE:
        case VAL_BOOL:
        case VAL_INT:
        case VAL_FLOAT:
            memcpy(data + 1, &val.as, sizeof(val.as));
            return 1 + sizeof(val.as);
            break;
            
        case VAL_OBJ: {
            Obj* obj = val.as.object;
            memcpy(data + 1, &obj->type, sizeof(ObjectType));
            
            switch(obj->type) {
                case OBJ_STRING: {
                    ObjString* str = (ObjString*)obj;
                    memcpy(data + 1 + sizeof(ObjectType), &str->length, sizeof(int));
                    memcpy(data + 1 + sizeof(ObjectType) + sizeof(int), str->chars, str->length);
                    return 1 + sizeof(ObjectType) + sizeof(int) + str->length;
                }
                
                // No other object types compiled in constants for now
            }
            break;
        }
    }

    return 0;
}

static int deserialize_value(char* data, Value* val) {
    val->type = data[0];
    
    switch(val->type) {
        case VAL_NONE:
        case VAL_BOOL:
        case VAL_INT:
        case VAL_FLOAT:
            memcpy(&val->as, data + 1, sizeof(val->as));
            return 1 + sizeof(val->as);
            break;
            
        case VAL_OBJ: {
            ObjectType obj_type;
            memcpy(&obj_type, data + 1, sizeof(ObjectType));
            
            switch(obj_type) {
                case OBJ_STRING: {
                    int length;
                    memcpy(&length, data + 1 + sizeof(ObjectType), sizeof(int));
                    char* chars = malloc(length + 1);
                    memcpy(chars, data + 1 + sizeof(ObjectType) + sizeof(int), length);
                    chars[length] = '\0';
                    
                    ObjString* str = malloc(sizeof(ObjString));
                    str->obj.type = OBJ_STRING;
                    str->length = length;
                    str->chars = chars;
                    
                    val->as.object = (Obj*)str;
                    return 1 + sizeof(ObjectType) + sizeof(int) + length;
                    break;
                }
                
                // No other object types compiled in constants for now
            }
            break;
        }
    }
}

Bytecode* bytecode_deserialize(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        printf("Cannot open file '%s'\n", filename);
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    char* bytecode_data = malloc(file_size);
    fread(bytecode_data, 1, file_size, file);
    fclose(file);

    Bytecode* bytecode = malloc(sizeof(Bytecode));
    bytecode_init(bytecode);

    int offset = 0;
    int instruction_count = *(int*)(bytecode_data + offset);
    offset += sizeof(int);
    int constant_count = *(int*)(bytecode_data + offset);
    offset += sizeof(int);

    bytecode->count = instruction_count;
    bytecode->capacity = instruction_count;
    bytecode->instructions = malloc(sizeof(Instruction) * instruction_count);
    memcpy(bytecode->instructions, bytecode_data + offset, sizeof(Instruction) * instruction_count);

    offset += sizeof(Instruction) * instruction_count;

    for (int i = 0; i < constant_count; i++) {
        Value val;
        int size = deserialize_value(bytecode_data + offset, &val);
        bytecode->constants = realloc(bytecode->constants, sizeof(Value) * (i + 1));
        bytecode->constants[i] = val;
        offset += size;
    }

    bytecode->const_count = constant_count;
    bytecode->capacity = constant_count;

    return bytecode;
}

int bytecode_serialize(Bytecode* bytecode, const char* filename) {
    int data_size = sizeof(int) * 2 + sizeof(Instruction) * bytecode->count + sizeof(Value) * bytecode->const_count;
    char* data = malloc(data_size);
    int offset = 0;

    *(int*)(data + offset) = bytecode->count;
    offset += sizeof(int);
    *(int*)(data + offset) = bytecode->const_count;
    offset += sizeof(int);

    memcpy(data + offset, bytecode->instructions, sizeof(Instruction) * bytecode->count);
    offset += sizeof(Instruction) * bytecode->count;


    for (int i = 0; i < bytecode->const_count; i++) {
        offset += serialize_value(data + offset, bytecode->constants[i]);
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        free(data);
        return 0; // Failed to open file
    }
    fwrite(data, 1, data_size, file);
    fclose(file);
    free(data);
    return 1; // Success
}

int bytecode_disasm(Bytecode* bytecode, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Failed to open file for disassembly: %s\n", filename);
        return 0; // Failed to open file
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
                    }
                }
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
    return 1; // Success
}