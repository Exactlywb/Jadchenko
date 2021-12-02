#include "stack.h"

#define STACK_SIZE_CELL     stack->data [0]
#define STACK_MAX_SIZE_CELL stack->data [1]

#define REAL_DATA           (stack->data + 2)

#define N_SEMS              3

static  int     get_sem_set     (const key_t key);
static  int     delete_sem_set  (const key_t key);

static  int     free_shared_mem (const key_t key, const int size);

Stack_t* attach_stack (key_t key, int size) {

    FUNCTION_SECURITY (key == -1, {}, NULL);
    FUNCTION_SECURITY (size <= 0, {}, NULL);

    Stack_t* stack = calloc (1, sizeof (*stack));
    FUNCTION_SECURITY  (IS_NULL (stack), 
                        {printf ("Bad memory allocate in function %s\n", __func__);}, 
                        NULL);
    
    stack->semId = get_sem_set (key);
    FUNCTION_SECURITY (stack->semId == -1, {free (stack);}, NULL);

    int shared_id = shmget (key, (size + 2) * sizeof (size_t), IPC_CREAT | IPC_EXCL | 0666);
    if (shared_id == -1) { //old stack

        printf ("attaching old stack...\n");

        shared_id = shmget (key, (size + 2) * sizeof (size_t), 0666);
        FUNCTION_SECURITY (shared_id == -1, {free (stack); 
                                             perror ("bad shmget ()");}, NULL);

        stack->data = (size_t*)shmat (shared_id, NULL, 0);
        FUNCTION_SECURITY (stack->data == (void*)-1, {free (stack);
                                                      perror ("bad shmat ()");}, NULL);

    } else {

        printf ("creating new stack...\n");
        
        shared_id = shmget(key, (size + 2) * sizeof(char), 0666);
        FUNCTION_SECURITY (shared_id == -1, {perror ("bad shmget");
                                             delete_sem_set (key);
                                             free (stack);}, NULL);
        
        stack->data = shmat (shared_id, NULL, 0);
        FUNCTION_SECURITY (stack->data == (void*)-1, {
            perror ("bad shmat ()");
            delete_sem_set (key);
            free (stack);
        }, NULL);

        int res1 = semctl (stack->semId, 0, SETVAL, 1);
        int res2 = semctl (stack->semId, 1, SETVAL, size);
        int res3 = semctl (stack->semId, 2, SETVAL, 0);

        FUNCTION_SECURITY (res1 == -1 || res2 == -1 || res3 == -1, {
            free (stack);
            delete_sem_set (key);
            perror ("bad semctl ()");
        }, NULL);

        STACK_SIZE_CELL = 0;
        STACK_MAX_SIZE_CELL = size;

    }

    stack->stackKey = key;

    return stack;

}

static int get_sem_set (const key_t key) {

    errno = 0;
    int semId = semget(key, N_SEMS, IPC_CREAT | IPC_EXCL | 0666);
    if (semId != -1) {
        //semaphores don't exist yet
        union semun arg;
        struct sembuf sop = {0};
        sop.sem_flg = SEM_UNDO;

        arg.val = 0;
        FUNCTION_SECURITY (semctl (semId, 0, SETVAL, arg) == -1, {
            semctl (key, 0, IPC_RMID);
            perror ("bad semctl ()");
        }, -1);

        FUNCTION_SECURITY (semop (semId, &sop, 1) == -1, {
            
            semctl (key, 0, IPC_RMID);
            perror ("bad semop ()");

        }, -1);

    }
    else {
        
        #define TRIES 10

        union semun arg;
        struct semid_ds ds;

        semId = semget(key, 1, 0666);
        FUNCTION_SECURITY (semId == -1, {perror ("bad semget ()");}, -1);

        arg.buf = &ds;
        for (size_t j = 0; j < TRIES; j++) {

            FUNCTION_SECURITY (semctl (semId, 0, IPC_STAT, arg) == -1, {
                perror ("bad semctl ()");
            }, -1);

            if (ds.sem_otime != 0)
                break;
            
            sleep (1);

        }

        FUNCTION_SECURITY (ds.sem_otime == 0, {
            printf ("Bad init in function %s\n", __func__);
        }, -1);

        #undef TRIES

    }

    return semId;

}

