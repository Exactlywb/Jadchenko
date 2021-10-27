#ifndef CANE_HEADER_
#define CANE_HEADER_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include <errno.h>
#include <assert.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <wait.h>

#define IS_NULL(ptr) (!ptr)
#define FUNCTION_PROTECT(condition, toDo, errToRet)     do {                                \
                                                                                            \
                                                            if (condition) {                \
                                                                                            \
                                                                toDo                        \
                                                                return errToRet;            \
                                                                                            \
                                                            }                               \
                                                                                            \
                                                        } while (0)

#define SET_ERRNO errno = 0

typedef enum ConveyerErrs {

    NO_CONVEYER_ERR,
    EMPTY_FILE,
    BAD_ALLOC,
    BAD_INPUT_DESC,
    NO_ANY_COMMAND,
    BAD_EXEC_COMMAND,
    BAD_FORK_CALL

} ConveyerErrs;

typedef struct Command {

    char**      argv;
    int         argc;

} Command;

int         RunConveyer     (char** argv, const int input);
size_t      GetFileSize     (const char* file);

#endif

