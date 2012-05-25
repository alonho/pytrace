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

static void ring_init(Ring *ring, unsigned char *buf, unsigned int size) {
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

void ring_free(Ring *ring) {
  free(ring->buf);
  free(ring);
}

static unsigned char* ring_raw_read(Ring *ring, unsigned char *buf, unsigned char *ring_buf, unsigned int size) {
  assert(size <= ring->size);
  int overflow = ring_buf + size - (ring->buf + ring->size);
  if (overflow < 0) {
    if (NULL != buf) {
      memcpy(buf, ring_buf, size);
    }
    return ring_buf + size;
  } else {
    if (NULL != buf) {
      memcpy(buf, ring_buf, size - overflow);
      memcpy(buf + size - overflow, ring->buf, overflow);
    }
    return ring->buf + overflow;
  }
}

static unsigned char* ring_raw_write(Ring *ring, unsigned char *ring_buf, unsigned char *buf, unsigned int size) {
  assert(size <= ring->size);
  int overflow = ring_buf + size - (ring->buf + ring->size);
  if (overflow <= 0) {
    memcpy(ring_buf, buf, size);
    return ring_buf + size;
  } else {
    memcpy(ring_buf, buf, size - overflow);
    memcpy(ring->buf, buf + size - overflow, overflow);
    return ring->buf + overflow;
  }
}

static void ring_clear(Ring *ring, unsigned int size) {
  long size_left = size;
  unsigned int available, record_size;
  unsigned char *read = ring->read;
  unsigned char *initial_read = ring->read;
  assert(size < ring->size);
  if (ring->write < ring->read) {
    available = ring->read - ring->write;
  } else {
    available = ring->buf + ring->size - ring->write + ring->read - ring->buf;
  }
  available--; // the reader should never meet the writer
  size_left -= available;
  while (size_left > 0) {
    size_left -= sizeof(int);
    read = ring_raw_read(ring, (unsigned char*) &record_size, read, sizeof(int));
    size_left -= record_size;
    read = ring_raw_read(ring, NULL, read, record_size);
  }
  if (read < initial_read) {
    ring->generation++;
  }
  ring->read = read;
}

void ring_write(Ring *ring, unsigned char *buf, unsigned int size) {
  unsigned char* write;
  ring_clear(ring, sizeof(int) + size);
  write = ring_raw_write(ring, ring->write, (unsigned char*) &size, sizeof(int));
  ring->write = ring_raw_write(ring, write, buf, size);
}

static void reader_reset(RingReader *reader) {
  reader->generation = reader->ring->generation;
  reader->last = reader->ring->read;
}

RingReader *reader_malloc(Ring *ring) {
  RingReader *reader = malloc(sizeof(RingReader));
  reader->ring = ring;
  reader_reset(reader);
  return reader;
}

void reader_free(RingReader *reader) {
  free(reader);
}

static int reader_overflow(RingReader *reader) {
  return !((reader->last >= reader->ring->read && reader->generation == reader->ring->generation) || 
	   (reader->last < reader->ring->read && reader->generation == reader->ring->generation + 1));
}

#define READ_OVERFLOW -1
int reader_read(RingReader *reader, unsigned char *buf) {
  unsigned char *read;
  if (reader_overflow(reader)) {
    goto overflow;
  }
  if (reader->last == reader->ring->write) {
    return 0;
  }

  // read the size of the record
  read = ring_raw_read(reader->ring, buf, reader->last, sizeof(int));
  if (reader_overflow(reader)) {
    goto overflow;
  }
  unsigned int size = *((int*) buf);

  // read the record
  read = ring_raw_read(reader->ring, buf, read, size);
  if (reader_overflow(reader)) {
    goto overflow;
  }
  if (read < reader->last) {
    reader->generation++;
  }  
  reader->last = read;
  return size;

 overflow:
  reader_reset(reader);
  return READ_OVERFLOW;
}
