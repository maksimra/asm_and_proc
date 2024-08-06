#include "../include/stack.hpp"
#include "../include/print_in_log.hpp"
#include "../include/compare_doubles.hpp"

#define STK_HASH       stk->stk_hash
#define DATA_HASH      stk->data_hash
#define TOTAL_CAPACITY new_capacity * sizeof(stk_element) + 2 * sizeof (CANARY_MAIN)
#define SIZE           stk->size
#define CAPACITY       stk->capacity

static FILE* log_file = stderr;

const size_t BASIC_CAPACITY = 50;

const stk_element POISON = 666;

const double limit = 1e-6;

const int COEFF_REALLOC_DOWN = 4;

const can_type CANARY_MAIN = 0xBADDEDBADDEDBAD;

const uint32_t hash_seed = 0xABFDDCAE;

//==================================================================================================

void stack_set_log_file (FILE* file)
{
    log_file = file;
}

StkError stack_ctor (Stack *stk, size_t capacity)
{
    PRINT_BEGIN();

    if (stk->data != NULL)
        return STK_ERROR_SECOND_CTOR;

    if (capacity <= 0)
        return STK_ERROR_NEGATIVE_CAPACITY;

    stk_resize (stk, capacity);
    SIZE = 0;
    CAPACITY = capacity;

    stk->canary_left = CANARY_MAIN;
    stk->canary_right = CANARY_MAIN;

    filler (stk->data, CAPACITY, &POISON, sizeof (POISON));

    STK_HASH = DATA_HASH = 0;
    STK_HASH = get_hash ((uint8_t*) stk, sizeof (Stack));
    DATA_HASH = get_hash ((uint8_t*) stk->data, CAPACITY * sizeof (stk_element));

    PRINT_END();
    return STK_NO_ERROR;
}

StkError stack_dtor (Stack *stk)
{
    PRINT_BEGIN();
    StkError error = STK_NO_ERROR;
    error = stk_verifier (stk);

    if (error == STK_ERROR_STR_HASH    ||
        error == STK_ERROR_LEFT_CANARY ||
        error == STK_ERROR_RIGHT_CANARY)
    {
        return error;
    }

    free ((char*) stk->data - sizeof(CANARY_MAIN));

    stk->data = NULL;
    CAPACITY = 0;
    SIZE = 0;
    STK_HASH = DATA_HASH = 0;
    PRINT_END();
    return STK_NO_ERROR;
}

