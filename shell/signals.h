#ifndef SIGNALS_H
#define SIGNALS_H

#include "headers.h"

// the process group ID of the currently running foreground process
// -1 means no foreground process running
extern pid_t foreground_pgid;

// call this once at startup to install all signal handlers
void setup_signal_handlers();

#endif