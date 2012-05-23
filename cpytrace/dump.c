#include <stdio.h>
#include "rb/rb.h"
#include "record.h"
#include "defs.h"

static long desc;
static char* buf;

void init() {
  buf = malloc(MAX_RECORD_SIZE);
  desc = rb_open(123123, RB_SIZE, RB_MODE_RD);
}

int get_record() {
  long size;
  long read_count;
  read_count = rb_read(desc, buf, sizeof(size));
  if (0 != read_count) {
    read_count = 0;
    size = (long) (*buf);
    do {
      read_count += rb_read(desc, buf + read_count, size - read_count);
    } while(size != read_count);
    return size;
  }
  return 0;
}

int main() {
  int size, i;
  Record *rec;
  init();
  while (1) {
    if ((size = get_record()) != 0) {
      rec = record__unpack(NULL, size, buf);
      printf("%f %s %s %d: ", rec->time, rec->module, rec->function, rec->type);
      for (i = 0; i < rec->n_arguments; i++) {
	printf("%s = %s, ", rec->arguments[i]->name, rec->arguments[i]->value);
      }
      printf("\n");
    }
  }
}
