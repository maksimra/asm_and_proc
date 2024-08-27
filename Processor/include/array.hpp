#ifndef ARRAY_HPP
#define ARRAY_HPP

#include <stdio.h>
#include <stddef.h>

typedef double elem_type;

enum ArrError
{
    ARR_NO_ERROR               = 0,
    ARR_ERROR_CALLOC           = 1,
    ARR_ERROR_NULL_PTR_TO_STR  = 2,
    ARR_ERROR_NULL_PTR_TO_ELEM = 3,
    ARR_ERROR_POSITION         = 4
};

struct Arr
{
    void* data;
    size_t elem_size;
};

void        arr_set_log_file (FILE* file);
ArrError    arr_ctor         (Arr* arr, size_t elem_size);
ArrError    arr_put          (Arr* arr, void* element, size_t position);
ArrError    arr_get          (Arr* arr, void* element, size_t position);
ArrError    arr_dtor         (Arr* arr);
void        arr_print_error  (ArrError error);
const char* arr_get_error    (ArrError error);

#endif // ARRAY_HPP
