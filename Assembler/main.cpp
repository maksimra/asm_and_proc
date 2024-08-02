#include "include/assembler.hpp"
#include "include/check_args.hpp"
#include "include/file_processing.hpp"
#include "include/void_stack.hpp"

#define PRINT_IF_NOT_OPEN(file, file_name) if (file == NULL) printf ("%s wasn't open.\n", file_name)

const int necessary_n_args = 2;

int main (const int argc, const char* argv[])
{
    AsmError      asm_error       = ASM_NO_ERROR;
    ArgsError     args_error      = ARGS_NO_ERROR;
    ProcFileError proc_file_error = PROC_FILE_NO_ERROR;
    StkError      stk_error       = STK_NO_ERROR;

    FILE* log_file = fopen ("log_file.txt", "w");

    PRINT_IF_NOT_OPEN (log_file, "log_file.txt");

    args_set_log_file      (log_file);  // pass NULL if you don't want
    asm_set_log_file       (log_file);  // to write to the log file
    proc_file_set_log_file (log_file);
    stack_set_log_file     (log_file);

    args_error = args_check (argc, argv, necessary_n_args);
    if (args_print_if_error (args_error))
        return EXIT_FAILURE;

    const char* name_of_input_file = argv[1];

    size_t number_of_lines = 0;
    char** ptr_to_lines = NULL;
    proc_file_error = process_file (&ptr_to_lines, name_of_input_file, &number_of_lines);
    if (proc_file_print_if_error (proc_file_error))
        return EXIT_FAILURE;

    Stack labels = {};
    stk_error = stack_ctor (&labels, sizeof(Label));
    stk_print_error (stk_error);
    asm_error = make_assem_file (&labels, ptr_to_lines, number_of_lines);
    asm_print_error (asm_error);

    asm_error = make_assem_file (&labels, ptr_to_lines, number_of_lines);
    asm_print_error (asm_error);

    labels_name_dtor (&labels);
    stk_error = stack_dtor (&labels);
    stk_print_error (stk_error);

    free (ptr_to_lines[0]);
    free (ptr_to_lines);

    fclose (log_file);
    return EXIT_SUCCESS;
}
