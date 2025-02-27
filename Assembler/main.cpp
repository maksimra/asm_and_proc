#include "include/assembler.hpp"
#include "include/check_args.hpp"
#include "include/file_processing.hpp"
#include "include/void_stack.hpp"

int main(const int argc, const char *argv[])
{
    AsmError asm_error = ASM_ERROR_OK;
    ArgsError args_error = ARGS_NO_ERROR;
    const int necessary_n_args = 3;

    FILE *log_file = fopen("log_file.txt", "w");
    if (log_file == NULL)
        fprintf(stderr, "log_file wasn't open.\n");

    int return_value = setvbuf(log_file, NULL, _IONBF, 0);
    if (return_value)
    {
        asm_print_error(ASM_ERROR_SETVBUF);
    }

    args_set_log_file(log_file); // pass NULL if you don't want
    asm_set_log_file(log_file);  // to write to the log file
    proc_file_set_log_file(log_file);
    stack_set_log_file(log_file);

    args_error = args_check(argc, argv, necessary_n_args);
    if (args_print_if_error(args_error))
    {
        fclose(log_file);
        return EXIT_FAILURE;
    }

    Assembler asm_struct = {};
    const char *name_of_input_file = argv[1];
    const char *name_of_output_file = argv[2];
    asm_error = asm_ctor(&asm_struct, name_of_input_file, name_of_output_file);
    if (asm_error)
    {
        asm_print_error(asm_error);
        fprintf(stderr, "Error initializing assembler.\n");
        fclose(log_file);
        return EXIT_FAILURE;
    }

    asm_error = asm_assembly(&asm_struct);
    if (asm_error)
    {
        asm_print_error(asm_error);
        fprintf(stderr, "Assembly error.\n");
        asm_dtor(&asm_struct);
        fclose(log_file);
        return EXIT_FAILURE;
    }

    asm_error = asm_dtor(&asm_struct);
    if (asm_error)
    {
        asm_print_error(asm_error);
        fprintf(stderr, "Error dtor Assembler.\n");
        fclose(log_file);
        return EXIT_FAILURE;
    }

    fclose(log_file);
    return EXIT_SUCCESS;
}
