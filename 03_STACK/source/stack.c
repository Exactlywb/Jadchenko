#include "stack.h"

stack_t* attach_stack (key_t key, int size) {

    FUNCTION_SECURITY (size < 0, {PRINT_ERR ("Size less than 0 isn't allowed");}, NULL);

    stack_t* stack = (stack_t*)calloc (1, sizeof (*stack));
    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Bad alloc for stack");}, NULL);

        

    return stack;

}

int detach_stack (stack_t* stack) {

    return 0;

}

