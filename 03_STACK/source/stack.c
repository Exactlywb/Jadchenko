#include "stack.h"

static int set_sync_key (stack_t* stack, key_t key) {
    
    FUNCTION_SECURITY (IS_NULL (stack), {perror ("Nullptr on stack_t* stack");}, -1);

    stack->sync_key = key;

    return 0;

}

stack_t* attach_stack (key_t key, int size) {

    FUNCTION_SECURITY (size < 0, {PRINT_ERR ("Size less than 0 isn't allowed");}, NULL);
    FUNCTION_SECURITY (key == -1, {PRINT_ERR ("Key value can't be equal with -1");}, NULL);

    stack_t* stack = NULL;

    int fullSize = sizeof (stack_t) + size * sizeof (void*);

    int id = shmget (key, fullSize, IPC_CREAT | IPC_EXCL | 0666);
    if (id == -1) {

        printf ("Stack is already created. Sharing with current process...\n");
 
        id = shmget (key, fullSize, 0666);
        FUNCTION_SECURITY (id == -1, {perror ("Bad shmget");}, NULL);        

        stack = shmat (id, NULL, 0);
        FUNCTION_SECURITY (stack == (void*)(-1), {perror ("Bad shmat ()");}, NULL); //really strange handling

        stack->data = (void**)(stack + 1);

    } else {

        printf ("Creating new stack...\n");
    
        id = shmget (key, fullSize, IPC_CREAT | 0666);
        
        stack = shmat (id, NULL, 0);
        FUNCTION_SECURITY (stack == (void*)(-1), {perror ("Bad shmat ()");}, NULL);
        
        stack->capacity = size;
        stack->size = 0;

        stack->data = (void**)(stack + 1);

        FUNCTION_SECURITY (set_sync_key (stack, key) != 0, {}, NULL);

    }

    return stack;

}

int get_size (stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Nullptr on stack");}, -1);
    return stack->size;

}

int get_count (stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Nullptr on stack");}, -1);
    return stack->capacity;

}

int detach_stack (stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Nullptr on stack_t* stack");}, -1);

    int detachRes = shmdt (stack);
    FUNCTION_SECURITY (detachRes == -1, {perror ("Bad shmdt ()");}, errno);

    return detachRes;

}

int mark_destruct (stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Nullptr on stack_t* stack");}, -1);

    int key = stack->sync_key;
    FUNCTION_SECURITY (key == -1, {PRINT_ERR ("Bad key in stack_t* stack");}, -1);

    int id = shmget (key, 0, 0);
    FUNCTION_SECURITY (id == -1, {perror ("Bad shmget ()");}, errno);

    FUNCTION_SECURITY (shmctl (id, IPC_RMID, 0) == -1, {perror ("Bad shmctl ()");}, errno);

    return 0;

}

int stack_dump (stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Nullptr on stack_t* stack");}, -1);

    printf ("\n// # STACK_DUMP:\n");
    
    printf ("Stack ptr: %p\n"           , stack);
    printf ("Stack capacity: %d\n"      , stack->capacity);
    printf ("Stack current size: %d\n"  , stack->size);

    printf ("Stack data: %p\n"          , stack->data);

    printf ("Stack sync_key: %d\n"      , stack->sync_key);

    for (int i = 0; i < stack->size; ++i)
        printf ("[%d]: %p\n", i, stack->data [i]);

    printf ("\n// #\n");

    return 0;

}

static int open_stack_semaphores (key_t key, int* answSemId) {

    int semId = semget (key, 1, IPC_CREAT | IPC_EXCL | 0666);
    
    if (semId > 0 && errno != EEXIST) {

        semId = semget (key, 1, IPC_CREAT | 0666);
        FUNCTION_SECURITY (semId == -1, {perror ("Bad semget ()");}, errno);

        int semErr = semctl (semId, 0, SETVAL, 1);
        FUNCTION_SECURITY (semErr == -1, {perror ("Bad semctl ()");}, errno);

    } else if (errno == EEXIST) {

        semId = semget (key, 1, 0666);
        FUNCTION_SECURITY (semId == -1, {perror ("Bad semget ()");}, errno);

    } else {

        perror ("Bad semget ()");
        return errno;

    }

    *answSemId = semId;

    return 0;

}

#define V(key)                                                                              \
                    int semId = 0;                                                          \
                    open_stack_semaphores (key, &semId);                                    \
                    struct sembuf curSem = {0, -1, SEM_UNDO};                               \
                                                                                            \
                    int semErr = semop (semId, &curSem, 1);                                 \
                    FUNCTION_SECURITY (semErr == -1, {perror ("Bad semop ()");}, errno);

#define P(key)                                                                              \
                    curSem.sem_op = 1;                                                      \
                    semErr = semop (semId, &curSem, 1);                                     \
                    FUNCTION_SECURITY (semErr == -1, {perror ("Bad semop ()");}, errno);

int push (stack_t* stack, void* val) {
    
    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Nullptr on stack_t* stack");}, -1);

    int key = stack->sync_key;
    FUNCTION_SECURITY (key < 0, {PRINT_ERR ("Bad key in stack");}, -1);

    V (key);

    //*
    //* #critical section
    //* 
    int size        = stack->size;
    int capacity    = stack->capacity;
    FUNCTION_SECURITY (size >= capacity, {  PRINT_ERR ("Stack is overflowed.");
                                            P (key); }, -1);

    stack->data [(stack->size)++] = val;

    P (key);    

    return 0;

}

int pop (stack_t* stack, void** val) {

    FUNCTION_SECURITY (IS_NULL (stack), {PRINT_ERR ("Nullptr on stack_t* stack");}, -1);

    int key = stack->sync_key;
    FUNCTION_SECURITY (key < 0, {PRINT_ERR ("Bad key in stack");}, -1);

    V (key);
    
    //*
    //* #critical section
    //*
    FUNCTION_SECURITY (stack->size <= 0, {  PRINT_ERR ("Can't pop from empty stack");
                                            P (key);}, -1);
    
    (*val) = stack->data [--stack->size];

    P (key);

    return 0;

}

