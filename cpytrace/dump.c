#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "defs.h"
#include "ring.h"
#include "shared_ring.h"
#include "record.pb-c.h"

unsigned char *buf;
Ring *ring;
RingReader *reader;

int traces = 0, overflows = 0;

void print_stats(int sig) {
  printf("traces: %d, overflows: %d\n", traces, overflows);
}

void init() {
  buf = malloc(MAX_RECORD_SIZE);
  ring = shared_ring_init(1);
  reader = reader_malloc(ring);
  signal(SIGINT, print_stats);
  db_init();
}

void print_record(Record *rec) {
  int i;
  for (i = 0; i < rec->depth; i++) {
    printf(" ");
  }
  printf("%f %s %s %d: ", rec->time, rec->module, rec->function, rec->lineno);
  for (i = 0; i < rec->n_arguments; i++) {
    printf("%s = %s", rec->arguments[i]->name, rec->arguments[i]->value);
    if (rec->n_arguments - 1 > i) {
      printf(", ");
    }
  }
  printf("\n");
}

void dump() {
  int size, i;
  Record *rec;
  while (1) {
    switch (size = reader_read(reader, buf)) {
    case 0:
      db_close();
      return;
      usleep(100);
      break;
    case -1:
      overflows++;
      break;
    default:
      traces++;
      rec = record__unpack(NULL, size, buf);
      db_handle_record(rec);
    }
  }
}

int main() {
  init();
  dump();
}
