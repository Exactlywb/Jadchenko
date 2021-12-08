#ifndef COMMON_H__
#define COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <syslog.h>

#define IS_NULL(ptr) (!ptr)

#define FUNCTION_SECURITY(condition, toDo, err) do {                        \
                                                                            \
                                                    if (condition) {        \
                                                        toDo                \
                                                        return err;         \
                                                    }                       \
                                                                            \
                                                } while (0)

#define STR_EQ(str1, str2, len) (!strncmp (str1, str2, len))

#endif
