#include "stack.h"

int main (void) {
    
    Stack_t* mainSt = attach_stack (10, 200);

    int childs [100] = {0};
    for (int i = 0; i < 100; i++) {

        int forkRes = fork ();
        if (forkRes == 0) {
            
            Stack_t* new = attach_stack (10, 20);
            push (new, i);
            detach_stack (new);
            return 0;

        }

        childs [i] = forkRes;

    }

    for (int i = 0; i < 100; ++i)
        kill (childs [i], SIGKILL);

    sleep (7);

    stack_dump (mainSt);

    mark_destruct (mainSt);
    detach_stack (mainSt);

    return 0;

}
