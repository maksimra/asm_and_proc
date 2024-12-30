#include <math.h>
#include "../include/processor.hpp"
#include "../include/file_processing.hpp"
#include "../include/compare_doubles.hpp"
#include "../include/print_in_log.hpp"
#include "../include/array.hpp"

#define PRINT_AND_RETURN_IF_ERROR_RAM(error) \
    if (error)                               \
    {                                        \
        ram_print_error(error);              \
        return PROC_ERROR_RAM;               \
    }

#define PRINT_AND_RETURN_IF_ERROR_STK(error) \
    if (error)                               \
    {                                        \
        stk_print_error(error);              \
        return PROC_ERROR_STK;               \
    }

#define RETURN_IF_ERROR(error) \
    if (error)                 \
    {                          \
        return error;          \
    }

#define STK proc_struct->stk

#define BUFFER proc_struct->input_buffer

static FILE *log_file = stderr;

const char MASK_ARG = (char)0b11100000;

const size_t num_of_double = 25;

const size_t BASIC_CAPACITY = 50;

void proc_set_log_file(FILE *file)
{
    log_file = file;
}

void proc_print_error(ProcError error)
{
    PRINT("%s\n", proc_get_error(error));
}

ProcError proc_calculations(Processor *proc_struct)
{
    PRINT_BEGIN();
    ProcError proc_error = PROC_ERROR_OK;
    size_t position = 0;
    // int i = 0;
    while (position < proc_struct->file_size)
    {
        // TODO: delete i++;
        // TODO: delete if (i > 30)
        // TODO: delete     abort();

        // printf("rax = %lg\nrbx = %lg\nrcx = %lg\nrdx = %lg\n",
        //        *(proc_struct->reg),
        //        *(proc_struct->reg + 1),
        //        *(proc_struct->reg + 2),
        //        *(proc_struct->reg + 3));
        // stack_dump(&(proc_struct->stk));
        char command = BUFFER[position] & (~MASK_ARG);
        // printf("com: %d\n", command);
        switch (command)
        {
            case PROC_CMD_PUSH:
                proc_error = proc_cmd_push(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_POP:
                proc_error = proc_cmd_pop(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_ADD:
                proc_error = proc_cmd_add(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_SUB:
                proc_error = proc_cmd_sub(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_MUL:
                proc_error = proc_cmd_mul(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_DIV:
                proc_error = proc_cmd_div(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_SQRT:
                proc_error = proc_cmd_sqrt(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_OUT:
                proc_error = proc_cmd_out(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_IN:
                proc_error = proc_cmd_in(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JMP:
                proc_error = proc_cmd_jmp(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JE:
                proc_error = proc_cmd_je(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JA:
                proc_error = proc_cmd_ja(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JB:
                proc_error = proc_cmd_jb(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JEA:
                proc_error = proc_cmd_jea(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JEB:
                proc_error = proc_cmd_jeb(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JNE:
                proc_error = proc_cmd_jne(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JNA:
                proc_error = proc_cmd_jeb(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_JNB:
                proc_error = proc_cmd_jea(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_CALL:
                proc_error = proc_cmd_call(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_RET:
                proc_error = proc_cmd_ret(proc_struct, &position);
                RETURN_IF_ERROR(proc_error);
                break;
            case PROC_CMD_HLT:
                return PROC_ERROR_OK;
            default:
                PRINT("Зашёл в default :(\n");
                return PROC_ERROR_CMDS;
        }
    }
    PRINT_END();
    return PROC_ERROR_OK;
}

ProcError proc_put_value(Processor *proc_struct, size_t *position, double value)
{
    char arg = BUFFER[*position] & MASK_ARG;
    (*position)++;
    double IP = NAN;
    memcpy(&IP, BUFFER + *position, sizeof(char));
    switch ((unsigned char)arg)
    {
        case ARG_REG:
            return proc_put_in_reg(proc_struct, position, value);
        case ARG_RAM:
            return proc_put_in_ram(proc_struct, position, value);
        case ARG_RAM + ARG_REG:
            return proc_put_in_ram_and_reg(proc_struct, position, value);
        default:
            return PROC_ERROR_ARG;
    }
}

ProcError proc_put_in_reg(Processor *proc_struct, size_t *position, double value)
{
    PRINT_BEGIN();
    char reg_number = 0;
    memcpy(&reg_number, BUFFER + *position, sizeof(char));

    proc_struct->reg[(int)reg_number] = value;
    (*position)++;
    PRINT_END();
    return PROC_ERROR_OK;
}

ProcError proc_put_in_ram(Processor *proc_struct, size_t *position, double value)
{
    PRINT_BEGIN();
    double array_index_in_double = NAN;
    memcpy(&array_index_in_double, BUFFER + *position, sizeof(double));

    ArrError arr_error = arr_put(&(proc_struct->ram),
                                 &value,
                                 (size_t)array_index_in_double);
    if (arr_error)
    {
        arr_print_error(arr_error);
        return PROC_ERROR_RAM;
    }

    *position += sizeof(double);
    PRINT_END();
    return PROC_ERROR_OK;
}

ProcError proc_put_in_ram_and_reg(Processor *proc_struct, size_t *position, double value)
{
    PRINT_BEGIN();
    double array_index_in_double = NAN;
    ProcError proc_error = proc_get_from_reg(proc_struct, position, &array_index_in_double);
    if (proc_error)
        return proc_error;

    ArrError arr_error = arr_put(&(proc_struct->ram),
                                 &value,
                                 (size_t)array_index_in_double);
    if (arr_error)
    {
        arr_print_error(arr_error);
        return PROC_ERROR_RAM;
    }

    PRINT_END();
    return proc_error;
}

ProcError proc_get_value(Processor *proc_struct, size_t *position, double *value)
{
    char arg = BUFFER[*position] & MASK_ARG;
    (*position)++;
    switch ((unsigned char)arg)
    {
        case ARG_NUM:
            return proc_get_num(proc_struct, position, value);
        case ARG_REG:
            return proc_get_from_reg(proc_struct, position, value);
        case ARG_RAM:
            return proc_get_from_ram(proc_struct, position, value);
        case ARG_RAM + ARG_REG:
            return proc_get_from_ram_and_reg(proc_struct, position, value);
        default:
            return PROC_ERROR_ARG;
    }
}

ProcError proc_get_num(Processor *proc_struct, size_t *position, double *value)
{
    PRINT_BEGIN();
    memcpy(value, BUFFER + *position, sizeof(double));
    *position += sizeof(double);
    PRINT_END();
    return PROC_ERROR_OK;
}

ProcError proc_get_from_reg(Processor *proc_struct, size_t *position, double *value)
{
    PRINT_BEGIN();
    char reg_number = 0;
    memcpy(&reg_number, BUFFER + *position, sizeof(char));
    PRINT("Номер регистра == %d\n", reg_number);

    *value = proc_struct->reg[(int)reg_number];
    (*position)++;
    PRINT_END();
    return PROC_ERROR_OK;
}

ProcError proc_get_from_ram(Processor *proc_struct, size_t *position, double *value)
{
    PRINT_BEGIN();
    double array_index_in_double = NAN;
    memcpy(&array_index_in_double, BUFFER + *position, sizeof(double));

    ArrError arr_error = arr_get(&(proc_struct->ram),
                                 value,
                                 (size_t)array_index_in_double);
    if (arr_error)
    {
        arr_print_error(arr_error);
        return PROC_ERROR_RAM;
    }

    (*position) += sizeof(double);
    PRINT_END();
    return PROC_ERROR_OK;
}

ProcError proc_get_from_ram_and_reg(Processor *proc_struct, size_t *position, double *value)
{
    PRINT_BEGIN();
    double array_index_in_double = NAN;
    ProcError proc_error = proc_get_from_reg(proc_struct, position, &array_index_in_double);
    if (proc_error)
        return proc_error;

    ArrError arr_error = arr_get(&(proc_struct->ram),
                                 value,
                                 (size_t)array_index_in_double);
    if (arr_error)
    {
        arr_print_error(arr_error);
        return PROC_ERROR_RAM;
    }

    PRINT_END();
    return proc_error;
}

ProcError proc_cmd_push(Processor *proc_struct, size_t *position)
{
    PRINT("case PUSH\n");
    double value = NAN;
    ProcError proc_error = proc_get_value(proc_struct, position, &value);
    if (proc_error)
        return proc_error;

    StkError stk_error = stack_push(&(STK), value);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    return proc_error;
}

ProcError proc_cmd_pop(Processor *proc_struct, size_t *position)
{
    PRINT("case POP\n");
    double value = NAN;
    StkError stk_error = stack_pop(&(STK), &value);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    ProcError proc_error = proc_put_value(proc_struct, position, value);
    return proc_error;
}

ProcError proc_cmd_jmp(Processor *proc_struct, size_t *position)
{
    PRINT("case JUMP\n");
    double value = NAN;
    ProcError proc_error = proc_get_value(proc_struct, position, &value);
    if (proc_error)
        return proc_error;

    *position = (size_t)value;
    return proc_error;
}

ProcError proc_cmd_je(Processor *proc_struct, size_t *position)
{
    PRINT("case JE\n");
    double value1 = NAN;
    double value2 = NAN;

    StkError stk_error = proc_get_two_numbers(&(STK), &value1, &value2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    if (compare_doubles(value1, value2) == 0)
        return proc_cmd_jmp(proc_struct, position);

    (*position) += sizeof(char) + sizeof(double);
    return PROC_ERROR_OK;
}

ProcError proc_cmd_ja(Processor *proc_struct, size_t *position)
{
    PRINT("case JA\n");
    double value1 = NAN;
    double value2 = NAN;

    StkError stk_error = proc_get_two_numbers(&(STK), &value1, &value2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    if (compare_doubles(value1, value2) > 0)
        return proc_cmd_jmp(proc_struct, position);

    (*position) += sizeof(char) + sizeof(double);
    return PROC_ERROR_OK;
}

ProcError proc_cmd_jb(Processor *proc_struct, size_t *position)
{
    PRINT("case JB\n");
    double value1 = NAN;
    double value2 = NAN;

    StkError stk_error = proc_get_two_numbers(&(STK), &value1, &value2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    if (compare_doubles(value1, value2) < 0)
        return proc_cmd_jmp(proc_struct, position);

    (*position) += sizeof(char) + sizeof(double);
    return PROC_ERROR_OK;
}

ProcError proc_cmd_jea(Processor *proc_struct, size_t *position)
{
    PRINT("case JEA\n");
    double value1 = NAN;
    double value2 = NAN;

    StkError stk_error = proc_get_two_numbers(&(STK), &value1, &value2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    if (compare_doubles(value1, value2) >= 0)
        return proc_cmd_jmp(proc_struct, position);

    (*position) += sizeof(char) + sizeof(double);
    return PROC_ERROR_OK;
}

ProcError proc_cmd_jeb(Processor *proc_struct, size_t *position)
{
    PRINT("case JEB\n");
    double value1 = NAN;
    double value2 = NAN;

    StkError stk_error = proc_get_two_numbers(&(STK), &value1, &value2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    if (compare_doubles(value1, value2) <= 0)
        return proc_cmd_jmp(proc_struct, position);

    (*position) += sizeof(char) + sizeof(double);
    return PROC_ERROR_OK;
}

ProcError proc_cmd_jne(Processor *proc_struct, size_t *position)
{
    PRINT("case JNE\n");
    double value1 = NAN;
    double value2 = NAN;
    StkError stk_error = proc_get_two_numbers(&(STK), &value1, &value2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    if (compare_doubles(value1, value2) != 0)
        return proc_cmd_jmp(proc_struct, position);

    (*position) += sizeof(char) + sizeof(double);
    return PROC_ERROR_OK;
}

ProcError proc_cmd_out(Processor *proc_struct, size_t *position)
{
    PRINT("case OUT\n");
    double number = NAN;
    StkError error = stack_pop(&(STK), &number);
    if (error)
    {
        stk_print_error(error);
        return PROC_ERROR_STK;
    }

    printf("%lg\n", number);
    (*position)++;
    return PROC_ERROR_OK;
}

ProcError proc_cmd_ret(Processor *proc_struct, size_t *position)
{
    PRINT("case RET\n");
    double number = NAN;
    StkError error = stack_pop(&(STK), &number);
    if (error)
    {
        stk_print_error(error);
        return PROC_ERROR_STK;
    }

    *position = (size_t)number;
    return PROC_ERROR_OK;
}

ProcError proc_cmd_call(Processor *proc_struct, size_t *position)
{
    PRINT("case CALL\n");
    StkError stk_error = stack_push(&(STK), (double)(*position + sizeof(char) + sizeof(double)));
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    ProcError proc_error = proc_cmd_jmp(proc_struct, position);
    return proc_error;
}

ProcError proc_cmd_div(Processor *proc_struct, size_t *position)
{
    PRINT("case DIV\n");
    double num1 = NAN;
    double num2 = NAN;
    StkError stk_error = proc_get_two_numbers(&(STK), &num1, &num2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    stk_error = stack_push(&(STK), num2 / num1);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    (*position)++;
    return PROC_ERROR_OK;
}

ProcError proc_cmd_sqrt(Processor *proc_struct, size_t *position)
{
    PRINT("case SQRT\n");
    double num = NAN;
    StkError stk_error = stack_pop(&(STK), &num);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    stk_error = stack_push(&(STK), sqrt(num));
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    (*position)++;
    return PROC_ERROR_OK;
}

StkError proc_get_two_numbers(Stack *stk, double *num1, double *num2)
{
    StkError error = stack_pop(stk, num1);
    if (error)
        return error;

    error = stack_pop(stk, num2);
    return error;
}

ProcError proc_cmd_mul(Processor *proc_struct, size_t *position)
{
    PRINT("case MUL\n");
    double num1 = NAN;
    double num2 = NAN;
    StkError stk_error = proc_get_two_numbers(&(STK), &num1, &num2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    stk_error = stack_push(&(STK), num1 * num2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    (*position)++;
    return PROC_ERROR_OK;
}

ProcError proc_cmd_sub(Processor *proc_struct, size_t *position)
{
    PRINT("case SUB\n");
    double num1 = NAN;
    double num2 = NAN;
    StkError stk_error = proc_get_two_numbers(&(STK), &num1, &num2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    stk_error = stack_push(&(STK), num2 - num1);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    (*position)++;
    return PROC_ERROR_OK;
}

ProcError proc_cmd_add(Processor *proc_struct, size_t *position)
{
    PRINT("case ADD\n");
    double num1 = NAN;
    double num2 = NAN;
    StkError stk_error = proc_get_two_numbers(&(STK), &num1, &num2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    stk_error = stack_push(&(STK), num1 + num2);
    if (stk_error)
    {
        stk_print_error(stk_error);
        return PROC_ERROR_STK;
    }

    (*position)++;
    return PROC_ERROR_OK;
}

ProcError proc_cmd_in(Processor *proc_struct, size_t *position)
{
    PRINT("case IN\n");
    printf("Enter your double in stack.\n");
    double number = NAN;
    scanf("%lf", &number);
    StkError error = stack_push(&(STK), number);
    if (error)
    {
        stk_print_error(error);
        return PROC_ERROR_STK;
    }

    (*position)++;
    return PROC_ERROR_OK;
}

ProcError proc_ctor(Processor *proc_struct, const char *name_of_input_file)
{
    assert(proc_struct);
    ArrError ram_error = ARR_NO_ERROR;
    StkError stk_error = STK_NO_ERROR;
    ProcFileError proc_file_error = PROC_FILE_NO_ERROR;
    if (proc_struct == NULL)
        return PROC_ERROR_NULL_PTR_STRUCT;

    FILE *temp_file = fopen(name_of_input_file, "rb");
    if (temp_file == NULL)
        return PROC_ERROR_FOPEN;
    proc_struct->input_file = temp_file;

    ProcError proc_error = PROC_ERROR_OK;
    proc_struct->reg = (stk_element *)calloc(NUM_OF_REGS, sizeof(stk_element));
    if (proc_struct->reg == NULL)
    {
        proc_error = PROC_ERROR_CALLOC;
        goto close_input_file;
    }

    proc_file_error = read_file_count_size(name_of_input_file, temp_file, &(proc_struct->file_size), &(BUFFER));
    if (proc_file_error)
    {
        proc_file_print_error(proc_file_error);
        proc_error = PROC_ERROR_PROC_FILE;
        goto free_memory_for_regs;
    }
    stk_error = stack_ctor(&(STK), BASIC_CAPACITY);
    if (stk_error)
    {
        stk_print_error(stk_error);
        proc_error = PROC_ERROR_STK;
        goto free_buffer;
    }
    ram_error = arr_ctor(&(proc_struct->ram), sizeof(double));
    if (ram_error)
    {
        arr_print_error(ram_error);
        proc_error = PROC_ERROR_RAM;
        goto stack_destructor;
    }
    goto out;

stack_destructor:
    stack_dtor(&(STK));
free_buffer:
    free(BUFFER);
free_memory_for_regs:
    free(proc_struct->reg);
close_input_file:
    fclose(proc_struct->input_file);
out:
    return proc_error;
}

ProcError proc_dtor(Processor *proc_struct)
{
    assert(proc_struct != NULL);
    if (proc_struct == NULL)
        return PROC_ERROR_NULL_PTR_STRUCT;

    ProcError proc_error = PROC_ERROR_OK;
    free(proc_struct->reg);
    fclose(proc_struct->input_file);
    proc_struct->file_size = 0;
    free(BUFFER);
    StkError stk_error = stack_dtor(&(STK));
    if (stk_error)
    {
        stk_print_error(stk_error);
        proc_error = PROC_ERROR_STK;
    }
    ArrError ram_error = arr_dtor(&(proc_struct->ram));
    if (ram_error)
    {
        arr_print_error(ram_error);
        proc_error = PROC_ERROR_RAM;
    }

    return proc_error;
}

const char *proc_get_error(ProcError error)
{
    switch (error)
    {
        case PROC_ERROR_OK:
            return "Processor: Ошибок в работе функций не выявлено.";
        case PROC_ERROR_FOPEN:
            return "Processor: Ошибка в работе функции fopen.";
        case PROC_ERROR_CMDS:
            return "Processor: Ошибка распознования команды.";
        case PROC_ERROR_STAT:
            return "Processor: Ошибка выполнения функции stat.";
        case PROC_ERROR_FSEEK:
            return "Processor: Ошибка выполнения функции fseek.";
        case PROC_ERROR_NULL_PTR_FILE:
            return "Processor: Нулевой указатель на файл.";
        case PROC_ERROR_CALLOC:
            return "Processor: Ошибка функции calloc.";
        case PROC_ERROR_STACK:
            return "Processor: Ошибка функции pop.";
        case PROC_ERROR_SETVBUF:
            return "Processor: Ошибка отключения буферизации.";
        case PROC_ERROR_NULL_PTR_STRUCT:
            return "Processor: Передан нулевой указатель на структуру.";
        case PROC_ERROR_PROC_FILE:
            return "Processor: Ошибка обработки файла.";
        case PROC_ERROR_STK:
            return "Processor: Ошибка стэка.";
        case PROC_ERROR_RAM:
            return "Processor: Ошибка работы в RAM.";
        case PROC_ERROR_ARG:
            return "Processor: Не найдено нужного типа аргумента.";
        default:
            return "Stack: Куда делся мой enum ошибок?";
    }
}
