#include "../include/assembler.hpp"
#include "../include/print_in_log.hpp"
#include "../include/void_stack.hpp"
#include "../include/skip_space.hpp"
#include <assert.h>

static FILE* log_file = stderr;

const int    MAX_SYMB = 20;

const int    NUMBER_OF_LABELS = 10;

const int LABEL_IS_NEW = -1;

const size_t NUMBER_OF_CMDS = sizeof (CMD_ARRAY) / sizeof (CMD_ARRAY[0]);

const size_t NUMBER_OF_REGS = sizeof (REG_ARRAY) / sizeof (REG_ARRAY[0]);

void asm_set_log_file (FILE* file)
{
    log_file = file;
}

Cmd search_command (const char* line, size_t size)
{
    for (size_t n_cmd = 1; n_cmd < NUMBER_OF_CMDS; n_cmd++)
    {
        if (strncmp (line, CMD_ARRAY[n_cmd].name, size) == 0)
            return CMD_ARRAY[n_cmd].cmd_enum;
    }
    return NOT_CMD;
}

Reg search_reg (const char* line, size_t size)
{
    for (size_t n_reg = 1; n_reg < NUMBER_OF_REGS; n_reg++)
    {
        if (strncmp (line, REG_ARRAY[n_reg].name, size) == 0)
            return REG_ARRAY[n_reg].reg_enum;
    }
    return NOT_REG;
}

int search_label (const char* line, size_t size, Stack* labels)
{
    for (size_t label_number = 0; label_number < labels->size; label_number++)
    {
        if (strncmp (line, ((Label*)(char*) labels->data + label_number * labels->elem_size)->name, size) == 0)
        {
            return (int) label_number;
        }
    }
    return LABEL_IS_NEW;
}

AsmError make_assem_file (Stack* labels, char** lines, size_t num_line, char* buffer, size_t* position)
{
    PRINT_BEGIN();
    assert (lines);

    AsmError asm_error = ASM_NO_ERROR;
    StkError stk_error = STK_NO_ERROR;

    for (size_t n_line = 0; n_line < num_line; n_line++)
    {
        const char* cur_line = lines[n_line];
        skip_space (&cur_line);

        if (try_command (labels, cur_line, buffer, position) ||
            try_label (labels, cur_line, *position, &stk_error))
        {
            if (stk_error)
            {
                stk_print_error (stk_error);
                asm_error = ASM_ERROR_STK;
                goto termination;
            }

            continue;
        }
        else
        {
            asm_error = ASM_ERROR_NOT_CMD;
        }
    }

termination:
    PRINT_END();
    return asm_error;
}

void labels_name_dtor (Stack* labels)
{
    for (size_t label_number = 0; label_number < labels->size; label_number++)
        free (((Label*)(char*) labels->data + label_number * labels->elem_size)->name);
}

bool try_label (Stack* labels, const char* cur_line, size_t position, StkError* error)
{
    const char* colon = strchr (cur_line, ':');
    if (colon != NULL)
    {
        int label_number = search_label (cur_line, (size_t) (colon - cur_line), labels);
        if (label_number != LABEL_IS_NEW)
        {
            return true;
        }
        else
        {
            Label new_label = {};
            new_label.name = (char*) calloc (MAX_SYMB, sizeof (char));
            new_label.len = (size_t) (colon - cur_line);
            strncpy (new_label.name, cur_line, new_label.len);
            new_label.ip = position;

            *error = stack_push (labels, &new_label);
            if (*error)
                return false;

            return true;
        }
    }
    return false;
}

bool try_command (Stack* labels, const char* cur_line, char* buffer, size_t* position)
{
    size_t len_of_command = 0;
    if (isalpha (*cur_line))
    {
        len_of_command++;
        while (isalpha(cur_line[len_of_command]))
            len_of_command++;

        Cmd command = search_command (cur_line, len_of_command);
        if (command != NOT_CMD)
        {
            cur_line += len_of_command;
            skip_space (&cur_line);

            buffer[*position] = (char) command;
            if (CMD_ARRAY[(int) command].has_reg)
                if (try_command_reg (cur_line, buffer, position))
                    return true;

            if (CMD_ARRAY[(int) command].has_digit)
                if (try_command_digit (cur_line, buffer, position))
                    return true;

            if (CMD_ARRAY[(int) command].has_label)
                if (try_command_label (labels, cur_line, buffer, position))
                    return true;

            (*position)++;

            return true;
        }
    }
    return false;
}

bool try_command_label (Stack* labels, const char* cur_line, char* buffer, size_t* position)
{
    size_t len_of_label = 0;
    if (isalpha (*cur_line))
    {
        len_of_label++;
        while (isalpha (cur_line[len_of_label]))
            len_of_label++;

        buffer[*position] |= MASK_NUMBER;
        *position += sizeof (char);

        int label_number = search_label (cur_line, len_of_label, labels);
        if (label_number != LABEL_IS_NEW)
        {
            memcpy (buffer + *position,
                    &(((Label*)((char*) labels->data + (size_t) label_number * labels->elem_size))->ip),
                    sizeof (size_t));

            *position += sizeof (size_t);
        }
        else
        {
            size_t value = 0;
            memcpy (buffer + *position, &value, sizeof (size_t));
            *position += sizeof (size_t);
        }
        return true;
    }
    return false;
}

