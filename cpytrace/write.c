//#include "ring.h"
#include "serial.h"
#include "defs.h"
#include "codec.h"
#include "rb/rb.h"

static long desc;
static unsigned char *encoded_buf;

inline void init_writer() {
  desc = rb_open(123123, RB_SIZE, RB_MODE_WR);
  encoded_buf = malloc(MAX_RECORD_SIZE);
}

void write_record(unsigned char* buf, unsigned long size) {
  encode(buf, size, encoded_buf);
  printf("%d\n", size);
  rb_write(desc, &size, sizeof(long));
  rb_write(desc, buf, size);
}

