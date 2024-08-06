#ifndef PROCESSOR_HPP
#define PROCESSOR_HPP

#include "stack.hpp"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdlib.h>

enum Cmd
{
    NOT_CMD = 0,
    PUSH    = 1,
    POP     = 2,
    ADD     = 3,
    SUB     = 4,
    MUL     = 5,
    DIV     = 6,
    OUT     = 7,
    IN      = 8,
    JMP     = 9,
    CALL    = 10,
    RET     = 11,
    HLT     = 12
};

enum Reg
{
    NOT_REG = 0,
    RAX     = 1,
    RBX     = 2,
    RCX     = 3,
    RDX     = 4
};

enum Args
{
    REG = 1 << 6,
    NUM = 1 << 5
};

enum ProcError
{
    PROC_NO_ERROR              = 0,
    PROC_ERROR_FOPEN           = 1,
    PROC_ERROR_CMDS            = 2,
    PROC_ERROR_STAT            = 3,
    PROC_ERROR_FSEEK           = 4,
    PROC_ERROR_ARGC            = 5,
    PROC_ERROR_NULL_PTR_FILE   = 8,
    PROC_ERROR_CALLOC          = 9,
    PROC_ERROR_STACK           = 10,
    PROC_ERROR_SETVBUF         = 11,
    PROC_ERROR_NULL_PTR_STRUCT = 12,
    PROC_ERROR_PROC_FILE       = 13,
    PROC_ERROR_STK             = 14
};

struct Proc
{
    FILE* input_file;
    size_t file_size;
    char* input_buffer;
    Stack stk;
};

ProcError       calculations             (Proc* proc_struct);
void            proc_print_error         (ProcError error);
void            proc_set_log_file        (FILE* file);
const char*     proc_get_error           (ProcError error);
ProcError       proc_ctor                (Proc* proc_struct, const char* name_of_input_file);
ProcError       proc_dtor                (Proc* proc_struct);

#endif // PROCESSOR_HPP
