#include "stack.h"

//==================================================================================================

#define PRINT_BEGIN() fprintf (log_file, "begin: %s.\n", __PRETTY_FUNCTION__)

#define PRINT_END() fprintf (log_file, "success end: %s\n", __PRETTY_FUNCTION__)

//==================================================================================================

static FILE* log_file = stderr;

//==================================================================================================

const size_t BASIC_CAPACITY = 50;

const stk_element POISON = 666;

const int COEFF_REALLOC_DOWN = 4;

const can_type CANARY_MAIN = 0xBADDEDBADDEDBAD;

const uint32_t hash_seed = 0xABFDDCAE;

//==================================================================================================

enum StkError stack_set_log_file (FILE* file)
{
    if (file == NULL)
        return STK_NULL_PTR_LOG;
    log_file = file;
    return STK_NO_ERROR;
}

enum StkError stack_ctor (struct Stack *stk, size_t capacity)
{
    PRINT_BEGIN();

    if (stk->data != NULL)
        return STK_SECOND_CTOR;

    if (capacity <= 0)
        return STK_NEGATIVE_CAPACITY;

    stk_resize (stk, capacity);
    stk->size = 0;
    stk->capacity = capacity;

    stk->canary_left = CANARY_MAIN;
    stk->canary_right = CANARY_MAIN;

    filler (stk->data, stk->capacity, &POISON, sizeof (POISON));

    stk->stk_hash = stk->data_hash = 0;
    stk->stk_hash = get_hash ((uint8_t*) stk, sizeof (Stack));
    stk->data_hash = get_hash ((uint8_t*) stk->data, stk->capacity * sizeof (stk_element));

    PRINT_END();
    return STK_NO_ERROR;
}

enum StkError stack_dtor (struct Stack *stk)
{
    PRINT_BEGIN();
    enum StkError error = STK_NO_ERROR;
    error = stk_verifier (stk);

    if (error == STK_STR_HASH_ERROR    ||
        error == STK_LEFT_CANARY_ERROR ||
        error == STK_RIGHT_CANARY_ERROR)
    {
        return error;
    }

    free ((char*) stk->data - sizeof(CANARY_MAIN)); // пустые строки
    stk->data = NULL;
    stk->capacity = 0;
    stk->size = 0;
    stk->stk_hash = stk->data_hash = 0;
    PRINT_END();
    return STK_NO_ERROR;
}

void stack_dump (struct Stack *stk)
{
    for (size_t i = 0; i < stk->size; i++)
    {
        fprintf (log_file, "%zu: <%lf> ", i, stk->data[i]);
    }

    fprintf (log_file, "capacity = %zu\n"
                       "size = %zu\n", stk->capacity, stk->size);
    fprintf (log_file, "left canary = 0x%jx\n", *(can_type*)((char*) stk->data - sizeof (can_type))); // define or inline func
    fprintf (log_file, "right canary = 0x%jx\n", *(can_type*)((char*) stk->data + stk->capacity * sizeof (stk_element)));
    fprintf (log_file, "struct hash = 0x%jx\n"
                       "data hash = 0x%jx\n", stk->data_hash, stk->stk_hash);
}

