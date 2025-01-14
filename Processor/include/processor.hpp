#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "stack.hpp"
#include "array.hpp"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>

enum ProcCmd
{
    PROC_CMD_NONE = 0,
    PROC_CMD_PUSH = 1,
    PROC_CMD_POP  = 2,
    PROC_CMD_ADD  = 3,
    PROC_CMD_SUB  = 4,
    PROC_CMD_MUL  = 5,
    PROC_CMD_DIV  = 6,
    PROC_CMD_SQRT = 7,
    PROC_CMD_OUT  = 8,
    PROC_CMD_IN   = 9,
    PROC_CMD_JMP  = 10,
    PROC_CMD_JE   = 11,
    PROC_CMD_JA   = 12,
    PROC_CMD_JB   = 13,
    PROC_CMD_JEA  = 14,
    PROC_CMD_JEB  = 15,
    PROC_CMD_JNE  = 16,
    PROC_CMD_JNA  = 17,
    PROC_CMD_JNB  = 18,
    PROC_CMD_CALL = 19,
    PROC_CMD_RET  = 20,
    PROC_CMD_HLT  = 21
};

enum ProcReg
{
    PROC_RAX    = 0,
    PROC_RBX    = 1,
    PROC_RCX    = 2,
    PROC_RDX    = 3,
    NUM_OF_REGS = 4
};

enum ProcArgs
{
    ARG_RAM  = 1 << 7,
    ARG_REG  = 1 << 6,
    ARG_NUM  = 1 << 5
};

enum ProcError
{
    PROC_ERROR_OK              = 0,
    PROC_ERROR_FOPEN           = 1,
    PROC_ERROR_CMDS            = 2,
    PROC_ERROR_STAT            = 3,
    PROC_ERROR_FSEEK           = 4,
    PROC_ERROR_NULL_PTR_FILE   = 5,
    PROC_ERROR_CALLOC          = 6,
    PROC_ERROR_STACK           = 7,
    PROC_ERROR_SETVBUF         = 8,
    PROC_ERROR_NULL_PTR_STRUCT = 9,
    PROC_ERROR_PROC_FILE       = 10,
    PROC_ERROR_STK             = 11,
    PROC_ERROR_RAM             = 12,
    PROC_ERROR_ARG             = 13
};

struct Processor
{
    FILE* input_file;
    size_t file_size;
    char* input_buffer;
    Stack stk;
    double* reg;
    Arr ram;
};

ProcError   proc_calculations         (Processor* proc_struct);
ProcError   proc_cmd_push             (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_pop              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_jmp              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_je               (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_ja               (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_jb               (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_jea              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_jeb              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_jne              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_out              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_ret              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_call             (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_div              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_sqrt             (Processor *proc_struct, size_t *position);
ProcError   proc_cmd_mul              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_sub              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_add              (Processor* proc_struct, size_t* position);
ProcError   proc_cmd_in               (Processor* proc_struct, size_t* position);
StkError    proc_get_two_numbers      (Stack* stk, double* num1, double* num2);
void        proc_print_error          (ProcError error);
void        proc_set_log_file         (FILE* file);
const char* proc_get_error            (ProcError error);
ProcError   proc_ctor                 (Processor* proc_struct, const char* name_of_input_file);
ProcError   proc_dtor                 (Processor* proc_struct);
ProcError   proc_put_value            (Processor* proc_struct, size_t* position, double value);
ProcError   proc_put_in_reg           (Processor* proc_struct, size_t* position, double value);
ProcError   proc_put_in_ram           (Processor* proc_struct, size_t* position, double value);
ProcError   proc_put_in_ram_and_reg   (Processor* proc_struct, size_t* position, double value);
ProcError   proc_get_value            (Processor* proc_struct, size_t* position, double* value);
ProcError   proc_get_num              (Processor* proc_struct, size_t* position, double* value);
ProcError   proc_get_from_reg         (Processor* proc_struct, size_t* position, double* value);
ProcError   proc_get_from_ram         (Processor* proc_struct, size_t* position, double* value);
ProcError   proc_get_from_ram_and_reg (Processor* proc_struct, size_t* position, double* value);


#endif // PROCESSOR_HPP
