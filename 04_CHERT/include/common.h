#ifndef COMMON_H__
#define COMMON_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>

#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>

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

#define STR_ERR strerror (errno)

#define BUF_SIZE 1000

char*   ConcatStrings   (const int num, ...);

#endif
