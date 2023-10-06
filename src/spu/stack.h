#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <stdio.h>
#include <time.h>

/*
    REMEMBER TO DO FOLLOWING LINES BEFORE #include "stack.h" IN YOUR FILE:
    typedef *your_type* Elem_t
    void inline print_elem_t(FILE *stream, Elem_t val) { *your code here* }

    FOR EXAMPLE:
    typedef int Elem_t
    void inline print_elem_t(FILE *stream, Elem_t val) { fprintf(stream, "%d", val); }

*/


/*
    USED DEFINES:
    1) STACK_DO_DUMP
    2) STACK_USE_POISON
    3) STACK_ABORT_ON_DUMP
    4) STACK_DUMP_ON_INVALID_POP
    5) STACK_USE_PROTECTION_CANARY
    6) STACK_USE_PROTECTION_HASH
    7) STACK_FULL_DEBUG_INFO
*/

//--------------------------------------------------------------------------------------------

#ifdef STACK_USE_POISON
typedef unsigned char poison_t;
const poison_t POISON_VALUE = (poison_t) 0xBE;
#define POISON_T_SPECF "%X"
#endif

typedef long int stacksize_t;
#define STACKSIZE_T_SPECF "%ld"

#ifdef STACK_USE_PROTECTION_CANARY
typedef unsigned long long canary_t;
#define CANARY_T_SPECF "%llX"
const canary_t CANARY_LEFT_DEFAULT_VALUE  = 0xDEDEDED;
const canary_t CANARY_RIGHT_DEFAULT_VALUE = 0xDEDEDED;
#endif

#ifdef STACK_USE_PROTECTION_HASH
typedef long long stackhash_t;
const stackhash_t HASH_DEFAULT_VALUE = 0;
#define STACKHASH_T_SPECF "%llX"
#endif

/*
    ------------------------------------TODO--------------------------------------

*/

//! @brief Holds values returned by funcs like stack_pop(), stack_push(), stack_ctor(), etc.
//! @note If STACK_ERROR_VERIFY is returned, you can call stack_verify() on your own
//! to see details. Also STACK_DUMP() is called automtaically if NDEBUG is not defined.
enum StackErrorCode
{
    STACK_ERROR_NO_ERROR            = 0, //< No errors occurred.
    STACK_ERROR_NULL_STK_PNT_PASSED = 1, //< NULL pointer to stack was passed.
    STACK_ERROR_VERIFY              = 2, //< stack_verify() returned non-zero value.
    STACK_ERROR_NULL_RET_VALUE_PNT  = 3, //< NULL passed as a pointer to the return value.
    STACK_ERROR_MEM_BAD_REALLOC     = 4, //< Stack reallocation failed.
    STACK_ERROR_NOTHING_TO_POP      = 5, //< Stack is empty, but pop() was called.
};

//! @brief Mask consisting of values of this enum is returned by stack_verify().
//! @note size may equal capacity, but next push() will call realloc().
//! @note Data pointer equalling NULL is considered fine only if
//! size == 0 and capacity == 0!
//! @note IF YOU CHANGE THIS ENUM, DON'T FORGET TO CHANGE print_verify_res()!!!
enum StackVerifyResFlag
{
    STACK_VERIFY_NULL_PNT           = 1 << 0,  //< Passed pointer to the stack is NULL.
    STACK_VERIFY_DATA_PNT_WRONG     = 1 << 1,  //< Pointer to data is NULL and either size != 0 or capacity != 0.
    STACK_VERIFY_SIZE_INVALID       = 1 << 2,  //< Size < 0 or size > capacity.
    STACK_VERIFY_CAPACITY_INVALID   = 1 << 3,  //< Capacity < 0.
#ifdef STACK_USE_PROTECTION_CANARY
    STACK_VERIFY_CANARY_STRCUT_DMG  = 1 << 4, //< One or both canaries in struct are damaged.
    STACK_VERIFY_CANARY_DATA_DMG    = 1 << 5, //< One or both canaries in data are damaged.
#endif
#ifdef STACK_USE_PROTECTION_HASH
    STACK_VERIFY_STRUCT_HASH_INVALID= 1 << 6, //< Stack's struct hash is invalid.
    STACK_VERIFY_DATA_HASH_INVALID  = 1 << 7, //< Stack's data hash is invalid.
#endif
};
//! @note MUST BE IN SYNC WITH StackVerifyResFlag enum above!!!
const char *verification_messages[] =
{
      "1: Passed pointer to the stack is NULL.",
      "2: Pointer to data is NULL and either size != 0 or capacity != 0.",
      "4: Size < 0 or size > capacity.",
      "8: Capacity < 0.",
#ifdef STACK_USE_PROTECTION_CANARY
     "16: One or both canaries in struct are damaged.",
     "32: One or both canaries in data are damaged.",
#endif
#ifdef STACK_USE_PROTECTION_HASH
     "64: Stack's struct hash is invalid."
    "128: Stack's data hash is invalid."
#endif

};

