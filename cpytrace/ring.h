#ifndef RING_H
#define RING_H

typedef struct {
  unsigned int size;
  unsigned long generation;
  unsigned char *read, *write, *buf;
} Ring;

typedef struct {
  Ring *ring;
  unsigned long generation;
  unsigned char *last;
} RingReader;

Ring *ring_from_memory(unsigned char *buf, unsigned int size);
Ring *ring_malloc(unsigned int size);
void ring_free(Ring *ring);
void ring_write(Ring *ring, unsigned char *buf, unsigned int size);

RingReader *reader_malloc(Ring *ring);
void reader_free(RingReader *reader);
int reader_read(RingReader *reader, unsigned char *buf);

#endif // RING_H
