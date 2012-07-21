#include <stdlib.h>
#include <stdio.h>
#include "assert.h"
#include "ring.c"

#define BUF_SIZE 11

void test_raw() {
  Ring *ring = ring_malloc(BUF_SIZE);
  unsigned char *buf = malloc(sizeof(unsigned char) * BUF_SIZE);
  unsigned char *reference = malloc(sizeof(unsigned char) * BUF_SIZE);
  int i;
  for (i=0; i < BUF_SIZE; i++) {
    reference[i] = i;
  }

  // raw write entire buffer and verify
  ring_raw_write(ring, 0, reference, BUF_SIZE);
  ring_raw_read(ring, buf, 0, BUF_SIZE);
  assert(0 == memcmp(reference, buf, BUF_SIZE));

  // raw write entire buffer from middle and verify
  ring_raw_write(ring, BUF_SIZE / 2, reference, BUF_SIZE);
  ring_raw_read(ring, buf, BUF_SIZE / 2, BUF_SIZE);
  assert(0 == memcmp(reference, buf, BUF_SIZE));
  
  ring_free(ring);
}

void test_ring() {
  int size;
  unsigned char *buf = malloc(sizeof(unsigned char) * BUF_SIZE);
  Ring *ring = ring_malloc(BUF_SIZE);
  RingReader *reader = reader_malloc(ring);

  size = reader_read(reader, buf);
  assert(0 == size);  
  
  printf("one write and one read\n");
  ring_write(ring, "11", 2);
  size = reader_read(reader, buf);
  assert(2 == size);
  assert(0 == memcmp(buf, "11", 2));

  printf("two writes and one read due to overflow\n");
  ring_write(ring, "22", 2);
  ring_write(ring, "33", 2);
  size = reader_read(reader, buf);
  assert(-1 == size);
  size = reader_read(reader, buf);
  assert(2 == size);
  assert(0 == memcmp(buf, "33", 2));
  
  printf("two small writes and two reads\n");
  ring_write(ring, "4", 1);
  ring_write(ring, "5", 1);
  size = reader_read(reader, buf);
  assert(1 == size);
  assert(0 == memcmp(buf, "4", 1));
  size = reader_read(reader, buf);
  assert(1 == size);
  assert(0 == memcmp(buf, "5", 1));  

  printf("fill once again\n");
  ring_write(ring, "123456", 6);
  size = reader_read(reader, buf);
  assert(6 == size);
  assert(0 == memcmp(buf, "123456", 6));

  ring_free(ring);
}

int main() {
  test_raw();
  test_ring();
  return 0;
}
