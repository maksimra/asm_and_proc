#include "assembler.hpp"

#define PRINT_BEGIN() fprintf (log_file, "begin: %s.\n",       __PRETTY_FUNCTION__)

#define PRINT_END()   fprintf (log_file, "success end: %s.\n", __PRETTY_FUNCTION__)

static FILE* log_file = stderr;

const int MAX_SYMB = 20;

const int MAX_MARKS = 10;

const int N_REGS = 4;

const size_t N_CMDS = sizeof (CMD) / sizeof (CMD[0]);

enum AsmError asm_set_log_file (FILE* file)
{
    PRINT_BEGIN();
    if (file == NULL)
        return ASM_NULL_PTR_LOG;
    log_file = file;
    PRINT_END();
    return ASM_NO_ERROR;
}

enum AsmError my_fread (size_t size, FILE *fp, char** buffer_ptr)
{
    PRINT_BEGIN();
    if (fp == NULL)
        return ASM_NULL_PTR_FILE;

    char* buffer = (char*) calloc (size + 1, sizeof (char));
    if (buffer == NULL)
        return ASM_CALLOC_FAIL;

    *buffer_ptr = buffer;
    int position = (int) ftell (fp);
    if (fseek (fp, 0, SEEK_SET) != 0)
        return ASM_ERROR_FSEEK;
    int temp = 0;
    for (size_t i = 0; i < size && (temp = fgetc (fp)) != EOF; i++)
    {
        buffer[i] = (char) temp;
        continue;
    }
    if (fseek (fp, position, SEEK_SET) != 0)
        return ASM_ERROR_FSEEK;

    PRINT_END();
    return ASM_NO_ERROR;
}

enum AsmError open_file_and_fill_stat (const char *NAME, struct stat *statbuf, FILE** file)
{
    PRINT_BEGIN();
    *file = fopen (NAME, "rb");
    if (*file == NULL)
        return ASM_ERROR_FOPEN;
    if (stat (NAME, statbuf))
        return ASM_ERROR_STAT;
    PRINT_END();
    return ASM_NO_ERROR;
}

enum AsmError ptr_to_lines (size_t size, size_t n_line, char* buffer, char*** lines)
{
    PRINT_BEGIN();
    char** temp_lines = (char**) calloc (n_line + 1, sizeof (char*));
    if (temp_lines == NULL)
    {
        return ASM_CALLOC_FAIL;
    }
    *lines = temp_lines;
    size_t a = 0;
    (*lines)[a++] = buffer;
    for (size_t k = 0; k < size; k++)
    {
        if (buffer[k] == '\0')
        {
            k += 2;
            (*lines)[a] = buffer + k;
            a++;
        }
    }
    PRINT_END();
    return ASM_NO_ERROR;
}

size_t line_processing (size_t size, char* buffer)
{
    PRINT_BEGIN();
    size_t n_line = 0;
    for (size_t k = 0 ; k < size; k++)
    {
        if (buffer[k] == '\r')
        {
            n_line++;
            buffer[k] = '\0';
            k++;
            continue;
        }
        if (buffer[k] == '\n')
        {
            n_line++;
            buffer[k] = '\0';
            continue;
        }
    }

    if (buffer[size - 1] != '\n')
        n_line++;

    PRINT_END();
    return n_line;
}

enum AsmError allocate_mem (Mark** marks, char** names_array)
{
    *marks = (Mark*) calloc (MAX_MARKS, sizeof (Mark));

    if (*marks == NULL)
        return ASM_CALLOC_FAIL;

    *names_array = (char*) calloc (MAX_MARKS, MAX_SYMB * sizeof (char));

    if (*names_array == NULL)
        return ASM_CALLOC_FAIL;

    for (size_t i = 0; i < MAX_MARKS; i++)
    {
        printf ("%zu\n", i);
        (*marks)[i].name = *names_array + i * MAX_SYMB;
    }
    return ASM_NO_ERROR;
}

int search_cmd (char* line, long size)
{
    size_t j = 0;
    while (j < N_CMDS)
    {
        if (strncmp (line, CMD[j].name, (size_t) size) != 0)
        {
            j++;
            continue;
        }
        return (int) j;
    }
    return -1;
}

int search_reg (char* line, long size)
{
    int i = 0;
    while (i < N_REGS)
    {
        if (strncmp (line, REG[i].name, (size_t) size) == 0)
        {
            return i;
        }
        i++;
    }
    return -1;
}

int search_mark (char* line, long size, Mark* marks, int n_marks)
{
    for (int i = 0; i < n_marks; i++)
    {
        if (strncmp (line, marks[i].name, (size_t) size) == 0)
        {
            return i;
        }
    }
    return -1;
}

enum AsmError assembly_file (char** lines, size_t num_line, Mark* marks, int* n_marks)
{
    PRINT_BEGIN();
    FILE *assemble_file = fopen ("assembler.txt", "wb");

