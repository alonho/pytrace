#ifndef DEFS_H
#define DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define MAX_ARGS 32
#define MAX_STR_SIZE 1024
// 1 for args names, 1 for args values, 1 for rest of fields of a record
#define MAX_RECORD_SIZE MAX_ARGS * MAX_STR_SIZE * 3 
// make sure MAX_RECORD_SIZE < RB_SIZE
#define RB_SIZE 1048576 * 2 // 2MB
// the address is only for sync between the reader/writer
#define RING_ADDRESS (void*) 0x40000000000
#define COMMIT_INTERVAL 5000
#define MAX_TRACES 10000

#define DONT_TRACE_NAME "PYTRACE_OFF"

#define TRUE 1
#define FALSE 0

#define ASSERT(expression)  \
  ((void) ((expression) ? 0 : __ASSERT (expression, __FILE__, __LINE__)))

#define __ASSERT(expression, file, lineno)  \
  (printf ("%s:%u: failed assertion\n", file, lineno), abort (), 0)

static inline int min(int x, int y) {
  return (x < y) ? x : y;
}

static inline double floattime(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (double) t.tv_sec + t.tv_usec * 0.000001;
}

#endif
