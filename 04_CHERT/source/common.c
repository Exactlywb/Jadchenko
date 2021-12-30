#include "common.h"

char* ConcatStrings (const int num, ...) {

    va_list strs;
    va_start (strs, num);
    const char* curStr = NULL;
    size_t fullLength = 0;
    for (int i = 0; i < num; ++i) {

        curStr = va_arg (strs, char*);
        fullLength += strlen (curStr);

    }

    va_end (strs);
    
    va_list newStrs;
    char* res = (char*)calloc (fullLength + 1, sizeof (*res));
    va_start (newStrs, num);

    for (int i = 0; i < num; ++i) {

        curStr = va_arg (newStrs, char*);

        size_t curLength = strlen (curStr);
        strncpy (res, curStr, curLength);

        res += curLength;

    }
    *res = 0;

    va_end (newStrs);
    
    res -= fullLength;

    return res;

}