uint32_t get_hash (const uint8_t* key, size_t length)
{
    size_t i = 0;
    uint32_t hash = 0;
    while (i != length)
    {
        hash += key[i++];
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash;
}

enum StkError stack_push (struct Stack *stk, stk_element elem)
{
    PRINT_BEGIN();
    enum StkError error = STK_NO_ERROR;
    error = stk_verifier (stk);
    if (error != STK_NO_ERROR)
        return error;
    error = stk_realloc_up (stk);
    if (error != STK_NO_ERROR)
        return error;
    *(stk->data + stk->size) = elem;
    ++(stk->size);
    stk->stk_hash = stk->data_hash = 0;
    stk->stk_hash = get_hash ((uint8_t*) stk, sizeof (Stack));
    stk->data_hash = get_hash ((uint8_t*) stk->data, stk->capacity * sizeof (stk_element));
    PRINT_END();
    return STK_NO_ERROR;
}

enum StkError stk_realloc_up (struct Stack* stk)
{
    PRINT_BEGIN();
    if (stk->size >= stk->capacity)
    {
        return stk_resize (stk, stk->capacity * 2);
    }
    return STK_NO_ERROR;
}

enum StkError stk_resize (struct Stack* stk, size_t new_capacity)
{
    PRINT_BEGIN();
    stk_element* temp = NULL;
    if (stk->data == NULL)
    {
        temp = (stk_element*) calloc (1, new_capacity * sizeof(stk_element) + 2 * sizeof (CANARY_MAIN));

        if (temp == NULL)
            return STK_REALLOC_FAIL;

        *(can_type*) temp = CANARY_MAIN;
        *(can_type*)((char*) temp + sizeof (CANARY_MAIN) + new_capacity * sizeof(stk_element)) = CANARY_MAIN;
        stk->data = (stk_element*)((char*) temp + sizeof (CANARY_MAIN));
    }
    else
    {
        *(can_type*) ((stk_element*) stk->data + stk->capacity) = 0;

        temp = (stk_element*) realloc ((char*) stk->data - sizeof (CANARY_MAIN),
                                        new_capacity * sizeof(stk_element) + 2 * sizeof (CANARY_MAIN));

        if (temp == NULL)
            return STK_REALLOC_FAIL;

        stk->data = (stk_element*)((char*) temp + sizeof (CANARY_MAIN));
        *(can_type*) ((stk_element*) stk->data + new_capacity) = CANARY_MAIN;
    }
    stk->capacity = new_capacity; // сделать обертку (единообразно)
    filler (stk->data + stk->size, stk->capacity - stk->size, &POISON, sizeof (POISON));
    PRINT_END();
    return STK_NO_ERROR;
}

enum StkError stack_clear (struct Stack *stk)
{
    PRINT_BEGIN();
    filler (stk->data, stk->capacity, &POISON, sizeof (POISON));
    stk->size = 0;
    stk->stk_hash = stk->data_hash = 0;
    stk->stk_hash = get_hash ((uint8_t*) stk, sizeof (Stack));
    stk->data_hash = get_hash ((uint8_t*) stk->data, stk->capacity * sizeof (stk_element));
    PRINT_END();
    return STK_NO_ERROR;
}

void filler (void* begin, size_t num_elem, const void* ptr_to_elem, size_t size_elem)
{
    PRINT_BEGIN();
    for (size_t i = 0; i < num_elem; i++)
    {
        memcpy ((char*) begin + i * size_elem, ptr_to_elem, size_elem);
    }
    PRINT_END();
}

enum StkError stack_pop (struct Stack *stk, stk_element* elem)
{
    PRINT_BEGIN();
    enum StkError error = STK_NO_ERROR;
    error = stk_verifier (stk);
    if (error != STK_NO_ERROR)
        return error; // define в лог чтобы писал

    if (stk->size <= 0)
        return STK_POP_FAIL;

    stk_element* ptr_to_last_element = stk->data + stk->size - 1;

    if (elem != NULL)
        *elem = *ptr_to_last_element;

    *ptr_to_last_element = POISON;
    --(stk->size);
    error = stk_realloc_down (stk);
    if (error != STK_NO_ERROR)
        return error;
    stk->stk_hash = stk->data_hash = 0;
    stk->stk_hash = get_hash ((uint8_t*) stk, sizeof (Stack));
    stk->data_hash = get_hash ((uint8_t*) stk->data, stk->capacity * sizeof (stk_element));
    PRINT_END();
    return STK_NO_ERROR;
}

enum StkError stk_realloc_down (struct Stack *stk)
{
    PRINT_BEGIN();
    if (stk->size < stk->capacity / COEFF_REALLOC_DOWN)
    {
        return stk_resize (stk, stk->capacity / 2);
    }
    return STK_NO_ERROR;
}

void stk_print_error (enum StkError error)
{
    PRINT_BEGIN();
    fprintf (log_file, "%s\n", stk_get_error (error));
    PRINT_END();
}

enum StkError stk_verifier (struct Stack* stk)
{
    PRINT_BEGIN();
    if (stk == NULL)
        return STK_NULL_PTR_STK;

    if (stk->capacity <= 0)
        return STK_NEGATIVE_CAPACITY;

    if (stk->data == NULL)
        return STK_NULL_PTR_DATA;

    if (stk->canary_left != CANARY_MAIN)
        return STK_LEFT_STR_CANARY_ERROR;

    if (stk->canary_right != CANARY_MAIN)
        return STK_RIGHT_STR_CANARY_ERROR;

    if (stk->size > stk->capacity)
        return STK_SIZE_BIGGER_CAPACITY;

    for (size_t i = 0; i < stk->capacity - stk->size; i++)
        if (stk->data[stk->size + i] != POISON)
            return STK_NO_POISON;

    for (size_t i = 0; i < stk->size; i++)
        if (stk->data[i] == POISON)
            return STK_POISON;

    if (*((can_type*) stk->data - 1) != CANARY_MAIN)
        return STK_LEFT_CANARY_ERROR;

    if (*(can_type*) ((stk_element*) stk->data + stk->capacity) != CANARY_MAIN)
        return STK_RIGHT_CANARY_ERROR;
    uint32_t prev_stk_hash = stk->stk_hash;
    uint32_t prev_data_hash = stk->data_hash;
    stk->stk_hash = stk->data_hash = 0;
    if (prev_stk_hash != get_hash ((uint8_t*) stk, sizeof (Stack)))
    {
        return STK_STR_HASH_ERROR;
    }

    if (prev_data_hash != get_hash ((uint8_t*) stk->data, stk->capacity * sizeof (stk_element)))
    {
        return STK_DATA_HASH_ERROR;
    }

    stk->stk_hash = prev_stk_hash;
    stk->data_hash = prev_data_hash;

    return STK_NO_ERROR;

}

const char* stk_get_error (enum StkError error)
{
    switch (error)
    {
        case STK_NO_ERROR:
            return "Stack: Ошибок в работе функций не выявлено.";
        case STK_REALLOC_FAIL:
            return "Stack: Ошибка в работе функции realloc.";
        case STK_POP_FAIL:
            return "Stack: Ошибка в работе функции pop.";
        case STK_CALLOC_FAIL:
            return "Stack: Ошибка в выделении памяти (calloc).";
        case STK_NEGATIVE_CAPACITY:
            return "Stack: Отрицательное значение capacity.";
        case STK_SECOND_CTOR:
            return "Stack: Stack уже был создан.";
        case STK_NULL_PTR_LOG:
            return "Stack: Вместо адреса log_file передан нулевой указатель.";
        case STK_NULL_PTR_DATA:
            return "Stack: Нулевой указатель stk->data";
        case STK_NULL_PTR_STK:
            return "Stack: Передан нулевой указатель на стэк.";
        case STK_SIZE_BIGGER_CAPACITY:
            return "Stack: Size больше, чем capacity.";
        case STK_NO_POISON:
            return "Stack: Пустые ячейки не POISON.";
        case STK_POISON:
            return "Stack: Ячейка(-и) POISON.";
        case STK_LEFT_CANARY_ERROR:
            return "Stack: Кто-то съел левую канарейку.";
        case STK_RIGHT_CANARY_ERROR:
            return "Stack: Кто-то съел правую канарейку.";
        case STK_LEFT_STR_CANARY_ERROR:
            return "Stack: Испортили левую канарейку структуры.";
        case STK_RIGHT_STR_CANARY_ERROR:
            return "Stack: Испортили правую канарейку структуры.";
        case STK_STR_HASH_ERROR:
            return "Stack: Повреждён хэш структуры.";
        case STK_DATA_HASH_ERROR:
            return "Stack: Повреждён хэш data.";
        default:
            return "Stack: Куда делся мой enum ошибок?";
    }
}
