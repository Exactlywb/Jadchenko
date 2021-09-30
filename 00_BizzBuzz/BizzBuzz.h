#include <stdio.h> 
#include <stdlib.h>
#include <ctype.h>

#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#define FUNCTION_ASSERT(condition, doBeforeRet, errToRet)                               \
                                                            do {                        \
                                                                                        \
                                                                if (condition) {        \
                                                                                        \
                                                                    doBeforeRet         \
                                                                    return errToRet;    \
                                                                                        \
                                                                }                       \
                                                                                        \
                                                            } while (0)                 \

#define IS_NULL(ptr) (!ptr)

#define SET_ERRNO errno = 0

typedef enum BB_WORD_TYPES {

    BB_SPACE_,
    BB_LETTERS_,
    BB_NUMBER_

}BB_WORD_TYPES;

int        RunBizzBuzz     (const int argc, char** argv);

