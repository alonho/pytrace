#include <stdlib.h>
#include <sys/shm.h>
#include "defs.h"
#include "shared_ring.h"

Ring *shared_ring_init(int read_only) {
  key_t key;
  int shmid, first_attach;
  struct shmid_ds shmds;
  unsigned char *mem;
  Ring *ring;

  key = ftok("/tmp", 12345);
  // read/write by user and group
  shmid = shmget(key, RB_SIZE, IPC_CREAT | SHM_R | SHM_W); 
  if (-1 == shmid) {
    perror("shmget");
    return NULL;
  }

  if(shmctl(shmid, IPC_STAT, &shmds) < 0) {
    perror("shmctl");
    return NULL;
  }
  
  first_attach = shmds.shm_nattch == 0;
  mem = shmat(shmid, 
	      RING_ADDRESS, 
	      SHM_RND | ((read_only && !first_attach) ? SHM_RDONLY : 0));
  if (-1 == (long) mem) {
    perror("shmat");
    return NULL;
  }
  
  if (first_attach) { // should be protected by a semaphore
    ring = ring_init_from_memory(mem, RB_SIZE);
    shmdt(RING_ADDRESS);
    mem = shmat(shmid, 
		RING_ADDRESS, 
		SHM_RND | (read_only ? SHM_RDONLY : 0));
    if (-1 == (long) mem) {
      perror("shmat");
      return NULL;
    }
  } else {
    ring = ring_from_memory(mem, RB_SIZE);
  }

  return ring;
}
