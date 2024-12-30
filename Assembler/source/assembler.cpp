#include <errno.h>
#include <assert.h>
#include <algorithm>
#include <string.h>

#include "../include/assembler.hpp"
#include "../include/print_in_log.hpp"
#include "../include/void_stack.hpp"
#include "../include/skip_space.hpp"

static FILE *log_file = stderr;

const int MAX_SYMB = 20;

const int NUMBER_OF_LABELS = 10;

const size_t LABEL_IS_NEW = SIZE_MAX;

const size_t NUMBER_OF_ROUNDS = 2;

const size_t MAX_LEN_FOR_PATH_TO_FILE = 25;

const RegInfo ASM_REGS[] =
    {
        {ASM_REG_NONE},
        {ASM_REG_RAX, "rax"},
        {ASM_REG_RBX, "rbx"},
        {ASM_REG_RCX, "rcx"},
        {ASM_REG_RDX, "rdx"}};

const size_t NUMBER_OF_REGS = sizeof(ASM_REGS) / sizeof(ASM_REGS[0]);

const CmdInfo ASM_CMDS[] =
    {
        {ASM_CMD_NONE},
        {ASM_CMD_PUSH, "push", true,  true,  true,  true,  4},
        {ASM_CMD_POP,  "pop",  false, true,  false, true,  3},
        {ASM_CMD_ADD,  "add",  false, false, false, false, 3},
        {ASM_CMD_SUB,  "sub",  false, false, false, false, 3},
        {ASM_CMD_MUL,  "mul",  false, false, false, false, 3},
        {ASM_CMD_DIV,  "div",  false, false, false, false, 3},
        {ASM_CMD_SQRT, "sqrt", false, false, false, false, 4},
        {ASM_CMD_OUT,  "out",  false, false, false, false, 3},
        {ASM_CMD_IN,   "in",   false, false, false, false, 2},
        {ASM_CMD_JMP,  "jmp",  true,  true,  true,  true,  3},
        {ASM_CMD_JE,   "je",   true,  true,  true,  true,  2},
        {ASM_CMD_JA,   "ja",   true,  true,  true,  true,  2},
        {ASM_CMD_JB,   "jb",   true,  true,  true,  true,  2},
        {ASM_CMD_JEA,  "jea",  true,  true,  true,  true,  2},
        {ASM_CMD_JEB,  "jeb",  true,  true,  true,  true,  2},
        {ASM_CMD_JNE,  "jne",  true,  true,  true,  true,  2},
        {ASM_CMD_JNA,  "jna",  true,  true,  true,  true,  2},
        {ASM_CMD_JNB,  "jnb",  true,  true,  true,  true,  2},
        {ASM_CMD_CALL, "call", true,  false, true,  true,  4},
        {ASM_CMD_RET,  "ret",  false, false, false, false, 3},
        {ASM_CMD_HLT,  "hlt",  false, false, false, false, 3}
        //                       ^      ^      ^      ^
        //                       |      |      |      |
        // TYPE OF ARGS:       digit   reg   label   ram
};

const size_t NUMBER_OF_CMDS = sizeof(ASM_CMDS) / sizeof(ASM_CMDS[0]);

void asm_set_log_file(FILE *file)
{
    log_file = file;
}

AsmCmd asm_lookup_command(const char *line, size_t size)
{
    for (size_t n_cmd = 1; n_cmd < NUMBER_OF_CMDS; n_cmd++)
    {
        if (strncmp(line, ASM_CMDS[n_cmd].name, std::max(size, ASM_CMDS[n_cmd].length)) == 0)
        {
            return ASM_CMDS[n_cmd].cmd_enum;
        }
    }
    return ASM_CMD_NONE;
}

AsmReg asm_lookup_reg(const char *line, size_t size)
{
    for (size_t n_reg = 1; n_reg < NUMBER_OF_REGS; n_reg++)
    {
        if (strncmp(line, ASM_REGS[n_reg].name, size) == 0)
            return ASM_REGS[n_reg].reg_enum;
    }
    return ASM_REG_NONE;
}

size_t asm_lookup_label(const char *line, size_t size, Stack *labels)
{
    for (size_t label_number = 0; label_number < labels->size; label_number++)
    {
        if (strncmp(line,
                    ((AsmLabel *)labels->data + label_number)->name,
                    std::max(size, ((AsmLabel *)labels->data + label_number)->len)) == 0)
        {
            return label_number;
        }
    }
    return LABEL_IS_NEW;
}