//! @brief Gets verification result and prints corresponding error message for every error.
inline void print_verify_res(FILE *stream, int verify_res)
{
    fprintf(stream, "Stack verification result: <%d>\n", verify_res);
    for (size_t ind = 0; ind < sizeof(verification_messages)/sizeof(verification_messages[0]); ind++)
    {
        if (verify_res & ( 1 << ind ))
        {
            printf("----> %s\n", verification_messages[ind]);
        }
    }
}

struct Stack
{
#ifdef STACK_USE_PROTECTION_CANARY
    canary_t canary_left = 0;
#endif

    Elem_t *data = NULL;
    stacksize_t size = -1;
    stacksize_t capacity = -1;

#ifdef STACK_USE_PROTECTION_HASH
    stackhash_t hash_struct = HASH_DEFAULT_VALUE;
    stackhash_t hash_data = HASH_DEFAULT_VALUE;
#endif

#ifdef STACK_DO_DUMP
    const char *stack_name = NULL;
    const char *orig_file_name = NULL;
    int orig_line = -1;
    const char *orig_func_name = NULL;
#endif
    void *p_origin = NULL; // настоящий указатель на начало блока памяти, в котором лежит data

#ifdef STACK_USE_PROTECTION_CANARY
    canary_t* p_data_canary_left = NULL;
    canary_t* p_data_canary_right = NULL;

    canary_t canary_right = 0;
#endif
};

//---------------------------------------------------------------------------------------------------

//! @brief Checks stack's condition.
//! @param [in] stk Stack to check.
//! @return Mask composed from StackVerifyResFlag enum values, equaling 0 if the stack is fine.
static int stack_verify(Stack *stk);

#ifdef STACK_USE_PROTECTION_CANARY
//! @brief Check's stack's canary struct protection state.
//! @param [in] stk Stack to check.
//! @return 1 if one of struct canaries is damaged, 0 otherwise.
static int stack_is_dmgd_canary_struct_(const Stack *stk);

//! @brief Check's stack's canary data protection state.
//! @param [in] stk Stack to check.
//! @return 1 if one of data canaries is damaged, 0 otherwise.
static int stack_is_dmgd_canary_data_(const Stack *stk);
#endif

#ifdef STACK_USE_PROTECTION_HASH
//! @brief Computes hash of the stack and returns it.
static stackhash_t stack_compute_hash(char * key, unsigned int len);

//! @brief Check's stack's data hash. Returns 1 if hash is valid, 0 otherwise.
static int stack_is_hash_data_valid(Stack *stk);

//! @brief Check's stack's struct hash. Returns 1 if hash is valid, 0 otherwise.
static int stack_is_hash_struct_valid(Stack *stk);

//! @brief Recomputes stack's hash and writes the new one in the stack.
static void stack_update_hash(Stack *stk);
#endif

