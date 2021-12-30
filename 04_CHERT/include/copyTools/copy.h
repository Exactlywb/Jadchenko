#ifndef COPY_H__
#define COPY_H__

#include "common.h"
#include "inotifyR.h"

int     RegularCopy     (const char* dirName, const char* output);
int     MountCopy       (Watch* watch, const char* dirName, const char* output, const char* shortDirName);  //wrapper for RegularCopy which checks /proc/self/mountinfo file

int     InotifyCopy     (Watch* watch, const char* dirName, const char* output);

#endif
