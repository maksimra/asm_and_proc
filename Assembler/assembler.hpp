#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

enum AsmError
{
    ASM_NO_ERROR      = 0,
    ASM_NULL_PTR_FILE = 1,
    ASM_ERROR_FOPEN   = 2,
    ASM_NULL_PTR_LOG  = 3,
    ASM_ERROR_SSCANF  = 4,
    ASM_ERROR_FSEEK   = 5,
    ASM_ERROR_FTELL   = 6,
    ASM_ERROR_STAT    = 7,
    ASM_ERROR_STRCHR  = 8,
    ASM_CALLOC_FAIL   = 9,
    ASM_NOT_CMD       = 10,
    ASM_NO_MARK       = 11,
    ASM_ERROR_READ    = 12,
    ASM_ERROR_ARGC    = 13
};

enum Cmd
{
    PUSH = 1,
    POP  = 2,
    ADD  = 3,
    SUB  = 4,
    MUL  = 5,
    DIV  = 6,
    OUT  = 7,
    IN   = 8,
    JMP  = 9,
    CALL = 10,
    RET  = 11,
    HLT  = -1
};

enum Reg
{
    RAX = 1,
    RBX = 2,
    RCX = 3,
    RDX = 4
};

struct Regs
{
    const char* name;
    enum Reg number;
};

const Regs REG[] =
{
    {"RAX", RAX},
    {"RBX", RBX},
    {"RCX", RCX},
    {"RDX", RDX}
};

struct Cmds
{
    const char* name;
    enum Cmd number;
    bool has_arg;
    bool has_mark;
    size_t length;
};

const Cmds CMD[] =
{
    {"PUSH", PUSH, true,  false, 4},
    {"ADD",  ADD,  false, false, 3},
    {"SUB",  SUB,  false, false, 3},
    {"MUL",  MUL,  false, false, 3},
    {"DIV",  DIV,  false, false, 3},
    {"OUT",  OUT,  false, false, 3},
    {"IN",   IN,   false, false, 2},
    {"POP",  POP,  true,  false, 3},
    {"JMP",  JMP,  true,  true,  3},
    {"CALL", CALL, true,  true,  4},
    {"RET",  RET,  false, false, 3},
    {"HLT",  HLT,  false, false, 3}
};

enum Mask_arg
{
    MASK_RAM    = 1 << 7,
    MASK_REG    = 1 << 6,
    MASK_NUMBER = 1 << 5
};

struct Mark
{
    char* name;
    int len;
    size_t ip;
};


enum AsmError    my_fread                (size_t size, FILE *fp, char** buffer_ptr);
enum AsmError    open_file_and_fill_stat (const char *NAME, struct stat *statbuf, FILE** file);
enum AsmError    ptr_to_lines            (size_t size, size_t line_counter, char* buffer, char*** lines);
size_t           line_processing         (size_t size, char* buffer);
char**           process_file            (const char* NAME, enum AsmError* error, size_t* line_counter);
enum AsmError    asm_set_log_file        (FILE* file);
void             asm_print_error         (enum AsmError error);
const char*      asm_get_error           (enum AsmError error);
enum AsmError    assembly_file           (char** lines, size_t n_line, Mark* marks, int* n_marks);
void             skip_space              (char** line);
int              search_mark             (char* line, long size, Mark* marks, int n_marks);
int              search_reg              (char* line, long size);
int              search_cmd              (char* line, long size);
enum AsmError    allocate_mem            (Mark** marks, char** names_array);


#endif // ASSEMBLER_H
