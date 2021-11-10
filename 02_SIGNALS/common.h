#ifndef COMMON_H__
#define COMMON_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define FUNCTION_SECURITY(condition, toDo, err)                             \
                                                do {                        \
                                                                            \
                                                    if (condition) {        \
                                                                            \
                                                        toDo                \
                                                        return err;         \
                                                                            \
                                                    }                       \
                                                                            \
                                                } while (0)

#define IS_NULL(ptr) (!ptr)

#endif
