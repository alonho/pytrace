#ifndef DUMP_H
#define DUMP_H

#include "ring.h"

void dump_init(Ring *ring);
void dump(void);

void dump_thread_start(void);
void dump_thread_stop(void);

#endif /* dump */