//---------------------------------------------------------------------------------------------------

//! @brief Stack constructor. ONLY FOR INTERNAL USE! USE MACRO stack_ctor()!
//! @details It doesn't allocate memory, setting size and capacity equalling 0. But
//! first push() will lead to realloc_up().
//! @param [in] stk Pointer to stack to construct.
//! @return StackErrorCode enum value.
StackErrorCode stack_ctor_( Stack *stk
#ifdef STACK_DO_DUMP
                            ,
                            const char *stack_name,
                            const char *orig_file_name,
                            const int orig_line,
                            const char *orig_func_name
#endif
                          );

//! @brief Stack deconstructor.
//! @param [in] stk Pointer to stack to deconstruct.
//! @return StackErrorCode enum value.
static StackErrorCode stack_dtor(Stack *stk);

//! @brief Pushes element to stack.
//! @param [in] stk Pointer to the stack.
//! @param [in] value Value to push to the stack.
//! @return StackErrorCode enu m value.
static StackErrorCode stack_push(Stack *stk, Elem_t value);

//! @brief Pops element from stack.
//! @param [in] stk Pointer to the stack.
//! @param [in] ret_value Pointer to put popped value to.
//! @return StackErrorCode enum value.
static StackErrorCode stack_pop(Stack *stk, Elem_t *ret_value);

//! @brief Checks stack's state and, if needed, reallocs memory for the stack and changes stk->data.
//! @param [in] stk Pointer to the stack.
//! @return StackErrorCode enum value.
static StackErrorCode stack_realloc(Stack *stk);

#ifndef STACK_DO_DUMP

#define STACK_DUMP(stk, verify_res) (void(0))

#else  //STACK_DO_DUMP is turned on

#define STACK_DUMP(stk, verify_res) stack_dump_( (stk), verify_res, __FILE__, __LINE__, __func__)

static void stack_dump_(Stack *stk, int verify_res, const char *file, int line, const char *func);

#endif //STACK_DO_DUMP

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------STACK.CPP---------------------------------------
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

#define STACK_CHECK(stk)    {               \
    int verify_res = stack_verify(stk);     \
    if ( verify_res != 0 ) {                \
        STACK_DUMP(stk, verify_res);        \
        return STACK_ERROR_VERIFY;          \
    }                                       \
}

int stack_verify(Stack *stk)
{
    int error = 0;

    if ( !stk )
    error |= STACK_VERIFY_NULL_PNT;

    if ( stk && !(stk->data) && (stk->size != 0 || stk->capacity != 0))
    error |= STACK_VERIFY_DATA_PNT_WRONG;

    if ( stk && (stk->size < 0 || stk->size > (stk->capacity) ) )
    error |= STACK_VERIFY_SIZE_INVALID;

    if ( stk && stk->capacity < 0)
    error |= STACK_VERIFY_CAPACITY_INVALID;

#ifdef STACK_USE_PROTECTION_CANARY
    if ( stk && stack_is_dmgd_canary_struct_(stk) )
    error |= STACK_VERIFY_CANARY_STRCUT_DMG;

    if ( stk && stk->data && stack_is_dmgd_canary_data_(stk) )
    error |= STACK_VERIFY_CANARY_DATA_DMG;
#endif

#ifdef STACK_USE_PROTECTION_HASH
    if (stk && stk->data && !stack_is_hash_struct_valid(stk))
    error |= STACK_VERIFY_STRUCT_HASH_INVALID;

    if (stk && stk->data && !stack_is_hash_data_valid(stk))
    error |= STACK_VERIFY_DATA_HASH_INVALID;
#endif

    return error;
}

#ifdef STACK_USE_PROTECTION_CANARY

int stack_is_dmgd_canary_struct_(const Stack *stk)
{
    assert(stk);

    if (stk->canary_left != CANARY_LEFT_DEFAULT_VALUE
     || stk->canary_right != CANARY_RIGHT_DEFAULT_VALUE)
     return 1;

    return 0;
}

