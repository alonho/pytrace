#ifndef DEFS_H
#define DEFS_H

#define MAX_ARGS 128
#define MAX_STR_SIZE 1024
#define MAX_RECORD_SIZE MAX_ARGS * MAX_STR_SIZE * 2 // double just to be safe
// make sure MAX_RECORD_SIZE < RB_SIZE
//#define RB_SIZE 4194304 
#define RB_SIZE 1048576

#endif
