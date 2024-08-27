#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include "void_stack.hpp"
#include "file_processing.hpp"

enum AsmError
{
    ASM_ERROR_OK              = 0,
    ASM_ERROR_NULL_PTR_FILE   = 1,
    ASM_ERROR_FOPEN           = 2,
    ASM_ERROR_NULL_PTR_LOG    = 3,
    ASM_ERROR_SSCANF          = 4,
    ASM_ERROR_FSEEK           = 5,
    ASM_ERROR_FTELL           = 6,
    ASM_ERROR_STAT            = 7,
    ASM_ERROR_STRCHR          = 8,
    ASM_ERROR_CALLOC          = 9,
    ASM_ERROR_NOT_CMD         = 10,
    ASM_ERROR_READ            = 11,
    ASM_ERROR_STK             = 12,
    ASM_ERROR_SETVBUF         = 13,
    ASM_ERROR_NULL_PTR_STRUCT = 14,
    ASM_ERROR_PROC_FILE       = 15,
    ASM_ERROR_FWRITE          = 16,
    ASM_ERROR_RAM             = 17,
    ASM_ERROR_STRTOD          = 18
};

enum AsmCmd
{
    ASM_CMD_NONE = 0,
    ASM_CMD_PUSH = 1,
    ASM_CMD_POP  = 2,
    ASM_CMD_ADD  = 3,
    ASM_CMD_SUB  = 4,
    ASM_CMD_MUL  = 5,
    ASM_CMD_DIV  = 6,
    ASM_CMD_OUT  = 7,
    ASM_CMD_IN   = 8,
    ASM_CMD_JMP  = 9,
    ASM_CMD_JE   = 10,
    ASM_CMD_JA   = 11,
    ASM_CMD_JB   = 12,
    ASM_CMD_JEA  = 13,
    ASM_CMD_JEB  = 14,
    ASM_CMD_JNE  = 15,
    ASM_CMD_JNA  = 16,
    ASM_CMD_JNB  = 17,
    ASM_CMD_CALL = 18,
    ASM_CMD_RET  = 19,
    ASM_CMD_HLT  = 20
};

enum AsmReg
{
    ASM_REG_NONE = -1,
    ASM_REG_RAX  = 0,
    ASM_REG_RBX  = 1,
    ASM_REG_RCX  = 2,
    ASM_REG_RDX  = 3
};

struct RegInfo
{
    AsmReg reg_enum;
    const char* name;
};

struct CmdInfo
{
    enum AsmCmd cmd_enum;
    const char* name;
    bool has_digit;
    bool has_reg;
    bool has_label;
    bool has_ram;
    size_t length;
};

enum AsmCmdMask
{
    ASM_MASK_RAM    = 1 << 7,
    ASM_MASK_REG    = 1 << 6,
    ASM_MASK_NUMBER = 1 << 5
};

struct AsmLabel
{
    const char* name;
    size_t len;
    size_t ip;
};

struct Assembler
{
    FILE* input_file;
    char** ptr_to_lines;
    size_t number_of_lines;
    char* output_buffer;
    FILE* output_file;
    Stack labels;
    double* ram;
};

void        asm_set_log_file      (FILE* file);
void        asm_print_error       (AsmError error);
const char* asm_get_error         (AsmError error);
AsmError    asm_make_assem_file   (Stack* labels, char** lines, size_t num_line, char* buffer, size_t* position);
AsmReg      asm_lookup_reg        (const char* line, size_t size);
AsmCmd      asm_lookup_command    (const char* line, size_t size);
size_t      asm_lookup_label      (const char* line, size_t size, Stack* labels);
bool        asm_try_command       (Stack* labels, const char* cur_line, char* buffer, size_t* position, AsmError* error);
bool        asm_try_label         (Stack* labels, const char* cur_line, size_t position, StkError* error);
bool        asm_try_command_label (Stack* labels, const char* cur_line, char* buffer, size_t* position);
bool        asm_try_command_digit (const char* cur_line, char* buffer, size_t* position, AsmError* error);
bool        asm_try_command_reg   (const char* cur_line, char* buffer, size_t* position);
bool        asm_try_command_ram   (Stack* labels, const char* cur_line, char* buffer, size_t* position, AsmError* error);
bool        asm_try_ram_digit     (const char** cur_line, char* buffer, size_t* position, AsmError* error);
bool        asm_try_ram_reg       (const char** cur_line, char* buffer, size_t* position);
bool        asm_try_ram_label     (Stack* labels, const char** cur_line, char* buffer, size_t* position);
AsmError    asm_ctor              (Assembler* asm_struct, const char* name_of_input_file, const char* name_of_output_file);
AsmError    asm_assembly          (Assembler* asm_struct);
AsmError    asm_dtor              (Assembler* asm_struct);
void        put_ip_in_buffer      (Stack* labels, size_t label_number, char* buffer, size_t position);

#endif // ASSEMBLER_HPP
