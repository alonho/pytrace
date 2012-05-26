#include <stdlib.h>
#include <sys/shm.h>
#include "defs.h"
#include "shared_ring.h"

Ring *shared_ring_init(void) {
  key_t key;
  int shmid;
  struct shmid_ds shmds;
  unsigned char *mem;
  Ring *ring;

  key = ftok("/tmp", 12345);
  // read/write by user and group
  shmid = shmget(key, RB_SIZE, IPC_CREAT | SHM_R | SHM_W); 
  if (-1 == shmid) {
    perror("shmget");
    return -1;
  }

  if(shmctl(shmid, IPC_STAT, &shmds) < 0) {
    perror("shmctl");
    return -1;
  }

  mem = shmat(shmid, (void*) 0x40000000000, SHM_RND);
  if (-1 == (long) mem) {
    perror("shmat");
    return -1;
  }

  if (shmds.shm_nattch == 1) { // should be protected by a semaphore
    ring = ring_init_from_memory(mem, RB_SIZE);
  } else {
    ring = ring_from_memory(mem, RB_SIZE);
  }

  return ring;
}
