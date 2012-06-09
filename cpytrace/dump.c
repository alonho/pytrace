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
  printf("%f %s %s %d: ", rec->time, rec->module.data, rec->function.data, rec->lineno);
  for (i = 0; i < rec->n_arguments; i++) {
    printf("%s = %s", rec->arguments[i]->name.data, rec->arguments[i]->value.data);
    if (rec->n_arguments - 1 > i) {
      printf(", ");
    }
  }
  printf("\n");
}

#define COMMIT_INTERVAL 5000

void dump() {
  int size, count=0;
  Record *rec;
  while (1) {
    switch (size = reader_read(reader, buf)) {
    case 0:
      usleep(100);
      db_commit();
      count = 0;
      break;
    case -1:
      overflows++;
      break;
    default:
      traces++;
      count++;
      rec = record__unpack(NULL, size, buf);
      //print_record(rec);
      db_handle_record(rec);
      if (COMMIT_INTERVAL < count) {
	count = 0;
	db_commit();
      }
    }
  }
}

int main() {
  init();
  dump();
}
