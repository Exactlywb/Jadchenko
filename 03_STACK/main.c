#include "stack.h"

int main (void) {

    stack_t* mainStack = attach_stack (30, 10);
    
    int a = 5;
    push (mainStack, &a);
    stack_dump (mainStack);
    
    void* b = NULL;
    pop (mainStack, &b);
    stack_dump (mainStack);
    printf ("POPPED: %p\n", b);

    mark_destruct (mainStack);
    detach_stack (mainStack);
    
    return 0;

}

