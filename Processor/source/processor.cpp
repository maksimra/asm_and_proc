#include "../include/processor.hpp"
#include "../include/file_processing.hpp"
#include "../include/print_in_log.hpp"

#define PRINT_AND_RETURN_IF_ERROR(error) if (error) \
                                         {\
                                            stk_print_error (error);\
                                            return PROC_ERROR_STK;\
                                         }

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
    StkError error = STK_NO_ERROR;
    stk_element reg[NUM_OF_REG] = {};
    stk_element num1 = 0;
    stk_element num2 = 0;
    for (size_t position = 0; position < proc_struct->file_size; position++)
    {
        switch (proc_struct->input_buffer[position])
        {
            case IN:
                PRINT ("case IN\n");
                printf ("Enter your double in stack.\n");
                scanf ("%lf", &num1);
                error = stack_push (&(proc_struct->stk), num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                break;
            case PUSH + NUM:
                PRINT ("case PUSH + NUM\n");
                memcpy (&num1, proc_struct->input_buffer + position + sizeof (char), sizeof (double));
                error = stack_push (&(proc_struct->stk), num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                position += sizeof (double);
                break;
            case PUSH + REG:
                PRINT ("case PUSH + REG\n");
                PRINT ("Номер регистра  = %d\n", proc_struct->input_buffer[position + sizeof (char)]);
                PRINT ("Кладу в стэк %lf\n", reg[(int) proc_struct->input_buffer[position + sizeof (char)]]);
                error = stack_push (&(proc_struct->stk), reg[(int) proc_struct->input_buffer[position + sizeof (char)]]);
                position += sizeof (char);
                PRINT_AND_RETURN_IF_ERROR(error);
                break;
            case POP + REG:
                PRINT ("case POP + REG\n");
                error = stack_pop (&(proc_struct->stk), &num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                PRINT ("Номер регистра = %d\n", proc_struct->input_buffer[position + sizeof (char)]);
                reg[(int) proc_struct->input_buffer[position + sizeof (char)]] = num1;
                position += sizeof (char);
                break;
            case ADD:
                PRINT ("case ADD\n");
                error = stack_pop (&(proc_struct->stk), &num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_pop (&(proc_struct->stk), &num2);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_push (&(proc_struct->stk), num1 + num2);
                PRINT_AND_RETURN_IF_ERROR(error);
                break;
            case SUB:
                PRINT ("case SUB\n");
                error = stack_pop (&(proc_struct->stk), &num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_pop (&(proc_struct->stk), &num2);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_push (&(proc_struct->stk), num2 - num1);
                PRINT_AND_RETURN_IF_ERROR (error);
                break;
            case MUL:
                PRINT ("case MUL\n");
                error = stack_pop (&(proc_struct->stk), &num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_pop (&(proc_struct->stk), &num2);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_push (&(proc_struct->stk), num1 * num2);
                PRINT_AND_RETURN_IF_ERROR(error);
                break;
            case DIV:
                PRINT ("case DIV\n");
                error = stack_pop (&(proc_struct->stk), &num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_pop (&(proc_struct->stk), &num2);
                PRINT_AND_RETURN_IF_ERROR(error);
                error = stack_push (&(proc_struct->stk), num2 / num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                break;
            case JMP + NUM:
                PRINT ("case JMP\n");
                memcpy (&position, proc_struct->input_buffer + position + sizeof (char), sizeof (size_t));
                position--; // костыль из-за инкрементирования, потом пофикшу (хз, правда, как)
                break;
            case CALL + NUM:
                PRINT ("case CALL\n");
                error = stack_push (&(proc_struct->stk), (double) (position + sizeof (size_t) + sizeof (char)));
                PRINT_AND_RETURN_IF_ERROR(error);
                memcpy (&position, proc_struct->input_buffer + position + sizeof (char), sizeof (size_t));
                position--;
                break;
            case RET:
                PRINT ("case RET\n");
                error = stack_pop (&(proc_struct->stk), &num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                printf ("num1 == %lf\n", num1);
                position = (size_t) num1;
                printf ("buffer[position] == %d\n", proc_struct->input_buffer[position]);
                position--;
                break;
            case OUT:
                PRINT ("case OUT\n");
                error = stack_pop (&(proc_struct->stk), &num1);
                PRINT_AND_RETURN_IF_ERROR(error);
                printf ("Результат: %lg\n", num1);
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

ProcError proc_ctor (Proc* proc_struct, const char* name_of_input_file)
{
    assert (proc_struct);
    StkError stk_error = STK_NO_ERROR;
    if (proc_struct == NULL)
        return PROC_ERROR_NULL_PTR_STRUCT;

    FILE* temp_file = fopen (name_of_input_file, "rb");
    if (temp_file == NULL)
        return PROC_ERROR_FOPEN;
    proc_struct->input_file = temp_file;

    ProcFileError proc_file_error = PROC_FILE_NO_ERROR;
    proc_file_error = read_file_count_size (name_of_input_file, temp_file, &(proc_struct->file_size), &(proc_struct->input_buffer));
    ProcError proc_error = PROC_NO_ERROR;
    if (proc_file_error)
    {
        proc_file_print_error (proc_file_error);
        proc_error = PROC_ERROR_PROC_FILE;
        goto close_input_file;
    }
    stk_error = stack_ctor (&(proc_struct->stk), BASIC_CAPACITY);
    if (stk_error)
    {
        stk_print_error (stk_error);
        proc_error = PROC_ERROR_STK;
        goto full_termination;
    }
    goto out;

full_termination:
    free (proc_struct->input_buffer);
close_input_file:
    fclose (proc_struct->input_file);
out:
    return proc_error;
}

ProcError proc_dtor (Proc* proc_struct)
{
    fclose (proc_struct->input_file);
    proc_struct->file_size = 0;
    free (proc_struct->input_buffer);
    StkError stk_error = stack_dtor (&(proc_struct->stk));
    if (stk_error)
    {
        stk_print_error (stk_error);
        return PROC_ERROR_STK;
    }
    return PROC_NO_ERROR;
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
        default:
            return "Stack: Куда делся мой enum ошибок?";
    }
}
