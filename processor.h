#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <sys\stat.h>
#include <assert.h>
#include <stdlib.h>
#include "stack.h"

enum Cmds
{
    PUSH = 1,
    POP = 2,
    ADD = 3,
    SUB = 4,
    MUL = 5,
    DIV = 6,
    OUT = 7,
    IN = 8,
    HLT = -1
};

enum Reg
{
    RAX = 1,
    RBX = 2,
    RCX = 3,
    RDX = 4
};

enum Args
{
    REG = 1 << 6,
    NUM = 1 << 5
};

enum Proc_error
{
    PROC_NO_ERROR = 0,
    PROC_ERROR_FOPEN = 1,
    PROC_ERROR_CMDS = 2,
    PROC_ERROR_STAT = 3,
    PROC_ERROR_FSEEK = 4,
    PROC_ERROR_ARGC = 5,
    PROC_NULL_PTR_LOG = 6,
    PROC_NULL_PTR_FILE = 7,
    PROC_CALLOC_FAIL = 8,
    PROC_STACK_POP_ERROR = 9,
    PROC_STACK_PUSH_ERROR = 10,
};

enum Proc_error calculations             (struct Stack* stk, char* buffer, struct stat statbuf);
void            proc_print_error         (enum Proc_error error);
enum Proc_error check_argc               (int argc);
enum Proc_error processing_file          (const char* NAME, char** buffer, FILE** code_file, struct stat* statbuf);
enum Proc_error open_file_and_fill_stat  (const char *NAME, FILE** file, struct stat* statbuf);
enum Proc_error proc_set_log_file        (FILE* file);
const char*     proc_get_error           (enum Proc_error error);
// enum Proc_error char_to_double           (char* buffer, double* array);
#endif // PROCESSOR_H
