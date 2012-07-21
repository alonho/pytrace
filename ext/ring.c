/*
requirements:
1. A cylcic buffer over shared memory.
2. The writer never blocks - if the reader is down or slow, oldest data should be lost.
3. The reader should be able to catch up when:
  a. It starts after the writer.
  b. The writer is faster and bypasses the reader.

shared memory will contain:
1. a circular buffer of trace records delimited by their length.
2. absolute indices as read/write pointers. every access requires modulo with the ring size. if the ring size is a power of two, it will also support wrap around of the index.

write procedure:
  1. calc the amount of records that will be overwritten, advance the read pointer after them.
  2. write the data.
  3. advance the write pointer.

read procedure:
  1. the reader saves locally (not in the shmem) a pointer to the 'current' record. 
  2. if it's the first read or overflow occured, current = read pointer (+10?)
  3. read a record, if read pointer > current, drop it. else,
  4. current++.
*/

#include <stdlib.h>
#include <string.h>
#include "ring.h"
#include "defs.h"

static void ring_init(Ring *ring, unsigned char *buf, unsigned int size) {
  ring->buf = buf;
  ring->size = size;
  ring->read_index = ring->write_index = 0;
}

Ring *ring_from_memory(unsigned char *buf, unsigned int size) {
  Ring *ring = (Ring *) buf;
  return ring;
}

Ring *ring_init_from_memory(unsigned char *buf, unsigned int size) {
  Ring *ring = ring_from_memory(buf, size);
  ring_init(ring, buf + sizeof(Ring), size - sizeof(Ring));
  return ring;
}

Ring *ring_malloc(unsigned int size) {
  Ring *ring = malloc(sizeof(Ring));
  unsigned char *buf = malloc(sizeof(Ring));
  ring_init(ring, buf, size);
  return ring;
}

void ring_free(Ring *ring) {
  free(ring->buf);
  free(ring);
}

static void ring_raw_read(Ring *ring, unsigned char *buf, unsigned long read_index, unsigned int size) {
  ASSERT(size <= ring->size);
  int real_index = read_index % ring->size;
  int overflow = real_index + size - ring->size;
  if (overflow <= 0) {
    memcpy(buf, ring->buf + real_index, size);
  } else {
    memcpy(buf, ring->buf + real_index, size - overflow);
    memcpy(buf + size - overflow, ring->buf, overflow);
  }
}

static void ring_raw_write(Ring *ring, unsigned long write_index, unsigned char *buf, unsigned int size) {
  ASSERT(size <= ring->size);
  int real_index = write_index % ring->size;
  int overflow = real_index + size - ring->size;
  if (overflow <= 0) {
    memcpy(ring->buf + real_index, buf, size);
  } else {
    memcpy(ring->buf + real_index, buf, size - overflow);
    memcpy(ring->buf, buf + size - overflow, overflow);
  }
}

static inline void ring_clear(Ring *ring, long size) {
  ASSERT(size < ring->size);
  size -= ring->size - (ring->write_index - ring->read_index);
  long read = ring->read_index;
  unsigned int record_size;

  while (size > 0 && read < ring->read_index + size) {
    ring_raw_read(ring, (unsigned char*) &record_size, read, sizeof(int));
    read += sizeof(int) + record_size;
  }
  ring->read_index = read;
}

void ring_write(Ring *ring, unsigned char *buf, unsigned int size) {
  ring_clear(ring, sizeof(int) + size);
  ring_raw_write(ring, ring->write_index, (unsigned char*) &size, sizeof(int));
  ring_raw_write(ring, ring->write_index + sizeof(int), buf, size);
  ring->write_index += sizeof(int) + size;
}

static inline int reader_overflow(RingReader *reader) {
  return reader->read_index < reader->ring->read_index;
}

#define SKIP_RECORDS 30
static void reader_reset(RingReader *reader) {
  unsigned long read_index = reader->ring->read_index;
  unsigned int size;
  int records = 0;

  while (records < SKIP_RECORDS && read_index != reader->ring->read_index) {
    ring_raw_read(reader->ring, (unsigned char*) &size, read_index, sizeof(int));
    if (reader_overflow(reader)) {
      records = 0;
      read_index = reader->ring->read_index;
    } else {
      records++;
      read_index += sizeof(int) + size;
    }
  }
  reader->read_index = read_index;
}

RingReader *reader_malloc(Ring *ring) {
  RingReader *reader = malloc(sizeof(RingReader));
  reader->ring = ring;
  reader->read_index = reader->ring->read_index;
  return reader;
}

void reader_free(RingReader *reader) {
  free(reader);
}

#define CHECK_OVERFLOW if (reader_overflow(reader)) { goto overflow; }
int reader_read(RingReader *reader, unsigned char *buf) {
  if (reader->read_index == reader->ring->write_index) {
    return 0;
  }
  CHECK_OVERFLOW;
  unsigned int size;
  ring_raw_read(reader->ring, (unsigned char*) &size, reader->read_index, sizeof(int));
  CHECK_OVERFLOW;
  ring_raw_read(reader->ring, buf, reader->read_index + sizeof(int), size);
  CHECK_OVERFLOW;
  reader->read_index += sizeof(int) + size;
  return size;
 overflow:
  reader_reset(reader);
  return READ_OVERFLOW;
}
