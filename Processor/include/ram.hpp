#ifndef RAM_HPP
#define RAM_HPP

#include <stdio.h>
#include <stddef.h>

typedef double elem_type;

enum RAMError
{
    RAM_NO_ERROR               = 0,
    RAM_ERROR_CALLOC           = 1,
    RAM_ERROR_NULL_PTR_TO_STR  = 2,
    RAM_ERROR_NULL_PTR_TO_ELEM = 3,
    RAM_ERROR_POSITION         = 4
};

struct DynArr
{
    void* data;
    size_t elem_size;
};

void        ram_set_log_file (FILE* file);
RAMError    ram_ctor         (DynArr* dyn_arr, size_t elem_size);
RAMError    ram_push         (DynArr* dyn_arr, void* element, size_t position);
RAMError    ram_get          (DynArr* dyn_arr, void* element, size_t position);
RAMError    ram_dtor         (DynArr* dyn_arr);
void        ram_print_error  (RAMError error);
const char* ram_get_error    (RAMError error);

#endif // RAM_HPP
