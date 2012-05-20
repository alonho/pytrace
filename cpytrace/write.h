#ifndef WRITE_H
#define WRITE_H

#include "rb/rb.h"
#include "defs.h"

static long desc;

inline void init_writer() {
  desc = rb_open(123123, RB_SIZE, RB_MODE_WR);
}

inline void write_record(char* buf, unsigned long size) {
  rb_write(desc, &size, sizeof(long));
  rb_write(desc, buf, size);
}

#endif /* WRITE_H */
