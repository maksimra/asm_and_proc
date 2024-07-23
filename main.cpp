#include "stack.h"
#include "processor.h"

size_t BASIC_CAPACITY = 50;

int main (int argc, char* argv[])
{
    enum Proc_error proc_error = PROC_NO_ERROR;
    enum StkError stk_error = STK_NO_ERROR;
    proc_error = check_argc(argc);
    proc_print_error (proc_error);
    struct Stack stk = {};
    struct stat statbuf = {};
    FILE* code_file = NULL;
    const char* NAME = argv[1];
    char* buffer = NULL;

    proc_error = processing_file (NAME, &buffer, &code_file, &statbuf);
    proc_print_error (proc_error); 


    stk_error = stack_ctor  (&stk, BASIC_CAPACITY);
    stk_print_error (stk_error);

    proc_error = calculations (&stk, buffer, statbuf);
    proc_print_error (proc_error);

    stack_dump (&stk);

    stk_error = stack_dtor  (&stk);
    stk_print_error (stk_error);

    free (buffer);

    return 0;
}
