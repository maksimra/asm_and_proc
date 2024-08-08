#include "../include/processor.hpp"
#include "../include/file_processing.hpp"
#include "../include/print_in_log.hpp"
#include "../include/ram.hpp"

#define PRINT_AND_RETURN_IF_ERROR_RAM(error) if (error) \
                                             { \
                                                ram_print_error (error); \
                                                return PROC_ERROR_RAM; \
                                             }

#define PRINT_AND_RETURN_IF_ERROR_STK(error) if (error) \
                                             { \
                                                stk_print_error (error); \
                                                return PROC_ERROR_STK; \
                                             }

#define RETURN_IF_ERROR(error) if (error) \
                               { \
                                   return error; \
                               }

#define STK    proc_struct->stk

#define BUFFER proc_struct->input_buffer

static FILE* log_file = stderr;

const size_t num_of_double = 25;

const size_t BASIC_CAPACITY = 50;

const int NUM_OF_REG = 4;

void proc_set_log_file (FILE* file)
{
    log_file = file;
}

void proc_print_error (ProcError error)
{
    PRINT_BEGIN();
    PRINT ("%s\n", proc_get_error (error));
    PRINT_END();
}

ProcError calculations (Proc* proc_struct)
{
    PRINT_BEGIN();
    StkError  stk_error  = STK_NO_ERROR;
    ProcError proc_error = PROC_NO_ERROR;
    RAMError  ram_error  = RAM_NO_ERROR;
    stk_element reg[NUM_OF_REG] = {};
    size_t position = 0;
    while (position < proc_struct->file_size)
    {
        switch ((unsigned char) BUFFER[(int) position])
        {
            case IN:
                stk_error = cmd_in (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case PUSH + RAM:
                proc_error = cmd_push_ram (proc_struct, &position);
                RETURN_IF_ERROR (proc_error);
                break;
            case PUSH + NUM:
                stk_error = cmd_push_num (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case PUSH + REG:
                stk_error = cmd_push_reg (proc_struct, &position, reg);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case POP + RAM:
                proc_error = cmd_pop_ram (proc_struct, &position);
                RETURN_IF_ERROR (proc_error);
                break;
            case POP + REG:
                stk_error = cmd_pop_reg (proc_struct, &position, reg);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case ADD:
                stk_error = cmd_add (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case SUB:
                stk_error = cmd_sub (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case MUL:
                stk_error = cmd_mul (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case DIV:
                stk_error = cmd_div (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case JMP + NUM:
                cmd_jmp_num (proc_struct, &position);
                break;
            case JMP + RAM:
                ram_error = cmd_jmp_ram (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_RAM (ram_error);
                break;
            case CALL + RAM:
                proc_error = cmd_call_ram (proc_struct, &position);
                RETURN_IF_ERROR (proc_error);
                break;
            case CALL + NUM:
                stk_error = cmd_call_num (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case RET:
                stk_error = cmd_ret (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case OUT:
                stk_error = cmd_out (proc_struct, &position);
                PRINT_AND_RETURN_IF_ERROR_STK (stk_error);
                break;
            case HLT:
                return PROC_NO_ERROR;
            default:
                PRINT ("Зашёл в default :(\n");
                return PROC_ERROR_CMDS;
        }
    }
    PRINT_END();
    return PROC_NO_ERROR;
}

ProcError cmd_call_ram (Proc* proc_struct, size_t* position)
{
    PRINT ("case CALL RAM\n");
    (*position)++;

    StkError stk_error = stack_push (&(STK), (double) (*position + sizeof (double)));
    PRINT_AND_RETURN_IF_ERROR_STK (stk_error);

    double array_index_in_double = NAN;
    memcpy (&array_index_in_double, BUFFER + *position, sizeof (double));

    double ram_value = NAN;
    RAMError ram_error = ram_get (&(proc_struct->ram),
                                  &ram_value,
                                  (size_t) array_index_in_double);
    PRINT_AND_RETURN_IF_ERROR_RAM (ram_error);

    *position = (size_t) ram_value;
    return PROC_NO_ERROR;
}

RAMError cmd_jmp_ram (Proc* proc_struct, size_t* position)
{
    PRINT ("case JMP RAM\n");
    (*position)++;

    double array_index_in_double = NAN;
    memcpy (&array_index_in_double, BUFFER + *position, sizeof (double));

    double ram_value = NAN;
    RAMError ram_error = ram_get (&(proc_struct->ram),
                                  &ram_value,
                                  (size_t) array_index_in_double);
    RETURN_IF_ERROR (ram_error);

    *position = (size_t) ram_value;
    return RAM_NO_ERROR;
}

ProcError cmd_pop_ram (Proc* proc_struct, size_t* position)
{
    PRINT ("case POP RAM\n");
    (*position)++;
    double number = NAN;
    StkError stk_error = stack_pop (&(STK), &number);
    PRINT_AND_RETURN_IF_ERROR_STK (stk_error);

    double array_index_in_double = NAN;
    memcpy (&array_index_in_double, BUFFER + *position, sizeof (double));

    RAMError ram_error = ram_push (&(proc_struct->ram),
                                   &number,
                                   (size_t) array_index_in_double);
    PRINT_AND_RETURN_IF_ERROR_RAM (ram_error);

    *position += sizeof (double);
    return PROC_NO_ERROR;
}

ProcError cmd_push_ram (Proc* proc_struct, size_t* position)
{
    PRINT ("case PUSH RAM\n");
    (*position)++;
    double array_index_in_double = NAN;
    memcpy (&array_index_in_double, BUFFER + *position, sizeof (double));

    double ram_value = NAN;
    RAMError ram_error = ram_get (&(proc_struct->ram),
                                  &ram_value,
                                  (size_t) array_index_in_double);
    PRINT_AND_RETURN_IF_ERROR_RAM (ram_error);

    StkError stk_error = stack_push (&(STK), ram_value);
    PRINT_AND_RETURN_IF_ERROR_STK (stk_error);

    (*position) += sizeof (double);
    return PROC_NO_ERROR;
}

StkError cmd_out (Proc* proc_struct, size_t* position)
{
    PRINT ("case OUT\n");
    double number = NAN;
    StkError error = stack_pop (&(STK), &number);
    RETURN_IF_ERROR(error);
    printf ("Результат: %lg\n", number);
    (*position)++;
    return error;
}

StkError cmd_ret (Proc* proc_struct, size_t* position)
{
    PRINT ("case RET\n");
    double number = NAN;
    StkError error = stack_pop (&(STK), &number);
    RETURN_IF_ERROR (error);
    *position = (size_t) number;
    return error;
}

StkError cmd_call_num (Proc* proc_struct, size_t* position)
{
    PRINT ("case CALL\n");
    (*position)++;
    StkError error = stack_push (&(STK), (double) (*position + sizeof (double)));
    RETURN_IF_ERROR (error);
    double position_in_double = NAN;
    memcpy (&position_in_double, BUFFER + *position, sizeof (double));
    *position = (size_t) position_in_double;
    return error;
}

void cmd_jmp_num (Proc* proc_struct, size_t* position)
{
    PRINT ("case JMP\n");
    (*position)++;
    double position_in_double = NAN;
    memcpy (&position_in_double, BUFFER + *position, sizeof (double));
    *position = (size_t) position_in_double;
}

StkError cmd_div (Proc* proc_struct, size_t* position)
{
    PRINT ("case DIV\n");
    double num1 = NAN;
    StkError error = stack_pop (&(STK), &num1);
    RETURN_IF_ERROR (error);

    double num2 = NAN;
    error = stack_pop (&(STK), &num2);
    RETURN_IF_ERROR (error);
    error = stack_push (&(STK), num2 / num1);
    (*position)++;
    return error;
}

StkError cmd_mul (Proc* proc_struct, size_t* position)
{
    PRINT ("case MUL\n");
    double num1 = NAN;
    StkError error = stack_pop (&(STK), &num1);
    RETURN_IF_ERROR (error);

    double num2 = NAN;
    error = stack_pop (&(STK), &num2);
    RETURN_IF_ERROR (error);
    error = stack_push (&(STK), num1 * num2);
    (*position)++;
    return error;
}

StkError cmd_sub (Proc* proc_struct, size_t* position)
{
    PRINT ("case SUB\n");
    double num1 = NAN;
    StkError error = stack_pop (&(STK), &num1);
    RETURN_IF_ERROR (error);

    double num2 = NAN;
    error = stack_pop (&(STK), &num2);
    RETURN_IF_ERROR (error);
    error = stack_push (&(STK), num2 - num1);
    (*position)++;
    return error;
}

StkError cmd_add (Proc* proc_struct, size_t* position)
{
    PRINT ("case ADD\n");
    double num1 = NAN;
    StkError error = stack_pop (&(STK), &num1);
    RETURN_IF_ERROR (error);

    double num2 = NAN;
    error = stack_pop (&(STK), &num2);
    RETURN_IF_ERROR (error);
    error = stack_push (&(STK), num1 + num2);
    (*position)++;
    return error;
}

StkError cmd_pop_reg (Proc* proc_struct, size_t* position, double* const reg)
{
    PRINT ("case POP REG\n");
    double number = NAN;
    StkError error = stack_pop (&(STK), &number);
    RETURN_IF_ERROR (error);

    (*position)++;
    PRINT ("Номер регистра = %d\n", BUFFER[*position]);
    reg[(int) BUFFER[*position]] = number;
    (*position)++;
    return error;
}

StkError cmd_push_reg (Proc* proc_struct, size_t* position, double* const reg)
{
    PRINT ("case PUSH REG\n");
    (*position)++;
    PRINT ("Номер регистра  = %d\n", BUFFER[*position]);
    PRINT ("Кладу в стэк %lf\n", reg[(int) BUFFER[*position]]);
    StkError error = stack_push (&(STK), reg[(int) BUFFER[*position]]);
    (*position)++;
    return error;
}

StkError cmd_push_num (Proc* proc_struct, size_t* position)
{
    PRINT ("case PUSH NUM\n");
    (*position)++;
    double number = NAN;
    memcpy (&number, BUFFER + *position, sizeof (double));
    StkError error = stack_push (&(STK), number);
    *position += sizeof (double);
    return error;
}

StkError cmd_in (Proc* proc_struct, size_t* position)
{
    PRINT ("case IN\n");
    printf ("Enter your double in stack.\n");
    double number = NAN;
    scanf ("%lf", &number);
    StkError error = stack_push (&(STK), number);
    (*position)++;
    return error;
}

ProcError proc_ctor (Proc* proc_struct, const char* name_of_input_file)
{
    assert (proc_struct);
    RAMError ram_error = RAM_NO_ERROR;
    StkError stk_error = STK_NO_ERROR;
    if (proc_struct == NULL)
        return PROC_ERROR_NULL_PTR_STRUCT;

    FILE* temp_file = fopen (name_of_input_file, "rb");
    if (temp_file == NULL)
        return PROC_ERROR_FOPEN;
    proc_struct->input_file = temp_file;

    ProcFileError proc_file_error = PROC_FILE_NO_ERROR;
    proc_file_error = read_file_count_size (name_of_input_file, temp_file, &(proc_struct->file_size), &(BUFFER));
    ProcError proc_error = PROC_NO_ERROR;
    if (proc_file_error)
    {
        proc_file_print_error (proc_file_error);
        proc_error = PROC_ERROR_PROC_FILE;
        goto close_input_file;
    }
    stk_error = stack_ctor (&(STK), BASIC_CAPACITY);
    if (stk_error)
    {
        stk_print_error (stk_error);
        proc_error = PROC_ERROR_STK;
        goto free_memory;
    }
    ram_error = ram_ctor (&(proc_struct->ram), sizeof (double));
    if (ram_error)
    {
        ram_print_error (ram_error);
        proc_error = PROC_ERROR_RAM;
        goto stack_destructor;
    }
    goto out;

stack_destructor:
    stack_dtor (&(STK));
free_memory:
    free (BUFFER);
close_input_file:
    fclose (proc_struct->input_file);
out:
    return proc_error;
}

ProcError proc_dtor (Proc* proc_struct)
{
    assert (proc_struct != NULL);
    if (proc_struct == NULL)
        return PROC_ERROR_NULL_PTR_STRUCT;

    ProcError proc_error = PROC_NO_ERROR;
    fclose (proc_struct->input_file);
    proc_struct->file_size = 0;
    free (BUFFER);
    StkError stk_error = stack_dtor (&(STK));
    if (stk_error)
    {
        stk_print_error (stk_error);
        proc_error = PROC_ERROR_STK;
    }
    RAMError ram_error = ram_dtor (&(proc_struct->ram));
    if (ram_error)
    {
        ram_print_error (ram_error);
        proc_error = PROC_ERROR_RAM;
    }

    return proc_error;
}

const char* proc_get_error (ProcError error)
{
    switch (error)
    {
        case PROC_NO_ERROR:
            return "Proc: Ошибок в работе функций не выявлено.";
        case PROC_ERROR_FOPEN:
            return "Proc: Ошибка в работе функции fopen.";
        case PROC_ERROR_CMDS:
            return "Proc: Ошибка распознования команды.";
        case PROC_ERROR_STAT:
            return "Proc: Ошибка выполнения функции stat.";
        case PROC_ERROR_FSEEK:
            return "Proc: Ошибка выполнения функции fseek.";
        case PROC_ERROR_ARGC:
            return "Proc: Введены некорректные аргументы.";
        case PROC_ERROR_NULL_PTR_FILE:
            return "Proc: Нулевой указатель на файл.";
        case PROC_ERROR_CALLOC:
            return "Proc: Ошибка функции calloc.";
        case PROC_ERROR_STACK:
            return "Proc: Ошибка функции pop.";
        case PROC_ERROR_SETVBUF:
            return "Proc: Ошибка отключения буферизации.";
        case PROC_ERROR_NULL_PTR_STRUCT:
            return "Proc: Передан нулевой указатель на структуру.";
        case PROC_ERROR_PROC_FILE:
            return "Proc: Ошибка обработки файла.";
        case PROC_ERROR_STK:
            return "Proc: Ошибка стэка.";
        case PROC_ERROR_RAM:
            return "Proc: Ошибка работы в RAM.";
        default:
            return "Stack: Куда делся мой enum ошибок?";
    }
}
