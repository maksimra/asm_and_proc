#include "assembler.hpp"
#include "check_args.hpp"

int main (const int argc, const char* argv[])
{
    enum AsmError asm_error = ASM_NO_ERROR;
    enum ArgsError args_error = ARGS_NO_ERROR;

    args_error = check_args (argc, argv);
    const char *NAME = argv[1];

    size_t line_counter = 0;
    Mark* marks = NULL;
    int n_marks = 0;
    char* names_array = NULL;

    char** lines = process_file (NAME, &error, &line_counter);
    allocate_mem (&marks, &names_array);
    asm_print_error (error);


    error = assembly_file (lines, line_counter, marks, &n_marks);
    asm_print_error (error);

    error = assembly_file (lines, line_counter, marks, &n_marks);
    asm_print_error (error);

    free (marks);
    free (names_array);
    free (*lines);
    free (lines);
    return 0;
}
