#ifndef DAEMON_H__
#define DAEMON_H__

#include "common.h"
#include "inotifyR.h"
#include "copy.h"

typedef enum CopyType {

    CLASSIC_MODE__,
    INOTIFY_MODE__

} CopyType;

int     RunDaemon           (const char* daemonName);

int     SetSignalsSettings  (sigset_t* signalsSet);
void    RunInterface        (char* src, char* dst, const sigset_t signalsSet);

#endif