static int delete_sem_set (const key_t key) {

    errno = 0;
    int semId = semget(key, N_SEMS, 0666);
    FUNCTION_SECURITY (semId == -1, {perror ("bad semget ()");}, -1);
        
    errno = 0;
    int semctlCheck = semctl(semId, 0, IPC_RMID);
    FUNCTION_SECURITY (semctlCheck == -1, {perror ("bad semctl ()");}, -1);
        
    return 0;
}

int mark_destruct (Stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack),                                             {}, -1);
    FUNCTION_SECURITY (free_shared_mem (stack->stackKey, get_size (stack)) == -1,   {}, -1);
    FUNCTION_SECURITY (delete_sem_set  (stack->stackKey) == -1,                     {}, -1);

}

static int free_shared_mem (const key_t key, const int size) {

    int sharedId = shmget (key, (size + 2) * sizeof (size_t), 0666);
    FUNCTION_SECURITY (sharedId == -1                         , {perror ("bad shmget");}   , -1);
    FUNCTION_SECURITY (shmctl (sharedId, IPC_RMID, NULL) == -1, {perror ("bad shmctl ()");}, -1);

    return 0;

}

int get_size (Stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {}, -1);

    return STACK_MAX_SIZE_CELL;
    
}

int get_count (Stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {}, -1);
    
    return STACK_SIZE_CELL;

}

int push (Stack_t* stack, size_t value) {

    FUNCTION_SECURITY (IS_NULL (stack), {}, -1);
    errno = 0;

    struct sembuf sops = {};
    sops.sem_num    = 1;
    sops.sem_op     = -1;
    sops.sem_flg    = SEM_UNDO;

    FUNCTION_SECURITY (semop (stack->semId, &sops, 1) == -1, {perror ("bad semop ()");}, -1);

    int curMaxSize  = get_size  (stack);
    int curSize     = get_count (stack);

    if (curMaxSize <= curSize) {

        printf ("There's no free slot for new value in shared stack!\n");
        return -1;

    } else {

        REAL_DATA [curSize] = value;
        STACK_SIZE_CELL     = curSize + 1;

    }
    
    sops.sem_op = 1;
    FUNCTION_SECURITY(semop (stack->semId, &sops, 1) == -1, {perror ("bad semop ()");}, -1);

    return 0;

}

int pop (Stack_t* stack, size_t* value) {

    FUNCTION_SECURITY (IS_NULL (stack) || IS_NULL (value), {}, -1);
    
    struct sembuf sops = {};
    sops.sem_num    = 1;
    sops.sem_op     = -1;
    sops.sem_flg    = SEM_UNDO;
    FUNCTION_SECURITY (semop (stack->semId, &sops, 1) == -1, {perror ("bad semop ()");}, -1);

    int curSize = get_count (stack);
    if (curSize <= 0) {

        printf ("Can't pop from stack: current size <= 0!\n");
        return -1;

    } else {

        *value = REAL_DATA [curSize - 1];
        STACK_SIZE_CELL = curSize - 1;

    }

    sops.sem_op = 1;
    FUNCTION_SECURITY (semop (stack->semId, &sops, 1) == -1, {perror ("bad semop ()");}, -1);

    return 0;

} 

int detach_stack (Stack_t* stack) {

    FUNCTION_SECURITY (IS_NULL (stack), {}, -1);

    errno = 0;
    int shmdtCheck = shmdt (stack->data);
    FUNCTION_SECURITY (shmdtCheck == -1, {
        perror ("bad shmdt ()"); 
        free (stack);
    }, -1);

    free (stack);

    return 0;

}

void stack_dump (Stack_t* stack) {

    printf ("\n\n\n");

    printf ("Stack ptr:         [%p];\n", stack);
    printf ("Stack max size:    [%d];\n", get_size  (stack));
    printf ("Stack cur size:    [%d];\n", get_count (stack));
    printf ("Stack data ptr:    [%p];\n", REAL_DATA);

    printf ("Elements: \n");
    for (int i = 0; i < get_count (stack); ++i)
        printf (" ---\t [%d]: %zu\n", i, REAL_DATA [i]);

    printf ("\n\n\n");
    
}