bool try_command_digit (const char* cur_line, char* buffer, size_t* position)
{
    size_t len_of_double = 0;
    if (isdigit (*cur_line))
    {
        len_of_double++;
        while (isdigit (cur_line[len_of_double]))
            len_of_double++;

        buffer[*position] |= MASK_NUMBER;
        (*position)++;

        double value = NAN;
        sscanf (cur_line, "%lf", &value);
        memcpy (buffer + *position, &value, sizeof (double));
        *position += sizeof (double);

        return true;
    }
    return false;
}

bool try_command_reg (const char* cur_line, char* buffer, size_t* position)
{
    size_t len_of_reg = 0;
    if (isalpha (*cur_line))
    {
        len_of_reg++;
        while (isalpha(cur_line[len_of_reg]))
            len_of_reg++;

        Reg reg = search_reg (cur_line, len_of_reg);
        if (reg != NOT_REG)
        {
            buffer[*position] |= MASK_REG;
            *position += sizeof (char);
            buffer[*position] = (char) reg;
            *position += sizeof (char);

            return true;
        }
    }
    return false;
}

AsmError asm_ctor (Assem* asm_struct, const char* name_of_input_file)
{
    assert (asm_struct);
    AsmError      asm_error       = ASM_NO_ERROR;
    ProcFileError proc_file_error = PROC_FILE_NO_ERROR;
    StkError      stk_error       = STK_NO_ERROR;
    if (asm_struct == NULL)
        return ASM_ERROR_NULL_PTR_STRUCT;

    char* temp_output_buffer = NULL;
    FILE* temp_file = fopen (name_of_input_file, "rb");
    if (temp_file == NULL)
        return ASM_ERROR_FOPEN;
    asm_struct->input_file = temp_file;

    temp_file = fopen ("../Processor/asm_output.txt", "wb");
    if (temp_file == NULL)
    {
        asm_error = ASM_ERROR_FOPEN;
        goto close_file;
    }
    asm_struct->output_file = temp_file;

    stk_error = stack_ctor (&(asm_struct->labels), sizeof (Label));
    if (stk_error)
    {
        stk_print_error (stk_error);
        asm_error = ASM_ERROR_STK;
        goto close_all_files;
    }

    proc_file_error = process_file (&(asm_struct->ptr_to_lines), name_of_input_file, asm_struct->input_file, &(asm_struct->number_of_lines));
    if (proc_file_error)
    {
        proc_file_print_error (proc_file_error);
        asm_error = ASM_ERROR_PROC_FILE;
        goto full_termination;
    }
    temp_output_buffer = (char*) calloc (asm_struct->number_of_lines, sizeof (double) + sizeof (char));
    if (temp_output_buffer == NULL)
    {
        asm_error = ASM_ERROR_CALLOC;
        goto full_termination;
    }
    asm_struct->output_buffer = temp_output_buffer;

    goto out;

full_termination:
    stack_dtor (&(asm_struct->labels));
close_all_files:
    fclose (asm_struct->output_file);
close_file:
    fclose (asm_struct->input_file);
out:
    return asm_error;
}

AsmError assembly (Assem* asm_struct)
{
    assert (asm_struct);

    if (asm_struct == NULL)
        return ASM_ERROR_NULL_PTR_STRUCT;

    AsmError error = ASM_NO_ERROR;
    size_t position = 0;
    error = make_assem_file (&(asm_struct->labels), asm_struct->ptr_to_lines, asm_struct->number_of_lines, asm_struct->output_buffer, &position);
    if (error)
    {
        return error;
    }

    position = 0;
    error = make_assem_file (&(asm_struct->labels), asm_struct->ptr_to_lines, asm_struct->number_of_lines, asm_struct->output_buffer, &position);
    if (error)
    {
        return error;
    }
    size_t return_value = fwrite (asm_struct->output_buffer, sizeof (char), position, asm_struct->output_file);
    if (return_value != position)
        return ASM_ERROR_FWRITE;

    return ASM_NO_ERROR;
}

AsmError asm_dtor (Assem* asm_struct)
{
    assert (asm_struct);
    AsmError asm_error = ASM_NO_ERROR;

    if (asm_struct == NULL)
        return ASM_ERROR_NULL_PTR_STRUCT;

    fclose (asm_struct->input_file);
    fclose (asm_struct->output_file);
    labels_name_dtor (&(asm_struct->labels));
    StkError stk_error = stack_dtor (&(asm_struct->labels));
    if (stk_error)
        asm_error = ASM_ERROR_STK;

    asm_struct->number_of_lines = 0;
    free (asm_struct->output_buffer);
    free ((asm_struct->ptr_to_lines)[0]);
    free (asm_struct->ptr_to_lines);

    return asm_error;
}

void asm_print_error (enum AsmError error)
{
    PRINT ("%s\n", asm_get_error (error));
}

const char* asm_get_error (enum AsmError error)
{
    PRINT_BEGIN();
    switch (error)
    {
        case ASM_NO_ERROR:
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
        default:
            return "Ass: Нужной ошибки не найдено...";
    }
}
