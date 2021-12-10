#ifndef COPY_H__
#define COPY_H__

#include "common.h"
#include "hashtable.h"

int     CopyDir             (const char* dirName, const char* output);
int     CopyUsingInotify    (const char* dirName, const char* output);

#endif
