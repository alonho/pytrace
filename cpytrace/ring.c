/*
Requirements:
1. A cylcic buffer over shared memory.
2. The writer never blocks - if the reader is down or slow, traces are lost.
3. The reader should be able to catch up when:
  a. It starts after the writer.
  b. The writer is faster and bypasses the reader.

shared memory will contain:
1. a circular buffer of trace records delimited by their length.
3. two pointers for the newest and oldest records.
3. generation id, for identifying wrap arounds.

write:
  2. advance the read pointer to the last record that will be available
  4. write the data
  5. advance the write pointer, upon wrap around advance generation id

read:
  1. if first, current = read pointer, else
  2. if overflow occured, current = read pointer (+10?)
  3. if current == write pointer, nothing to read, else
  4. read a record, if read pointer > current, drop it. else,
  5. current++.

overflow didn't occur if:
  1. current pointer > read pointer and generation id == current generation id.
  2. or, current_pointer < read pointer and generation id == current generation id + 1.
*/

struct Ring {
  unsigned long generation, size;
  unsigned char *read, *write, *buf;
};

void ring_init(Ring *ring, unsigned char *buf, unsigned int size) {
  ring->buf = ring->read = ring->write = buf;
  ring->size = size;
  ring->generation = 0;
}

Ring *ring_from_memory(unsigned char *buf, unsigned int size) {
  Ring *ring = buf;
  ring_init(ring, buf + sizeof(Ring), size - sizeof(Ring));
}

void ring_read(Ring *ring, unsigned char *buf, unsigned char *ring_buf, size_t size) {
  assert(size < ring->size);
  int overflow = ring_buf + size - ring->buf + size;
  if (overflow < 0) {
    memcpy(buf, ring_buf, size);
  } else {
    memcpy(buf, ring_buf, size - overflow);
    memcpy(buf + size - overflow, ring->buf, overflow);
  }
}

void ring_write(Ring *ring, unsigned char *ring_buf, unsigned char *buf, size_t size) {
  assert(size < ring->size);
  int overflow = ring_buf + size - ring->buf + size;
  if (overflow < 0) {
    memcpy(ring_buf, buf, size);
  } else {
    memcpy(ring_buf, buf, size - overflow);
    memcpy(ring->buf, buf + size - overflow, overflow);
  }
}

struct RingReader {
  Ring *ring;
  unsigned long generation;
  unsigned char *last;
}

RingReader *reader reader_init(Ring *ring) {
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
	   (reader->last < reader->ring->read && reader->generation == reader->ring->generation - 1))
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
  int size = *((int*) buf);

  // read the record
  ring_read(reader->ring, buf, reader->last + sizeof(int), size);
  if (reader_overflow(reader)) {
    goto overflow;
  }
  reader->last += sizeof(int) + size;
  return size;

 oveflow:
  reader_reset(reader);
  return READ_OVERFLOW;
}
