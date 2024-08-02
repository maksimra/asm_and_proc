#include "../include/assembler.hpp"
#include "../include/print_in_log.hpp"
#include "../include/void_stack.hpp"
#include "../include/skip_space.hpp"

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

enum AsmError make_assem_file (Stack* labels, char** lines, size_t num_line)
{
    PRINT_BEGIN();
    assert (lines);

    AsmError asm_error = ASM_NO_ERROR;
    StkError stk_error = STK_NO_ERROR;

    FILE *assemble_file = fopen ("asm_output.txt", "wb");

    if (assemble_file == NULL)
        return ASM_ERROR_FOPEN;

    char* buffer = (char*) calloc (num_line, sizeof (double) + sizeof (char));
    size_t position = 0;

    for (size_t n_line = 0; n_line < num_line; n_line++)
    {
        const char* cur_line = lines[n_line];
        skip_space (&cur_line);

        if (try_command (labels, cur_line, buffer, &position) ||
            try_label (labels, cur_line, position, &stk_error))
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
    fwrite (buffer, sizeof (char), position, assemble_file);

termination:
    fclose (assemble_file);
    free (buffer);
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
        default:
            return "Ass: Нужной ошибки не найдено...";
    }
}