AsmError asm_make_assem_file(Stack *labels, char **lines, size_t num_line, char *buffer, size_t *position)
{
    PRINT_BEGIN();
    assert(lines);

    AsmError asm_error = ASM_ERROR_OK;
    StkError stk_error = STK_NO_ERROR;

    for (size_t n_line = 0; n_line < num_line; n_line++)
    {
        const char *cur_line = lines[n_line];
        bool is_void_string = skip_space(&cur_line);
        if (is_void_string)
            continue;

        if ( // TODO: комментарии добавить
            asm_try_command(labels, cur_line, buffer, position, &asm_error) ||
            asm_try_label(labels, cur_line, *position, &stk_error))
        {
            if (asm_error)
            {
                return asm_error;
            }
            if (stk_error)
            {
                stk_print_error(stk_error);
                return ASM_ERROR_STK;
            }
        }
        else
        {
            return ASM_ERROR_NOT_CMD;
        }
    }

    PRINT_END();
    return asm_error;
}

bool asm_try_label(Stack *labels, const char *cur_line, size_t position, StkError *error)
{
    const char *colon = strchr(cur_line, ':');
    if (colon != NULL)
    {
        size_t label_number = asm_lookup_label(cur_line, (size_t)(colon - cur_line), labels);
        if (label_number != LABEL_IS_NEW)
        {
            return true;
        }
        else
        {
            AsmLabel new_label = {};
            new_label.name = cur_line;
            new_label.len = (size_t)(colon - cur_line);
            new_label.ip = position;

            *error = stack_push(labels, &new_label);
            if (*error)
                return false;

            return true;
        }
    }
    return false;
}

bool asm_try_command(Stack *labels, const char *cur_line, char *buffer, size_t *position, AsmError *error)
{
    size_t len_of_command = 0;
    if (isalpha(*cur_line))
    {
        do
        {
            len_of_command++;
        } while (isalpha(cur_line[len_of_command]) ||
                 cur_line[len_of_command] == '_');

        AsmCmd command = asm_lookup_command(cur_line, len_of_command);
        if (command != ASM_CMD_NONE)
        {
            cur_line += len_of_command;
            skip_space(&cur_line);

            buffer[*position] = (char)command;
            if (ASM_CMDS[(int)command].has_reg)
                if (asm_try_command_reg(cur_line, buffer, position))
                    return true;

            if (ASM_CMDS[(int)command].has_ram)
                if (asm_try_command_ram(labels, cur_line, buffer, position, error))
                    return true;

            if (ASM_CMDS[(int)command].has_digit)
                if (asm_try_command_digit(cur_line, buffer, position, error))
                    return true;

            if (ASM_CMDS[(int)command].has_label)
                if (asm_try_command_label(labels, cur_line, buffer, position))
                    return true;

            (*position)++;

            return true;
        }
    }
    return false;
}

bool asm_try_command_ram(Stack *labels, const char *cur_line, char *buffer, size_t *position, AsmError *error)
{
    if (*cur_line == '[')
    {
        cur_line++;

        if (asm_try_ram_digit(&cur_line, buffer, position, error) ||
            asm_try_ram_reg(&cur_line, buffer, position) ||
            asm_try_ram_label(labels, &cur_line, buffer, position))
        {
            if (*error)
                return false;

            if (*cur_line != ']')
            {
                *error = ASM_ERROR_RAM;
                return false;
            }
            return true;
        }
        else
        {
            *error = ASM_ERROR_RAM;
            return false;
        }
    }
    return false;
}

bool asm_try_ram_digit(const char **cur_line, char *buffer, size_t *position, AsmError *error)
{
    if (isdigit(**cur_line))
    {
        buffer[*position] |= (char)ASM_MASK_RAM;
        (*position)++;

        double value = strtod(*cur_line, NULL);
        if (errno == ERANGE)
        {
            *error = ASM_ERROR_STRTOD;
            return false;
        }

        memcpy(buffer + *position, &value, sizeof(double));
        *position += sizeof(double);

        bool has_point = false;
        do
        {
            (*cur_line)++;
            if (**cur_line == '.' && has_point == false)
            {
                (*cur_line)++;
                has_point = true;
            }
        } while (isdigit(**cur_line));

        return true;
    }
    return false;
}

bool asm_try_ram_reg(const char **cur_line, char *buffer, size_t *position)
{
    size_t len_of_reg = 0;
    if (isalpha(**cur_line))
    {
        len_of_reg++;
        while (isalpha((*cur_line)[len_of_reg]))
            len_of_reg++;

        AsmReg reg = asm_lookup_reg(*cur_line, len_of_reg);
        if (reg != ASM_REG_NONE)
        {
            (*cur_line) += len_of_reg;

            buffer[*position] |= (char)ASM_MASK_REG;
            buffer[*position] |= (char)ASM_MASK_RAM;
            (*position)++;
            buffer[*position] = (char)reg;
            (*position)++;

            return true;
        }
    }
    return false;
}

