#ifndef COMMON_H__
#define COMMON_H__

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <errno.h>

#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define IS_NULL(ptr) (!ptr)
#define NULLIFY_ERRNO errno = 0

#define FUNCTION_SECURITY(condition, toDo, err) do {                    \
                                                    if (condition) {    \
                                                        toDo            \
                                                        return err;     \
                                                    }                   \
                                                } while (0)   

#define PRINT_ERR(msg) printf ("\033[0;31m%s: error was catched in function %s"               \
                               "on line %d in file %s\e[0m\n", msg, __func__,            \
                                                        __LINE__, __FILE__)

#endif

