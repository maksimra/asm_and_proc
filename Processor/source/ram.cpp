#include "../include/ram.hpp"
#include "../include/print_in_log.hpp"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static FILE* log_file = stderr;

const size_t CAPACITY = 1000;

void ram_set_log_file (FILE* file)
{
    log_file = file;
}

RAMError ram_ctor (DynArr* dyn_arr, size_t elem_size)
{
    PRINT_BEGIN();
    assert (dyn_arr != NULL);

    if (dyn_arr == NULL)
        return RAM_ERROR_NULL_PTR_TO_STR;

    void* temp_data = calloc (CAPACITY, elem_size);
    if (temp_data == NULL)
        return RAM_ERROR_CALLOC;

    dyn_arr->data = temp_data;
    dyn_arr->elem_size = elem_size;
    PRINT_END();
    return RAM_NO_ERROR;
}

RAMError ram_push (DynArr* dyn_arr, void* element, size_t position)
{
    PRINT_BEGIN();
    assert (dyn_arr != NULL);

    if (dyn_arr == NULL)
        return RAM_ERROR_NULL_PTR_TO_STR;

    if (element == NULL)
        return RAM_ERROR_NULL_PTR_TO_ELEM;

    if (position >= CAPACITY)
        return RAM_ERROR_POSITION;

    memcpy ((char*) dyn_arr->data + position * dyn_arr->elem_size, element, dyn_arr->elem_size);
    PRINT_END();
    return RAM_NO_ERROR;
}

RAMError ram_get (DynArr* dyn_arr, void* element, size_t position)
{
    PRINT_BEGIN();
    assert (dyn_arr != NULL);

    if (dyn_arr == NULL)
        return RAM_ERROR_NULL_PTR_TO_STR;

    if (element == NULL)
        return RAM_ERROR_NULL_PTR_TO_ELEM;

    if (position >= CAPACITY)
        return RAM_ERROR_POSITION;

    memcpy (element, (char*) dyn_arr->data + position * dyn_arr->elem_size, dyn_arr->elem_size);
    PRINT_END();
    return RAM_NO_ERROR;
}

RAMError ram_dtor (DynArr* dyn_arr)
{
    PRINT_BEGIN();
    assert (dyn_arr != NULL);

    if (dyn_arr == NULL)
        return RAM_ERROR_NULL_PTR_TO_STR;

    free (dyn_arr->data);
    dyn_arr->elem_size = 0;
    PRINT_END();
    return RAM_NO_ERROR;
}

void ram_print_error (RAMError error)
{
    PRINT ("%s\n", ram_get_error (error));
}

const char* ram_get_error (RAMError error)
{
    switch (error)
    {
        case RAM_NO_ERROR:
            return "RAM: Ошибок в работе функций не выявлено.";
        case RAM_ERROR_NULL_PTR_TO_STR:
            return "RAM: Передан нулевой указатель на структуру.";
        case RAM_ERROR_NULL_PTR_TO_ELEM:
            return "RAM: Передан нулевой указатель на элемент.";
        case RAM_ERROR_CALLOC:
            return "RAM: Ошибка в выделении памяти (calloc).";
        case RAM_ERROR_POSITION:
            return "RAM: не хватило размера динамического массива.";
        default:
            return "RAM: Нужной ошибки не найдено...";
    }
}
