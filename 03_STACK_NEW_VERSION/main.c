#include "stack.h"

int main (void) {

    stack_t* mainSt = attach_stack (10, 20);
    //mark_destruct (mainSt);

    for (int i = 0; i < 10; i++) {

        int forkRes = fork ();
        if (forkRes == 0) {
            
            stack_t* new = attach_stack (10, 20);
            push (new, i);
            size_t trash = 0;
            pop  (new, &trash);
            detach_stack (new);
            return 0;

        }

    }

    sleep (5);

    stack_dump (mainSt);

    mark_destruct (mainSt);
    detach_stack (mainSt);

    return 0;

}

