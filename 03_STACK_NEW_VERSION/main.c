#include "stack.h"

int main (void) {

    Stack_t* mainSt = attach_stack (10, 200);

    int childs [100] = {0};
    for (int i = 0; i < 100; i++) {

        int forkRes = fork ();
        if (forkRes == 0) {
            
            Stack_t* new = attach_stack (10, 20);
            push (new, i);
            size_t trash = 0;
            pop (new, &trash);
            detach_stack (new);
            return 0;

        }

        childs [i] = forkRes;

    }

    sleep (7);

    stack_dump (mainSt);

    mark_destruct (mainSt);
    detach_stack (mainSt);

    return 0;

}