int stack_is_dmgd_canary_data_(const Stack *stk)
{
    assert(stk);

    if (*(stk->p_data_canary_left) != CANARY_LEFT_DEFAULT_VALUE
     || *(stk->p_data_canary_right) != CANARY_RIGHT_DEFAULT_VALUE)
     return 1;

    return 0;
}

#endif

#ifdef STACK_USE_PROTECTION_HASH
stackhash_t stack_compute_hash(char *key, unsigned int len)
{
    const unsigned int m = 0x5bd1e995;
    const unsigned int seed = 0;
    const int r = 24;

    unsigned int hash = seed ^ len;

    const unsigned char *data = (const unsigned char *) key;
    unsigned int k = 0;

    while (len >= 4)
    {
        k  = data[0];
        k |= data[1] << 8;
        k |= data[2] << 16;
        k |= data[3] << 24;

        k *= m;
        k ^= k >> r;
        k *= m;

        hash *= m;
        hash ^= k;

        data += 4;
        len -= 4;
    }

    switch (len)
    {
        case 3:
        hash ^= data[2] << 16;
        // fall through
        case 2:
        hash ^= data[1] << 8;
        // fall through
        case 1:
        hash ^= data[0];
        hash *= m;
        // fall through
        default:
        break;
    };

    hash ^= hash >> 13;
    hash *= m;
    hash ^= hash >> 15;

    return hash;
}

inline stackhash_t stack_compute_hash_data_(Stack *stk)
{
    assert(stk);

    return stack_compute_hash( (char *) stk->data, (unsigned int) (stk->capacity)*sizeof(Elem_t));
}

inline stackhash_t stack_compute_hash_struct_(Stack *stk)
{
    assert(stk);

    return stack_compute_hash( (char *) stk, sizeof(*stk) );
}

int stack_is_hash_data_valid(Stack *stk)
{
    assert(stk);

    if ( stk->hash_data == stack_compute_hash_data_(stk)) return 1;
    return 0;
}

int stack_is_hash_struct_valid(Stack *stk)
{
    assert(stk);

    stackhash_t curr_hash = stk->hash_struct;
    stk->hash_struct = HASH_DEFAULT_VALUE;
    stackhash_t actual_hash = stack_compute_hash_struct_(stk);
    stk->hash_struct = curr_hash;
    if (curr_hash == actual_hash) return 1;
    return 0;
}

void stack_update_hash(Stack *stk)
{
    assert(stk);

    if (stk->data)
    {
        stk->hash_data = stack_compute_hash_data_(stk);
    }
    else
    {
        stk->hash_data = HASH_DEFAULT_VALUE;
    }

    stk->hash_struct = HASH_DEFAULT_VALUE;
    stk->hash_struct = stack_compute_hash_struct_(stk);
}
#endif


//---------------------------------------------------------------------------------------------------------------

#ifdef STACK_DO_DUMP
#define stack_ctor(stk) stack_ctor_(stk, #stk, __FILE__, __LINE__, __func__)
#else
#define stack_ctor(stk) stack_ctor_(stk)
#endif

StackErrorCode stack_ctor_( Stack *stk
#ifdef STACK_DO_DUMP
                            ,
                            const char *stack_name,
                            const char *orig_file_name,
                            const int orig_line,
                            const char *orig_func_name
#endif
                          )
{
    if (!stk) return STACK_ERROR_NULL_STK_PNT_PASSED;

    stack_dtor(stk);

    stk->data = NULL;
    stk->p_origin = NULL;
    stk->capacity = 0;
    stk->size = 0;
#ifdef STACK_DO_DUMP
    stk->stack_name = stack_name;
    stk->orig_file_name = orig_file_name;
    stk->orig_line = orig_line;
    stk->orig_func_name = orig_func_name;
#endif
#ifdef STACK_USE_PROTECTION_CANARY
    stk->canary_left = CANARY_LEFT_DEFAULT_VALUE;
    stk->canary_right = CANARY_RIGHT_DEFAULT_VALUE;
#endif

#ifdef STACK_USE_PROTECTION_HASH
    stack_update_hash(stk);
#endif
    return STACK_ERROR_NO_ERROR;
}

