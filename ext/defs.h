#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>

#define MAX_ARGS 32
#define MAX_STR_SIZE 1024
// 1 for args names, 1 for args values, 1 for rest of fields of a record
#define MAX_RECORD_SIZE MAX_ARGS * MAX_STR_SIZE * 3 
// make sure MAX_RECORD_SIZE < RB_SIZE
#define RB_SIZE 4194304 // 4MB
// the address is only for sync between the reader/writer
#define RING_ADDRESS (void*) 0x40000000000
#define COMMIT_INTERVAL 5000

#define TRUE 1
#define FALSE 0

#define ASSERT(expression)  \
  ((void) ((expression) ? 0 : __ASSERT (expression, __FILE__, __LINE__)))

#define __ASSERT(expression, file, lineno)  \
  (printf ("%s:%u: failed assertion\n", file, lineno), abort (), 0)

inline static int min(int x, int y) {
  return (x < y) ? x : y;
}

#endif
