#include "include/stack.hpp"
#include "include/processor.hpp"
#include "include/check_args.hpp"
#include "include/file_processing.hpp"
#include "include/array.hpp"

#define PRINT_IF_NOT_OPEN(file, file_name) if (file == NULL) fprintf (stderr, "%s wasn't open.\n", file_name)

const int necessary_n_args = 2;

int main (const int argc, const char* argv[])
{
    ProcError proc_error = PROC_ERROR_OK;
    ArgsError args_error = ARGS_NO_ERROR;

    FILE* log_file = fopen ("log_file.txt", "w");
    PRINT_IF_NOT_OPEN (log_file, "log_file.txt");
    int return_value = setvbuf (log_file, NULL, _IONBF, 0);
    if (return_value)
    {
        proc_print_error (PROC_ERROR_SETVBUF);
    }

    args_set_log_file      (log_file); // pass NULL if you don't want
    proc_set_log_file      (log_file); // to write to the log file
    stack_set_log_file     (log_file);
    proc_file_set_log_file (log_file);
    arr_set_log_file       (log_file);

    args_error = args_check (argc, argv, necessary_n_args);
    if (args_print_if_error (args_error))
    {
        fclose (log_file);
        return EXIT_FAILURE;
    }

    Processor proc_struct = {};
    const char* name_of_input_file = argv[1];
    proc_error = proc_ctor (&proc_struct, name_of_input_file);
    if (proc_error)
    {
        proc_print_error (proc_error);
        fprintf (stderr, "Error initializing processor.\n");
        fclose (log_file);
        return EXIT_FAILURE;
    }

    proc_error = proc_calculations (&proc_struct);
    if (proc_error)
    {
        proc_print_error (proc_error);
        fprintf (stderr, "Error processing.\n");
        proc_dtor (&proc_struct);
        fclose (log_file);
        return EXIT_FAILURE;
    }

    proc_error = proc_dtor (&proc_struct);
    if (proc_error)
    {
        proc_print_error (proc_error);
        fprintf (stderr, "Error dtor Proc.\n");
        fclose (log_file);
        return EXIT_FAILURE;
    }

    fclose (log_file);
    return EXIT_SUCCESS;
}
