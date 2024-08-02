#include "include/assembler.hpp"
#include "include/check_args.hpp"
#include "include/file_processing.hpp"
#include "include/void_stack.hpp"

#define PRINT_IF_NOT_OPEN(file, file_name) if (file == NULL) printf ("%s wasn't open.\n", file_name)

const int necessary_n_args = 2;

int main (const int argc, const char* argv[])
{
    enum AsmError      asm_error       = ASM_NO_ERROR;
    enum ArgsError     args_error      = ARGS_NO_ERROR;
    enum ProcFileError proc_file_error = PROC_FILE_NO_ERROR;

    FILE* log_file_check_args = fopen ("log_file_check_args.txt", "w");
    FILE* log_file_asm        = fopen ("log_file_asm.txt",        "w");
    FILE* log_file_proc_file  = fopen ("log_file_proc_file.txt",  "w");
    FILE* log_file_stack      = fopen ("log_file_stack.txt",      "w");

    PRINT_IF_NOT_OPEN (log_file_stack,      "log_file_stack.txt");
    PRINT_IF_NOT_OPEN (log_file_check_args, "log_file_check_args.txt");
    PRINT_IF_NOT_OPEN (log_file_asm,        "log_file_asm.txt");
    PRINT_IF_NOT_OPEN (log_file_proc_file,  "log_file_proc_file.txt");

    args_set_log_file      (log_file_check_args);  // pass NULL if you don't want
    asm_set_log_file       (log_file_asm);         // to write to the log file
    proc_file_set_log_file (log_file_proc_file);
    stack_set_log_file     (log_file_stack);

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
    asm_error = make_assem_file (&labels, ptr_to_lines, number_of_lines);
    asm_print_error (asm_error);

    asm_error = make_assem_file (&labels, ptr_to_lines, number_of_lines);
    asm_print_error (asm_error);

    free (ptr_to_lines[0]);
    free (ptr_to_lines);

    fclose (log_file_check_args);
    fclose (log_file_asm);
    fclose (log_file_proc_file);
    return EXIT_SUCCESS;
}
