#include "stack.h"
#include "processor.h"

size_t BASIC_CAPACITY = 50;

int main (int argc, char* argv[])
{
    enum Proc_error proc_error = PROC_NO_ERROR;
    enum StkError stk_error = STK_NO_ERROR;
    check_argc(argc);
    struct Stack stk = {};
    struct stat statbuf = {};
    FILE *code_file = NULL;
    const char* NAME = argv[1];
    char* buffer = NULL;
    double* int_array = NULL;

    proc_error = processing_file (NAME, &buffer, &code_file, &int_array, &statbuf);
    proc_print_error (proc_error);


    stk_error = stack_ctor  (&stk, BASIC_CAPACITY);
    stk_print_error (stk_error);

    proc_error = calculations (&stk, int_array, statbuf);
    proc_print_error (proc_error);

    stack_dump (&stk);

    stk_error = stack_dtor  (&stk);
    stk_print_error (stk_error);

    return 0;
}