    if (assemble_file == NULL)
        return ASM_ERROR_FOPEN;

    char* bin_cmd = (char*) calloc (num_line, sizeof (double) + sizeof (char));
    size_t bin_pos = 0;

    for (size_t n_line = 0; n_line < num_line; n_line++)
    {
        char* cur_line = lines[n_line];
        char* pos_space = strchr (cur_line, ' ');  // пробел может быть первым
        int n_cmd = search_cmd (cur_line, pos_space - cur_line);
        if (n_cmd != -1)
        {
            bin_cmd[bin_pos] = CMD[n_cmd].number;
            cur_line += CMD[n_cmd].length;
            if (CMD[n_cmd].has_arg)
            {
                skip_space (&cur_line);
                pos_space = strchr (cur_line, '\0');
                int n_reg = search_reg (cur_line, pos_space - cur_line);

                if (n_reg != -1)
                {
                    bin_cmd[bin_pos] |= MASK_REG;
                    bin_pos += sizeof (char);
                    bin_cmd[bin_pos] = REG[n_reg].number;
                    bin_pos += sizeof (char);
                }
                else if (isdigit (*cur_line)) //может с минуса начинаться (скормить scanf)
                {
                    bin_cmd[bin_pos] |= MASK_NUMBER;
                    bin_pos += sizeof (char);
                    double value = NAN;
                    sscanf (cur_line, "%lf", &value);
                    memcpy (bin_cmd + bin_pos, &value, sizeof (double));
                    bin_pos += sizeof (double);
                }
                else
                {
                    pos_space = strchr (cur_line, '\0');
                    int n_mark = search_mark (cur_line, pos_space - cur_line, marks, *n_marks);
                    bin_cmd[bin_pos] |= MASK_NUMBER;
                    bin_pos += sizeof (char);
                    if (n_mark != -1)
                    {
                        memcpy (bin_cmd + bin_pos, &(marks[n_mark].ip), sizeof (size_t));
                        bin_pos += sizeof (size_t);
                    }
                    else
                    {
                        size_t value = 0;
                        memcpy (bin_cmd + bin_pos, &value, sizeof (size_t));
                        bin_pos += sizeof (size_t);
                    }
                }
            }
            else
            {
                bin_pos += sizeof (char);
            }
        }
        else
        {
            char* colon = strchr (cur_line, ':');
            if (colon != NULL)
            {
                strncpy (marks[*n_marks].name, cur_line, (size_t) (colon - cur_line));
                marks[*n_marks].ip = bin_pos - 1;
                (*n_marks)++;
            }
            else
            {
                return ASM_NOT_CMD;
            }
        }
    }

    fwrite (bin_cmd, sizeof (char), bin_pos, assemble_file);
    fclose (assemble_file);
    free (bin_cmd);
    PRINT_END();
    return ASM_NO_ERROR;
}

void skip_space (char** line)
{
    for (size_t i = 0; isspace (**line); i++)
    {
        (*line)++;
    }
}

void asm_print_error (enum AsmError error)
{
    PRINT_BEGIN();
    fprintf (log_file, "%s\n", asm_get_error (error));
    PRINT_END();
}

const char* asm_get_error (enum AsmError error)
{
    PRINT_BEGIN();
    switch (error)
    {
        case ASM_NO_ERROR:
            return "Ass: Ошибок в работе функций не выявлено.";
        case ASM_NULL_PTR_FILE:
            return "Ass: Передан нулевой указатель на файл.";
        case ASM_ERROR_FOPEN:
            return "Ass: Ошибка в работе функции fopen.";
        case ASM_NULL_PTR_LOG:
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
        case ASM_CALLOC_FAIL:
            return "Ass: Ошибка в выделении памяти (calloc).";
        case ASM_NOT_CMD:
            return "Asm: Не команда.";
        case ASM_NO_MARK:
            return "Asm: Нет такого регистра :(";
        case ASM_ERROR_READ:
            return "Asm: Некорректная запись в файле.";
        case ASM_ERROR_ARGC:
            return "Asm: Введены некорректные аргументы.";
        default:
            return "Ass: Куда делся мой enum ошибок?";
    }
}


char** process_file (const char* NAME, enum AsmError* error, size_t* n_line)
{
    struct stat statbuf = {};
    FILE *source_code_file = NULL;
    *error = open_file_and_fill_stat (NAME, &statbuf, &source_code_file);
    asm_print_error (*error);
    char *buffer = NULL;
    *error = my_fread ((size_t) statbuf.st_size, source_code_file, &buffer);
    asm_print_error (*error);

    *n_line = line_processing ((size_t) statbuf.st_size, buffer);
    asm_print_error (*error);
    char** lines = NULL;
    *error = ptr_to_lines ((size_t) statbuf.st_size, *n_line, buffer, &lines);
    asm_print_error (*error);
    return lines;
}

