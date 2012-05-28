#include <stdio.h>
#include <stdlib.h>
#include <signal.h>;
#include "record.h"
#include "defs.h"
#include "ring.h"
#include "shared_ring.h"

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
}

int main() {
  int size, i;
  Record *rec;
  init();
  while (1) {
    switch (size = reader_read(reader, buf)) {
    case 0:
      usleep(1);
      break;
    case -1:
      overflows++;
      break;
    default:
      traces++;
      rec = record__unpack(NULL, size, buf);
      printf("%f %s %s %d: ", rec->time, rec->module, rec->function, rec->type);
      for (i = 0; i < rec->n_arguments; i++) {
	printf("%s = %s, ", rec->arguments[i]->name, rec->arguments[i]->value);
      }
      printf("\n");
    }
  }
}
