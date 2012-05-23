// http://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing

#define end_block(code) (*code_ptr = (code), code_ptr = source++, code = 1)

unsigned long encode(const unsigned char *source,
		     unsigned long size,
		     unsigned char *dest) {
  const unsigned char *orig_dest = dest;
  const unsigned char *end = source + size;
  unsigned char *code_ptr = dest++;
  unsigned char code = 1;
 
  while (source < end) {
    if (0 == *source) {
      end_block(code);
    } else {
      *dest++ = *source;
      code++;
      if (code == 0xff) {
        end_block(code);
      }
    }
    source++;
  }

  end_block(code);

  if (0 == *dest) {
    *(++dest) = 0;
  }

  return dest - orig_dest;
}
 
unsigned long decode(const unsigned char *source,
		     unsigned long size, 
		     unsigned char *dest)
{
  const unsigned char *orig_dest = dest;
  const unsigned char *end = source + size;
  while (source < end)
  {
    int i, code = *source++;
    for (i = 1; i < code; i++) {
      *dest++ = *source++;
    }
    if (code < 0xff) {
      *dest++ = 0;
    }
  }
  if (0 == *dest) {
    dest--;
  }
  return dest - orig_dest;
}
