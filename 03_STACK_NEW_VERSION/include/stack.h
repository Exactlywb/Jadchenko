#ifndef STACK_H__
#define STACK_H__

#include "common.h"

union semun {

      int               val;        /* value for SETVAL */
      struct semid_ds   *buf;       /* buffer for IPC_STAT, IPC_SET */
      unsigned short    *array;     /* array for GETALL, SETALL */
                                    /* Linux specific part: */
      struct seminfo    *__buf;     /* buffer for IPC_INFO */

};

typedef struct Stack_t {

    size_t*     data;

    int         shmId;      //little optimization
    int         semId;      //too

    key_t       stackKey;

} Stack_t;

Stack_t*    attach_stack    (key_t key,       int       size);

int         mark_destruct   (Stack_t* stack);
int         detach_stack    (Stack_t* stack);

int         get_size        (Stack_t* stack);
int         get_count       (Stack_t* stack);

int         push            (Stack_t* stack, size_t     value);
int         pop             (Stack_t* stack, size_t*    value);

void        stack_dump      (Stack_t* stack);

#endif