bool asm_try_ram_label(Stack *labels, const char **cur_line, char *buffer, size_t *position)
{
    size_t len_of_label = 0;
    if (isalpha(**cur_line))
    {
        len_of_label++;
        while (isalpha((*cur_line)[len_of_label]) || (*cur_line)[len_of_label] == '_')
            len_of_label++;

        buffer[*position] |= (char)ASM_MASK_RAM;
        (*position)++;

        size_t label_number = asm_lookup_label(*cur_line, len_of_label, labels);
        if (label_number != LABEL_IS_NEW)
        {
            put_ip_in_buffer(labels, label_number, buffer, *position);
        }
        else
        {
            double value = 0;
            memcpy(buffer + *position, &value, sizeof(double));
        }
        *position += sizeof(double);
        *cur_line += len_of_label;
        return true;
    }
    return false;
}

void put_ip_in_buffer(Stack *labels, size_t label_number, char *buffer, size_t position)
{
    double label_ip = (double)(((AsmLabel *)labels->data + label_number)->ip);
    memcpy(buffer + position, &label_ip, sizeof(double));
}

bool asm_try_command_label(Stack *labels, const char *cur_line, char *buffer, size_t *position)
{
    size_t len_of_label = 0;
    if (isalpha(*cur_line))
    {
        len_of_label++;
        while (isalpha(cur_line[len_of_label]) || cur_line[len_of_label] == '_')
            len_of_label++;

        buffer[*position] |= ASM_MASK_NUMBER;
        (*position)++;

        size_t label_number = asm_lookup_label(cur_line, len_of_label, labels);
        if (label_number != LABEL_IS_NEW)
        {
            put_ip_in_buffer(labels, label_number, buffer, *position);

            *position += sizeof(double);
        }
        else
        {
            double value = 0;
            memcpy(buffer + *position, &value, sizeof(double));
            *position += sizeof(double);
        }
        return true;
    }
    return false;
}

bool asm_try_command_digit(const char *cur_line, char *buffer, size_t *position, AsmError *error)
{
    if (isdigit(*cur_line) || *cur_line == '-')
    {
        double value = strtod(cur_line, NULL);
        if (errno == ERANGE)
        {
            *error = ASM_ERROR_STRTOD;
            return false;
        }

        buffer[*position] |= ASM_MASK_NUMBER;
        (*position)++;

        memcpy(buffer + *position, &value, sizeof(double));
        *position += sizeof(double);

        return true;
    }
    return false;
}

bool asm_try_command_reg(const char *cur_line, char *buffer, size_t *position)
{
    size_t len_of_reg = 0;
    if (isalpha(*cur_line))
    {
        len_of_reg++;
        while (isalpha(cur_line[len_of_reg]))
            len_of_reg++;

        AsmReg reg = asm_lookup_reg(cur_line, len_of_reg);
        if (reg != ASM_REG_NONE)
        {
            buffer[*position] |= ASM_MASK_REG;
            *position += sizeof(char);
            buffer[*position] = (char)reg;
            *position += sizeof(char);

            return true;
        }
    }
    return false;
}

AsmError asm_ctor(Assembler *asm_struct, const char *name_of_input_file, const char *name_of_output_file)
{
    assert(asm_struct);
    AsmError asm_error = ASM_ERROR_OK;
    ProcFileError proc_file_error = PROC_FILE_NO_ERROR;
    StkError stk_error = STK_NO_ERROR;
    if (asm_struct == NULL)
        return ASM_ERROR_NULL_PTR_STRUCT;

    char *temp_output_buffer = NULL;
    FILE *temp_file = fopen(name_of_input_file, "rb");
    if (temp_file == NULL)
        return ASM_ERROR_FOPEN;
    asm_struct->input_file = temp_file;

    temp_file = fopen(name_of_output_file, "wb");
    if (temp_file == NULL)
    {
        asm_error = ASM_ERROR_FOPEN;
        goto close_input_file;
    }
    asm_struct->output_file = temp_file;

    stk_error = stack_ctor(&(asm_struct->labels), sizeof(AsmLabel));
    if (stk_error)
    {
        stk_print_error(stk_error);
        asm_error = ASM_ERROR_STK;
        goto close_all_files;
    }

    proc_file_error = process_file(&(asm_struct->ptr_to_lines), name_of_input_file, asm_struct->input_file, &(asm_struct->number_of_lines));
    if (proc_file_error)
    {
        proc_file_print_error(proc_file_error);
        asm_error = ASM_ERROR_PROC_FILE;
        goto full_termination;
    }
    temp_output_buffer = (char *)calloc(asm_struct->number_of_lines, sizeof(double) + sizeof(char));
    if (temp_output_buffer == NULL)
    {
        asm_error = ASM_ERROR_CALLOC;
        goto full_termination;
    }
    asm_struct->output_buffer = temp_output_buffer;

    goto out;

full_termination:
    stack_dtor(&(asm_struct->labels));
close_all_files:
    fclose(asm_struct->output_file);
close_input_file:
    fclose(asm_struct->input_file);
out:
    return asm_error;
}

