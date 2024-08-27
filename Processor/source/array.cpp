#include "../include/array.hpp"
#include "../include/print_in_log.hpp"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static FILE* log_file = stderr;

const size_t CAPACITY = 1000;

void arr_set_log_file (FILE* file)
{
    log_file = file;
}

ArrError arr_ctor (Arr* arr, size_t elem_size)
{
    PRINT_BEGIN();
    assert (arr != NULL);

    if (arr == NULL)
        return ARR_ERROR_NULL_PTR_TO_STR;

    void* temp_data = calloc (CAPACITY, elem_size);
    if (temp_data == NULL)
        return ARR_ERROR_CALLOC;

    arr->data = temp_data;
    arr->elem_size = elem_size;
    PRINT_END();
    return ARR_NO_ERROR;
}

ArrError arr_put (Arr* arr, void* element, size_t position)
{
    PRINT_BEGIN();
    assert (arr != NULL);

    if (arr == NULL)
        return ARR_ERROR_NULL_PTR_TO_STR;

    if (element == NULL)
        return ARR_ERROR_NULL_PTR_TO_ELEM;

    if (position >= CAPACITY)
        return ARR_ERROR_POSITION;

    memcpy ((char*) arr->data + position * arr->elem_size, element, arr->elem_size);
    PRINT_END();
    return ARR_NO_ERROR;
}

ArrError arr_get (Arr* arr, void* element, size_t position)
{
    PRINT_BEGIN();
    assert (arr != NULL);

    if (arr == NULL)
        return ARR_ERROR_NULL_PTR_TO_STR;

    if (element == NULL)
        return ARR_ERROR_NULL_PTR_TO_ELEM;

    if (position >= CAPACITY)
        return ARR_ERROR_POSITION;

    memcpy (element, (char*) arr->data + position * arr->elem_size, arr->elem_size);
    PRINT_END();
    return ARR_NO_ERROR;
}

ArrError arr_dtor (Arr* arr)
{
    PRINT_BEGIN();
    assert (arr != NULL);

    if (arr == NULL)
        return ARR_ERROR_NULL_PTR_TO_STR;

    free (arr->data);
    arr->elem_size = 0;
    PRINT_END();
    return ARR_NO_ERROR;
}

void arr_print_error (ArrError error)
{
    PRINT ("%s\n", arr_get_error (error));
}

const char* arr_get_error (ArrError error)
{
    switch (error)
    {
        case ARR_NO_ERROR:
            return "RAM: Ошибок в работе функций не выявлено.";
        case ARR_ERROR_NULL_PTR_TO_STR:
            return "RAM: Передан нулевой указатель на структуру.";
        case ARR_ERROR_NULL_PTR_TO_ELEM:
            return "RAM: Передан нулевой указатель на элемент.";
        case ARR_ERROR_CALLOC:
            return "RAM: Ошибка в выделении памяти (calloc).";
        case ARR_ERROR_POSITION:
            return "RAM: не хватило размера динамического массива.";
        default:
            return "RAM: Нужной ошибки не найдено...";
    }
}