StackErrorCode stack_dtor(Stack *stk)
{
    if (!stk) return STACK_ERROR_NULL_STK_PNT_PASSED;

    stk->capacity = -1;
    stk->size = -1;
    if (stk->p_origin) free(stk->p_origin);
    stk->p_origin = NULL;
    stk->data = NULL;

#ifdef STACK_DO_DUMP
    stk->stack_name = NULL;
    stk->orig_file_name = NULL;
    stk->orig_line = -1;
    stk->orig_func_name = NULL;
#endif

#ifdef STACK_USE_PROTECTION_CANARY
    stk->canary_left = 0;
    stk->canary_right = 0;

    stk->p_data_canary_left = NULL;
    stk->p_data_canary_right = NULL;
#endif

#ifdef STACK_USE_PROTECTION_HASH
    stk->hash_struct = HASH_DEFAULT_VALUE;
    stk->hash_data = HASH_DEFAULT_VALUE;
#endif

    return STACK_ERROR_NO_ERROR;
}

StackErrorCode stack_push(Stack *stk, Elem_t value)
{
    STACK_CHECK(stk)

    StackErrorCode mem_realloc_res = stack_realloc(stk); // сам stack_realloc определяет, нужно ли делать realloc
    if ( mem_realloc_res )
    {
        return mem_realloc_res;
    }

    (stk->data)[(stk->size)++] = value;

#ifdef STACK_USE_PROTECTION_HASH
    stack_update_hash(stk);
#endif

    return STACK_ERROR_NO_ERROR;
}

#ifdef STACK_USE_POISON
inline void fill_with_poison_(Stack *stk, stacksize_t ind)
{
    assert(stk);
    assert(0 <= ind && ind < stk->capacity);

    poison_t *ptr = (poison_t *) (stk->data + ind);
    for (size_t i = 0; i < sizeof(Elem_t)/sizeof(poison_t); i++)
    {
        *ptr = POISON_VALUE;
        ptr += 1;
    }
}
#endif

StackErrorCode stack_pop(Stack *stk, Elem_t *ret_value)
{
    STACK_CHECK(stk)
    if ( !ret_value ) return STACK_ERROR_NULL_RET_VALUE_PNT;

    if (stk->size == 0)
    {
#ifdef STACK_DUMP_ON_INVALID_POP
        STACK_DUMP(stk, 0);
#endif
        return STACK_ERROR_NOTHING_TO_POP;
    }
    *ret_value = stk->data[--(stk->size)];

#ifdef STACK_USE_POISON
    fill_with_poison_(stk, stk->size);
#endif

#ifdef STACK_USE_PROTECTION_HASH
    stack_update_hash(stk);
#endif

    StackErrorCode mem_realloc_res = stack_realloc(stk); // сам stack_realloc определяет, нужно ли делать realloc
    if ( mem_realloc_res )
    {
        return mem_realloc_res;
    }

#ifdef STACK_USE_PROTECTION_HASH
    stack_update_hash(stk);
#endif

    return STACK_ERROR_NO_ERROR;
}

//-------------------------------------------------------------------------------------------------------

#ifdef STACK_USE_POISON
inline void fill_up_with_poison_(Stack *stk, stacksize_t start_with_index)
{
    for (stacksize_t ind = start_with_index; ind < stk->capacity; ind++)
    {
        fill_with_poison_(stk, ind);
    }
}
#endif

