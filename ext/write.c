#include "shared_ring.h"
#include "ring.h"
#include "defs.h"

static Ring *ring;

void init_writer(void) {
  ring = shared_ring_init(0);
  ASSERT(0 != ring);
}

void write_record(unsigned char* buf, unsigned long size) {
  ring_write(ring, buf, size);
}