void stack_dump (Stack *stk)
{
    for (size_t i = 0; i < SIZE; i++)
    {
        PRINT ("%zu: <%lf> ", i, stk->data[i]);
    }

    PRINT ("capacity = %zu\n"
           "size = %zu\n", CAPACITY, SIZE);
    PRINT ("left canary = 0x%jx\n",  *(can_type*)((char*) stk->data - sizeof (can_type)));
    PRINT ("right canary = 0x%jx\n", *(can_type*)((char*) stk->data + CAPACITY * sizeof (stk_element)));
    PRINT ("struct hash = 0x%jx\n"
           "data hash = 0x%jx\n", STK_HASH, DATA_HASH);
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

StkError stack_push (Stack *stk, stk_element elem)
{
    PRINT_BEGIN();
    StkError error = STK_NO_ERROR;
    error = stk_verifier (stk);
    if (error != STK_NO_ERROR)
        return error;
    error = stk_realloc_up (stk);
    if (error != STK_NO_ERROR)
        return error;
    stk->data[SIZE] = elem;
    ++(SIZE);
    STK_HASH = DATA_HASH = 0;
    STK_HASH = get_hash ((uint8_t*) stk, sizeof (Stack));
    DATA_HASH = get_hash ((uint8_t*) stk->data, CAPACITY * sizeof (stk_element));
    PRINT_END();
    return STK_NO_ERROR;
}

StkError stk_realloc_up (Stack* stk)
{
    PRINT_BEGIN();
    if (SIZE >= CAPACITY)
    {
        return stk_resize (stk, CAPACITY * 2);
    }
    return STK_NO_ERROR;
}

StkError stk_resize (Stack* stk, size_t new_capacity)
{
    PRINT_BEGIN();
    stk_element* temp = NULL;
    if (stk->data == NULL)
    {
        temp = (stk_element*) calloc (1, TOTAL_CAPACITY);

        if (temp == NULL)
            return STK_ERROR_CALLOC;

        set_left_canary (temp);
        set_right_canary (temp, new_capacity);
        set_data (stk, temp);
    }
    else
    {
        *(can_type*) (stk->data + CAPACITY) = 0;

        temp = (stk_element*) realloc ((char*) stk->data - sizeof (CANARY_MAIN),
                                        TOTAL_CAPACITY);

        if (temp == NULL)
            return STK_ERROR_REALLOC;

        set_data (stk, temp);
        set_right_canary (temp, new_capacity);
    }
    CAPACITY = new_capacity;
    filler (stk->data + SIZE, CAPACITY - SIZE, &POISON, sizeof (POISON));
    PRINT_END();
    return STK_NO_ERROR;
}

void set_left_canary (stk_element* new_ptr)
{
    *(can_type*) new_ptr = CANARY_MAIN;
}

void set_right_canary (stk_element* new_ptr, size_t new_capacity)
{
    *(can_type*)((char*) new_ptr + sizeof (CANARY_MAIN) + new_capacity * sizeof(stk_element)) = CANARY_MAIN;
}

void set_data (Stack* stk, stk_element* new_ptr)
{
    stk->data = (stk_element*)((char*) new_ptr + sizeof (CANARY_MAIN));
}

StkError stack_clear (Stack *stk)
{
    PRINT_BEGIN();
    filler (stk->data, CAPACITY, &POISON, sizeof (POISON));
    SIZE = 0;
    STK_HASH = DATA_HASH = 0;
    STK_HASH = get_hash ((uint8_t*) stk, sizeof (Stack));
    DATA_HASH = get_hash ((uint8_t*) stk->data, CAPACITY * sizeof (stk_element));
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

StkError stack_pop (Stack *stk, stk_element* elem)
{
    PRINT_BEGIN();
    StkError error = STK_NO_ERROR;
    error = stk_verifier (stk);
    if (error != STK_NO_ERROR)
        return error;

    if (SIZE <= 0)
        return STK_ERROR_POP;

    stk_element* ptr_to_last_element = stk->data + SIZE - 1;

    if (elem != NULL)
        *elem = *ptr_to_last_element;

    *ptr_to_last_element = POISON;
    --(SIZE);
    error = stk_realloc_down (stk);
    if (error != STK_NO_ERROR)
        return error;
    STK_HASH = DATA_HASH = 0;
    STK_HASH = get_hash ((uint8_t*) stk, sizeof (Stack));
    DATA_HASH = get_hash ((uint8_t*) stk->data, CAPACITY * sizeof (stk_element));
    PRINT_END();
    return STK_NO_ERROR;
}

StkError stk_realloc_down (Stack *stk)
{
    PRINT_BEGIN();
    if (SIZE < CAPACITY / COEFF_REALLOC_DOWN)
    {
        return stk_resize (stk, CAPACITY / 2);
    }
    PRINT_END();
    return STK_NO_ERROR;
}

void stk_print_error (StkError error)
{
    PRINT ("%s\n", stk_get_error (error));
}

StkError stk_verifier (Stack* stk)
{
    PRINT_BEGIN();
    if (stk == NULL)
        return STK_ERROR_NULL_PTR_STK;

    if (CAPACITY <= 0)
        return STK_ERROR_NEGATIVE_CAPACITY;

    if (stk->data == NULL)
        return STK_ERROR_NULL_PTR_DATA;

    if (stk->canary_left != CANARY_MAIN)
        return STK_ERROR_LEFT_STR_CANARY;

    if (stk->canary_right != CANARY_MAIN)
        return STK_ERROR_RIGHT_STR_CANARY;

    if (SIZE > CAPACITY)
        return STK_ERROR_SIZE_BIGGER_CAPACITY;

    for (size_t i = 0; i < CAPACITY - SIZE; i++)
        if (compare_doubles (stk->data[SIZE + i], POISON))
            return STK_ERROR_NO_POISON;

    for (size_t i = 0; i < SIZE; i++)
        if (compare_doubles (stk->data[i], POISON) == 0)
            return STK_ERROR_POISON;

    if (*((can_type*) stk->data - 1) != CANARY_MAIN)
        return STK_ERROR_LEFT_CANARY;

    if (*(can_type*) (stk->data + CAPACITY) != CANARY_MAIN)
        return STK_ERROR_RIGHT_CANARY;

    uint32_t prev_stk_hash = (uint32_t) STK_HASH;
    uint32_t prev_data_hash = (uint32_t) DATA_HASH;
    STK_HASH = DATA_HASH = 0;

    if (prev_stk_hash != get_hash ((uint8_t*) stk, sizeof (Stack)))
    {
        return STK_ERROR_STR_HASH;
    }

    if (prev_data_hash != get_hash ((uint8_t*) stk->data, CAPACITY * sizeof (stk_element)))
    {
        return STK_ERROR_DATA_HASH;
    }

    STK_HASH = prev_stk_hash;
    DATA_HASH = prev_data_hash;

    return STK_NO_ERROR;

}

const char* stk_get_error (StkError error)
{
    switch (error)
    {
        case STK_NO_ERROR:
            return "Stack: Ошибок в работе функций не выявлено.";
        case STK_ERROR_REALLOC:
            return "Stack: Ошибка в работе функции realloc.";
        case STK_ERROR_POP:
            return "Stack: Ошибка в работе функции pop.";
        case STK_ERROR_CALLOC:
            return "Stack: Ошибка в выделении памяти (calloc).";
        case STK_ERROR_NEGATIVE_CAPACITY:
            return "Stack: Отрицательное значение capacity.";
        case STK_ERROR_SECOND_CTOR:
            return "Stack: Stack уже был создан.";
        case STK_ERROR_NULL_PTR_DATA:
            return "Stack: Нулевой указатель stk->data";
        case STK_ERROR_NULL_PTR_STK:
            return "Stack: Передан нулевой указатель на стэк.";
        case STK_ERROR_SIZE_BIGGER_CAPACITY:
            return "Stack: Size больше, чем capacity.";
        case STK_ERROR_NO_POISON:
            return "Stack: Пустые ячейки не POISON.";
        case STK_ERROR_POISON:
            return "Stack: Ячейка(-и) POISON.";
        case STK_ERROR_LEFT_CANARY:
            return "Stack: Кто-то съел левую канарейку.";
        case STK_ERROR_RIGHT_CANARY:
            return "Stack: Кто-то съел правую канарейку.";
        case STK_ERROR_LEFT_STR_CANARY:
            return "Stack: Испортили левую канарейку структуры.";
        case STK_ERROR_RIGHT_STR_CANARY:
            return "Stack: Испортили правую канарейку структуры.";
        case STK_ERROR_STR_HASH:
            return "Stack: Повреждён хэш структуры.";
        case STK_ERROR_DATA_HASH:
            return "Stack: Повреждён хэш data.";
        default:
            return "Stack: Нужной ошибки не найдено...";
    }
}
