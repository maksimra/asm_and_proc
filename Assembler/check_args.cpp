#include "check_args.hpp"

#define PRINT_BEGIN() fprintf (log_file, "begin: %s.\n",      __PRETTY_FUNCTION__)
#define PRINT_END()   fprintf (log_file, "success end: %s\n", __PRETTY_FUNCTION__)
#define PRINT(...) if (log_file != NULL) fprintf (log_file, __VA_ARGS__)

static FILE* log_file = stderr;

static const int necessary_n_args = 2;

void args_set_log_file (FILE* file)
{
    log_file = file;
}

ArgsError args_check (const int argc, const char** argv, int ref_argc)
{
    PRINT_BEGIN();
    if (argc != ref_argc)
        return ARGS_ERROR_ARGC;

    FILE* file = fopen (argv[1], "r"); // TODO: функция должна вместо этого
                                       // смотреть флаги. (спросить лично у Коли)
    if (file == NULL)
        return ARGS_ERROR_ARGV;

    fclose (file);
    PRINT_END();
    return ARGS_NO_ERROR;
}

void args_print_error (ArgsError error)
{
    PRINT ("%s\n", args_get_error (error));
}

const char* args_get_error (ArgsError error)
{
    switch (error)
    {
        case ARGS_NO_ERROR:
            return "Args: Ошибок в работе функций не выявлено.";
        case ARGS_ERROR_ARGC:
            return "Args: Введено некорректное число аргументов.";
        case ARGS_ERROR_ARGV:
            return "Argv: Файл не открылся (вторым аргументом должно быть имя файла).";
        default:
            return "Args: Нужной ошибки не найдено...";
    }
}
