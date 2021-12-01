#ifndef COMMON_H__
#define COMMON_H__

#include <stdio.h>
#include <stdlib.h>

#include <errno.h>

#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/sem.h>

#include <unistd.h>
#include <fcntl.h>

#define IS_NULL(ptr) (!ptr)

#define FUNCTION_SECURITY(condition, toDo, err)     do {                    \
                                                                            \
                                                        if (condition) {    \
                                                            toDo            \
                                                            return err;     \
                                                        }                   \
                                                                            \
                                                    } while (0)

#endif