inline StackErrorCode stack_realloc_helper_( Stack *stk, Elem_t **new_data_p, void **p_new_origin )
{
    assert(stk);
    assert(new_data_p);
    assert(p_new_origin);

    size_t calloc_first_arg = (size_t) stk->capacity;
    size_t calloc_second_arg = sizeof(Elem_t);
#ifdef STACK_USE_PROTECTION_CANARY
    calloc_first_arg = 3 + (stk->capacity + 1)*sizeof(Elem_t) / sizeof(canary_t);
    calloc_second_arg = sizeof(canary_t);
#endif

    void *p_calloc = (void *) calloc( calloc_first_arg, calloc_second_arg );
    if (!p_calloc) return STACK_ERROR_MEM_BAD_REALLOC;

    Elem_t *new_data = (Elem_t *) p_calloc;
#ifdef STACK_USE_PROTECTION_CANARY
    stk->p_data_canary_left = (canary_t *) p_calloc;

    char *p_left_canary_end = ((char *) p_calloc) + sizeof(canary_t);

    size_t empty_space_between_left_canary_and_data = sizeof(Elem_t) - ((__PTRDIFF_TYPE__)( p_left_canary_end ) % sizeof(Elem_t));
    if ( empty_space_between_left_canary_and_data == sizeof(Elem_t) ) empty_space_between_left_canary_and_data = 0;
    new_data = (Elem_t *)(((char *) p_left_canary_end) + empty_space_between_left_canary_and_data);

    char *p_data_end = (char *)(new_data + stk->capacity);
    size_t empty_space_between_data_and_right_canary = sizeof(canary_t) - ((__PTRDIFF_TYPE__)( p_data_end ) % sizeof(canary_t));
    if (empty_space_between_data_and_right_canary == sizeof(canary_t)) empty_space_between_data_and_right_canary = 0;
    stk->p_data_canary_right = (canary_t *)(((char *) p_data_end) + empty_space_between_data_and_right_canary);

    *(stk->p_data_canary_left) = CANARY_LEFT_DEFAULT_VALUE;
    *(stk->p_data_canary_right) = CANARY_RIGHT_DEFAULT_VALUE;
#ifdef STACK_FULL_DEBUG_INFO
    printf( "Realloc hepler info:\n"
            "p_left_canary_end = %p\n"
            "empty_space_between_left_canary_and_data = %llu\n"
            "new_data = %p\n"
            "p_data_end = %p\n"
            "empty_space_between_data_and_right_canary = %llu\n"
            "p_data_canary_right = %p\n"
            "canary data left = " CANARY_T_SPECF "\n"
            "canary data right = " CANARY_T_SPECF "\n", (void *) p_left_canary_end,
                                                        empty_space_between_left_canary_and_data,
                                                        (void *) new_data,
                                                        (void *) p_data_end,
                                                        empty_space_between_data_and_right_canary,
                                                        (void *) stk->p_data_canary_right,
                                                        *(stk->p_data_canary_left),
                                                        *(stk->p_data_canary_right) );
#endif
    assert( ((__PTRDIFF_TYPE__)new_data)%sizeof(Elem_t) == 0);
    assert( ((__PTRDIFF_TYPE__) stk->p_data_canary_left )%sizeof(canary_t) == 0 );
    assert( ((__PTRDIFF_TYPE__) stk->p_data_canary_right )%sizeof(canary_t) == 0 );
    assert( (size_t) ((char *)stk->p_data_canary_right + sizeof(canary_t) - (char *)p_calloc)
        <= calloc_first_arg*calloc_second_arg );
#endif

    *new_data_p = new_data;
    *p_new_origin = p_calloc;

    return STACK_ERROR_NO_ERROR;
}

