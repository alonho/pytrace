#include <stdio.h>
#include "rb/rb.h"
#include "defs.h"

static long desc;
static char* buf;

void init() {
  buf = malloc(MAX_RECORD_SIZE);
  desc = rb_open(123123, RB_SIZE, RB_MODE_RD);
}

void get_record() {
  long size;
  long read_count;
  read_count = rb_read(desc, buf, sizeof(size));
  if (0 != read_count) {
    read_count = 0;
    size = (long) (*buf);
    do {
      read_count += rb_read(desc, buf + read_count, size - read_count);
    } while(size != read_count);
  }
}

int main() {
  init();
  while (1) {
    get_record();
  }
}