AsmError asm_assembly(Assembler *asm_struct)
{
    assert(asm_struct);

    if (asm_struct == NULL)
        return ASM_ERROR_NULL_PTR_STRUCT;

    AsmError error = ASM_ERROR_OK;
    size_t position = 0;
    for (size_t number_of_iterations = 0; number_of_iterations < NUMBER_OF_ROUNDS; number_of_iterations++)
    {
        position = 0;
        error = asm_make_assem_file(&(asm_struct->labels), asm_struct->ptr_to_lines, asm_struct->number_of_lines, asm_struct->output_buffer, &position);
        if (error)
        {
            return error;
        }
    }

    size_t return_value = fwrite(asm_struct->output_buffer, sizeof(char), position, asm_struct->output_file);
    if (return_value != position)
        return ASM_ERROR_FWRITE;

    return ASM_ERROR_OK;
}

AsmError asm_dtor(Assembler *asm_struct)
{
    assert(asm_struct);
    AsmError asm_error = ASM_ERROR_OK;

    if (asm_struct == NULL)
        return ASM_ERROR_NULL_PTR_STRUCT;

    fclose(asm_struct->input_file);
    fclose(asm_struct->output_file);
    StkError stk_error = stack_dtor(&(asm_struct->labels));
    if (stk_error)
        asm_error = ASM_ERROR_STK;

    asm_struct->number_of_lines = 0;
    free(asm_struct->output_buffer);
    free((asm_struct->ptr_to_lines)[0]);
    free(asm_struct->ptr_to_lines);

    return asm_error;
}

void asm_print_error(enum AsmError error)
{
    PRINT("%s\n", asm_get_error(error));
}

const char *asm_get_error(enum AsmError error)
{
    PRINT_BEGIN();
    switch (error)
    {
    case ASM_ERROR_OK:
        return "Ass: Ошибок в работе функций не выявлено.";
    case ASM_ERROR_NULL_PTR_FILE:
        return "Ass: Передан нулевой указатель на файл.";
    case ASM_ERROR_FOPEN:
        return "Ass: Ошибка в работе функции fopen.";
    case ASM_ERROR_NULL_PTR_LOG:
        return "Ass: Вместо адреса log_file передан нулевой указатель.";
    case ASM_ERROR_SSCANF:
        return "Ass: Ошибка в работе функции sscanf.";
    case ASM_ERROR_FSEEK:
        return "Ass: Ошибка в работе функции fseek.";
    case ASM_ERROR_FTELL:
        return "Ass: Ошибка в работе функции ftell.";
    case ASM_ERROR_STAT:
        return "Ass: Ошибка в работе функции stat.";
    case ASM_ERROR_STRCHR:
        return "Ass: Ошибка в работе функции strchr.";
    case ASM_ERROR_CALLOC:
        return "Ass: Ошибка в выделении памяти (calloc).";
    case ASM_ERROR_NOT_CMD:
        return "Asm: Не обнаружено команды.";
    case ASM_ERROR_READ:
        return "Asm: Некорректная запись в файле.";
    case ASM_ERROR_STK:
        return "Asm: Ошибка работы стэка.";
    case ASM_ERROR_SETVBUF:
        return "Asm: Ошибка отключения буферизации.";
    case ASM_ERROR_NULL_PTR_STRUCT:
        return "Asm: Передан нулевой указатель на структуру.";
    case ASM_ERROR_PROC_FILE:
        return "Asm: Ошибка обработки файла.";
    case ASM_ERROR_FWRITE:
        return "Asm: Ошибка работы функции fwrite.";
    case ASM_ERROR_RAM:
        return "Asm: Ошибка синтаксиса обращения к оперативной памяти.";
    case ASM_ERROR_STRTOD:
        return "Asm: Ошибка работы функции strtod.";
    default:
        return "Ass: Нужной ошибки не найдено...";
    }
}