//! @brief Doubles (if MEM_MULTIPLIER == 2) the capacity of the stack, allocates new memory,
//! moves data to the new place, frees old memory. Supports case when data pointer
//! equals NULL and capacity == 0.
inline StackErrorCode stack_realloc_up_( Stack *stk, const int MEM_MULTIPLIER )
{
    if (stk->capacity == 0)
    {
        stk->capacity = 1;
    }
    stk->capacity = MEM_MULTIPLIER * stk->capacity;

    Elem_t *new_data = NULL;
    void *p_new_origin = NULL;
    if ( stack_realloc_helper_(stk, &new_data, &p_new_origin) ) return STACK_ERROR_MEM_BAD_REALLOC;
    assert(new_data);
    assert(p_new_origin);

    if (stk->data && stk->size > 0)
    {
#ifdef STACK_FULL_DEBUG_INFO
        printf("@@@ before memcpy in realloc up\n");
        for (stacksize_t ind = 0; ind < stk->capacity; ind++)
        {
            print_elem_t(stdout, stk->data[ind]);
            printf("\n");
        }
        printf("@@@\n");
#endif
        memcpy(new_data, stk->data, (stk->size)*sizeof(Elem_t));
    }
    if (stk->p_origin)
    {
        free(stk->p_origin);
    }
    stk->data = new_data;
    stk->p_origin = p_new_origin;

#ifdef STACK_USE_POISON
    fill_up_with_poison_(stk, stk->size);
#endif
#ifdef STACK_FULL_DEBUG_INFO
    printf("@@@ end of realloc up\n");
    for (stacksize_t ind = 0; ind < stk->capacity; ind++)
    {
        print_elem_t(stdout, stk->data[ind]);
        printf("\n");
    }
    printf("@@@\n");
#endif

    return STACK_ERROR_NO_ERROR;
}

//! @brief Divides by two (if MEM_MULTIPLIER == 2) the capacity of the stack,
//! allocates new memory, moves data to the new place, frees old memory.
inline StackErrorCode stack_realloc_down_(Stack *stk, const int MEM_MULTIPLIER)
{
    stk->capacity = (stk->capacity) / MEM_MULTIPLIER;

    Elem_t *new_data = NULL;
    void *p_new_origin = NULL;
    if ( stack_realloc_helper_(stk, &new_data, &p_new_origin) )
        return STACK_ERROR_MEM_BAD_REALLOC;

    assert(new_data);
    assert(p_new_origin);

#ifdef STACK_FULL_DEBUG_INFO
    printf("@@@ before memcpy in realloc down\n");
    for (stacksize_t ind = 0; ind < stk->capacity; ind++)
    {
        print_elem_t(stdout, stk->data[ind]);
        printf("\n");
    }
    printf("@@@\n");
#endif

    if (stk->size > 0) memcpy(new_data, stk->data, ((size_t) stk->size)*sizeof(Elem_t));

    free(stk->p_origin);

    stk->data = new_data;

#ifdef STACK_FULL_DEBUG_INFO
    printf("@@@ end of realloc down\n");
    for (stacksize_t ind = 0; ind < stk->capacity; ind++)
    {
        print_elem_t(stdout, stk->data[ind]);
        printf("\n");
    }
    printf("@@@\n");
#endif

    return STACK_ERROR_NO_ERROR;
}

StackErrorCode stack_realloc(Stack *stk)
{
    STACK_CHECK(stk)

    const int MEM_MULTIPLIER = 2;

    if ( stk->size >= stk->capacity )
    {
        StackErrorCode realloc_up_res = stack_realloc_up_(stk, MEM_MULTIPLIER);
        if (!realloc_up_res) return realloc_up_res;
    }
    else if ( stk->size > 0 && stk->size * ( MEM_MULTIPLIER * MEM_MULTIPLIER ) <= stk->capacity )
    {
        StackErrorCode realloc_down_res = stack_realloc_down_(stk, MEM_MULTIPLIER);
        if (!realloc_down_res) return realloc_down_res;
    }

    return STACK_ERROR_NO_ERROR;
}

//-------------------------------------------------------------------------------------------------------

#ifdef STACK_DO_DUMP

