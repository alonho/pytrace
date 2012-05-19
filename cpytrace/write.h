#ifndef WRITE_H
#define WRITE_H

#include "rb/rb.h"

#define RB_SIZE 2^20 // MB

static long desc;

inline void init_writer() {
  desc = rb_open(123123, RB_SIZE, RB_MODE_WR);
}

inline void write_record(char* buf, unsigned long size) {
  printf("bla");
  rb_write(desc, &size, sizeof(long));
  rb_write(desc, buf, size);
}

#endif /* WRITE_H */
