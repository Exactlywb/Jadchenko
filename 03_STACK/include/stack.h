#ifndef STACK_H__
#define STACK_H__

#include "common.h"

typedef struct stack_t {

    int     capacity;
    int     size;

    void**  data;

    key_t   sync_key;

} stack_t;

stack_t*    attach_stack    (key_t      key,        int     size);
int         detach_stack    (stack_t*   stack);
int         mark_destruct   (stack_t*   stack);

int         get_size        (stack_t*   stack);
int         get_count       (stack_t*   stack);

int         push            (stack_t*   stack,      void*   val);
int         pop             (stack_t*   stack,      void**  val);

int         stack_dump      (stack_t* stack);

//!TODO
//int         set_wait        (int        val,        timespec* timeout);

#endif

