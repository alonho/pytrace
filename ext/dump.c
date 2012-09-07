#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "db.h"
#include "defs.h"
#include "ring.h"
#include "shared_ring.h"
#include "record_pb.h"
#include "dump.h"

RingReader *reader;
unsigned char *buf;

void dump_init(Ring *ring) {
  db_init();
  reader = reader_malloc(ring);
  buf = malloc(MAX_RECORD_SIZE);
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

int should_stop = 0;

void dump(void) {
  int size, count=0, last_was_overflow=FALSE, last_was_none=FALSE;
  Record *rec;
  while (1) {
    switch (size = reader_read(reader, buf)) {
    case 0:
      if (FALSE == last_was_none) {
	db_truncate(MAX_TRACES);
	last_was_none = TRUE;
      }
      if (should_stop) {
	db_commit();
	should_stop = 0;
	return;
      }
      db_commit();
      usleep(50000); // 50 ms
      count = 0;
      break;
    case READ_OVERFLOW:
      if (FALSE == last_was_overflow) {
	db_handle_lost();
	last_was_overflow = TRUE;
      }
      break;
    default:
      count++;
      last_was_overflow = FALSE;
      last_was_none = FALSE;
      rec = record__unpack(NULL, size, buf);
      assert(NULL != rec);
      db_handle_record(rec);
      record__free_unpacked(rec, NULL);
      if (COMMIT_INTERVAL < count) {
	count = 0;
	db_truncate(MAX_TRACES);
	db_commit();
      }
    }
  }
}

pthread_t dump_thread;
void dump_thread_start(void) {
  pthread_create(&dump_thread, NULL,  (void*) dump, NULL);
  should_stop = 0;
}

void dump_thread_stop(void) {
  should_stop = 1;
  while (1 == should_stop) {
    usleep(10);
  }
}