inline void stack_dump_data_( Stack *stk )
{
    fprintf(stderr, "\t{\n");

#ifdef STACK_USE_PROTECTION_CANARY
    if ( stk->p_data_canary_left )
    {
        fprintf(stderr, "\tLeft data canary[%p] = <" CANARY_T_SPECF ">\n", (void *) stk->p_data_canary_left,
                                                                            *(stk->p_data_canary_left));
    }
#endif

    for (stacksize_t ind = 0; ind < stk->capacity; ind++)
    {
        fprintf(stderr, "\t\t[" STACKSIZE_T_SPECF "][%p]\t = <", ind, (void *)(stk->data + ind));
        print_elem_t(stderr, stk->data[ind]);
        fprintf(stderr, ">");

#ifdef STACK_USE_POISON
        if (ind >= stk->size)
        {
            fprintf(stderr, " (MAYBE POISON: <" POISON_T_SPECF ">)", *((int *) stk->data + ind));
        }
#endif

        if (stk->size == ind) fprintf(stderr, " <--");

        fprintf(stderr, "\n");
    }

#ifdef STACK_USE_PROTECTION_CANARY
    if ( stk->p_data_canary_right )
    {
        fprintf(stderr, "\tRight data canary[%p] = <" CANARY_T_SPECF ">\n",
                (void *) stk->p_data_canary_right,
                *(stk->p_data_canary_right));
    }
#endif

    fprintf(stderr, "\t}\n");
}

inline void print_curr_local_time_(FILE *stream)
{
    time_t curr_time = time(NULL);
    tm curr_local_time = *localtime(&curr_time);


    fprintf(stream, "%d-%02d-%02d %02d:%02d:%02d",    curr_local_time.tm_year + 1900,
                                                        curr_local_time.tm_mon + 1,
                                                        curr_local_time.tm_mday,
                                                        curr_local_time.tm_hour,
                                                        curr_local_time.tm_min,
                                                        curr_local_time.tm_sec);
}

void stack_dump_(Stack *stk, int verify_res, const char *file, const int line, const char *func)
{
    fprintf(stderr, "STACK DUMP at ");
    print_curr_local_time_(stderr);
    fprintf(stderr, "\n");

    print_verify_res(stderr, verify_res);

    fprintf(stderr, "Stack[%p] \"%s\" declared in %s(%d), in function %s. "
                    "STACK_DUMP() called from %s(%d), from function %s.\n", (void *)    stk,
                                                                                        stk->stack_name,
                                                                                        stk->orig_file_name,
                                                                                        stk->orig_line,
                                                                                        stk->orig_func_name,
                                                                                        file, line, func);

    if (!stk)
    {
        fprintf(stderr, "Stack pointer is NULL, no further information is accessible.\n");
        return;
    }

    fprintf(stderr, "{\n");
#ifdef STACK_USE_PROTECTION_CANARY
    fprintf(stderr, "\tleft_canary = <" CANARY_T_SPECF ">\n", stk->canary_left);
    fprintf(stderr, "\tright_canary = <" CANARY_T_SPECF ">\n", stk->canary_right);
#endif
    fprintf(stderr, "\tsize = <" STACKSIZE_T_SPECF ">\n"
                    "\tcapacity = <" STACKSIZE_T_SPECF ">\n"
                    "\tdata[%p]\n", stk->size, stk->capacity, (void *) stk->data);
#ifdef STACK_USE_PROTECTION_HASH
    fprintf(stderr, "\thash_struct = <" STACKHASH_T_SPECF ">\n"
                    "\thash_data = <" STACKHASH_T_SPECF ">\n", stk->hash_struct, stk->hash_data);
#endif
    if ( !(stk->data) )
    {
        fprintf(stderr, "Data pointer is NULL. Data cannot be accessed.\n");
        return;
    }

    stack_dump_data_(stk);

    fprintf(stderr, "}\n");

#ifdef STACK_ABORT_ON_DUMP
    abort();
#endif
}

#endif // STACK_DO_DUMP

#endif // STACK_H
