#ifndef STACK_H
#define STACK_H
#define STK_CANARY_PROTECTION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#ifdef STK_CANARY_PROTECTION
    #define STK_ON_CANARY_PROTECTION(...)  __VA_ARGS__
#else
    #define STK_ON_CANARY_PROTECTION(...)
#endif

typedef double stk_element;
typedef uint64_t can_type;

enum StkError
{
    STK_NO_ERROR = 0,
    STK_REALLOC_FAIL = 1,
    STK_POP_FAIL = 2,
    STK_CALLOC_FAIL = 3,
    STK_NEGATIVE_CAPACITY = 4,
    STK_SECOND_CTOR = 5,
    STK_NULL_PTR_LOG = 6,
    STK_NULL_PTR_STK = 7,
    STK_NULL_PTR_DATA = 8,
    STK_SIZE_BIGGER_CAPACITY = 9,
    STK_NO_POISON = 10,
    STK_POISON = 11,
    STK_LEFT_CANARY_ERROR = 12,
    STK_RIGHT_CANARY_ERROR = 13,
    STK_LEFT_STR_CANARY_ERROR = 14,
    STK_RIGHT_STR_CANARY_ERROR = 15,
    STK_DATA_HASH_ERROR = 16,
    STK_STR_HASH_ERROR = 17
};

struct Stack
{
    STK_ON_CANARY_PROTECTION(can_type canary_left;)

    stk_element *data;
    size_t size;
    size_t capacity;
    uint64_t stk_hash;
    uint64_t data_hash;

    STK_ON_CANARY_PROTECTION(can_type canary_right;)
};

enum StkError          stack_ctor         (struct Stack *stk, size_t capacity);

enum StkError          stack_dtor         (struct Stack *stk);

enum StkError          stack_push         (struct Stack *stk, stk_element elem);

enum StkError          stack_pop          (struct Stack *stk, stk_element* elem);

enum StkError          stack_clear        (struct Stack *stk);

void                   stk_print_error    (enum StkError error);

void                   filler             (void* begin, size_t num_elem, const void* ptr_to_elem, size_t size_elem);

enum StkError          stack_set_log_file (FILE* file);

enum StkError          stk_resize         (struct Stack* stk, size_t new_capacity);

const char*            stk_get_error      (enum StkError error);

enum StkError          stk_verifier       (struct Stack* stk);

enum StkError          stk_realloc_down   (struct Stack *stk);

enum StkError          stk_realloc_up     (struct Stack* stk);

uint32_t               get_hash           (const uint8_t* key, size_t length);

void                   stack_dump         (struct Stack *stk);

int                    compare_doubles    (double x, double y);

#endif // STACK_H
