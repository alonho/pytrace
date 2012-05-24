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

void ring_init(Ring *ring, unsigned char *buf, unsigned int size);
Ring *ring_from_memory(unsigned char *buf, unsigned int size);
Ring *ring_malloc(unsigned int size);
void ring_read(Ring *ring, unsigned char *buf, unsigned char *ring_buf, unsigned int size);
void ring_clear(Ring *ring, unsigned int size);
void ring_write(Ring *ring, unsigned char *ring_buf, unsigned char *buf, unsigned int size);

RingReader *reader_init(Ring *ring);
void reader_reset(RingReader *reader);
int reader_overflow(RingReader *reader);
int reader_read(RingReader *reader, char *buf);
