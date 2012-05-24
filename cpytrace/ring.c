/*
Requirements:
1. A cylcic buffer over shared memory.
2. The writer never blocks - if the reader is down or slow, traces are lost.
3. The reader should be able to catch up when:
  a. It starts after the writer.
  b. The writer is faster and bypasses the reader.

shared memory will contain:
1. a circular buffer of trace records delimited by their length.
2. two pointers for the newest record and last valid record.
3. a generation id, for identifying wrap arounds.

write procedure:
  1. calc the amount of records that will be overwritten, advance the read pointer after them.
  2. write the data.
  3. advance the write pointer, upon wrap around advance generation id.

read procedure:
  1. the reader saves locally (not in the shmem) a pointer to the 'current' record and the last generation id. 
  2. if it's the first read or overflow occured, current = read pointer (+10?)
  3. if current == write pointer, nothing to read, else
  4. read a record, if read pointer > current, drop it. else,
  5. current++.

overflow didn't occur if:
  1. current pointer > read pointer and generation id == current generation id.
  2. or, current_pointer < read pointer and generation id == current generation id + 1.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "ring.h"

void ring_init(Ring *ring, unsigned char *buf, unsigned int size) {
  ring->buf = ring->read = ring->write = buf;
  ring->size = size;
  ring->generation = 0;
}

Ring *ring_from_memory(unsigned char *buf, unsigned int size) {
  Ring *ring = (Ring *) buf;
  ring_init(ring, buf + sizeof(Ring), size - sizeof(Ring));
}

Ring *ring_malloc(unsigned int size) {
  Ring *ring = malloc(sizeof(Ring));
  unsigned char *buf = malloc(sizeof(Ring));
  ring_init(ring, buf, size);
  return ring;
}

void ring_read(Ring *ring, unsigned char *buf, unsigned char *ring_buf, unsigned int size) {
  assert(size < ring->size);
  int overflow = ring_buf + size - ring->buf + size;
  if (overflow < 0) {
    memcpy(buf, ring_buf, size);
  } else {
    memcpy(buf, ring_buf, size - overflow);
    memcpy(buf + size - overflow, ring->buf, overflow);
  }
}

void ring_clear(Ring *ring, unsigned int size) {
  assert(size < ring->size);
  if (ring->write < ring->read) {
    if (ring->read - ring->write >= size) {
      return;
    }
  } else {
    if (ring->buf + ring->size - ring->write + ring->read - ring->buf >= size) {
      return;
    }
  }
}

void ring_write(Ring *ring, unsigned char *ring_buf, unsigned char *buf, unsigned int size) {
  assert(size < ring->size);
  int overflow = ring_buf + size - ring->buf + size;
  if (overflow < 0) {
    memcpy(ring_buf, buf, size);
  } else {
    memcpy(ring_buf, buf, size - overflow);
    memcpy(ring->buf, buf + size - overflow, overflow);
  }
}

RingReader *reader_init(Ring *ring) {
  RingReader *reader = malloc(sizeof(RingReader));
  reader_reset(reader);
  return reader;
}

void reader_reset(RingReader *reader) {
  reader->generation = reader->ring->generation;
  reader->last = reader->ring->read;
}

int reader_overflow(RingReader *reader) {
  return !((reader->last > reader->ring->read && reader->generation == reader->ring->generation) || 
	   (reader->last < reader->ring->read && reader->generation == reader->ring->generation - 1));
}

#define READ_OVERFLOW -1
#define READ_EMPTY 0
int reader_read(RingReader *reader, char *buf) {
  if (reader_overflow(reader)) {
    goto overflow;
  }
  if (reader->last == reader->ring->write) {
    return READ_EMPTY;
  }

  // read the size of the record
  ring_read(reader->ring, buf, reader->last, sizeof(int));
  if (reader_overflow(reader)) {
    goto overflow;
  }
  unsigned int size = *((int*) buf);

  // read the record
  ring_read(reader->ring, buf, reader->last + sizeof(int), size);
  if (reader_overflow(reader)) {
    goto overflow;
  }
  reader->last += sizeof(int) + size;
  return size;

 overflow:
  reader_reset(reader);
  return READ_OVERFLOW;
}
