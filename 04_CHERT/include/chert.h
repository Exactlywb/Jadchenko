#ifndef CHERT_H__
#define CHERT_H__

#include "common.h"
#include "copy.h"

int     RunTimur            ();

void    RunInterface        (const char* src, const char* dst, const sigset_t signalsSet);
int     SetSignalsSettings  (sigset_t* signalsSet);

#endif
 