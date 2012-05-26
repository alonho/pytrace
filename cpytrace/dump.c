#include <stdio.h>
#include <stdlib.h>
#include "record.h"
#include "defs.h"
#include "ring.h"
#include "shared_ring.h"

unsigned char *buf;
Ring *ring;
RingReader *reader;

void init() {
  buf = malloc(MAX_RECORD_SIZE);
  ring = shared_ring_init();
  reader = reader_malloc(ring);
}

int main() {
  int size, i=0;
  Record *rec;
  init();
  while (1) {
    switch (size = reader_read(reader, buf)) {
    case -1:
      printf("overflow\n");
      break;
    case 0:
      printf("no data");
      sleep(1);
      break;
    default:
      i++;
      
      rec = record__unpack(NULL, size, buf);
      /*printf("%f %s %s %d: ", rec->time, rec->module, rec->function, rec->type);
      for (i = 0; i < rec->n_arguments; i++) {
	printf("%s = %s, ", rec->arguments[i]->name, rec->arguments[i]->value);
	}*/
      printf("%d\n", i);
    }
  }
}
