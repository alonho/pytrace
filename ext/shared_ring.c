#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include "defs.h"
#include "shared_ring.h"

static inline int shm_exists(key_t key) {
  return shmget(key, RB_SIZE, 0) != -1; 
}

static inline int shm_init(key_t key) {
  return shmget(key, RB_SIZE, IPC_CREAT | SHM_R | SHM_W); 
}

static inline unsigned char* attach_read(int shmid) {
  // the address can be removed if ring->buf will be relative
  return shmat(shmid, RING_ADDRESS, SHM_RND | SHM_RDONLY);
}

static inline unsigned char* attach_write(int shmid) {
  return shmat(shmid, RING_ADDRESS, SHM_RND);
}

Ring *shared_ring_init(int readonly) {
  key_t key;
  Ring *ring;
  int shmid, shm_existed;
  unsigned char *mem;
  key = ftok("/tmp", 12345); // getpid() is better but maps a new one everytime, how to clean it?
  shm_existed = shm_exists(key);
  // read/write by user and group
  if (-1 == (shmid = shm_init(key))) {
    perror("shmget");
    return NULL;
  }

  if (!shm_existed) {
    mem = attach_write(shmid);
    if (-1 == (long) mem) {
      perror("shmat");
      return NULL;
    }
    ring = ring_init_from_memory(mem, RB_SIZE);
    if (readonly) {
      shmdt(mem);
      mem = attach_read(shmid);
      if (-1 == (long) mem) {
	perror("shmat");
	return NULL;
      }
      ring_from_memory(mem, RB_SIZE);
    }
  } else {
    if (readonly) {
      mem = attach_read(shmid);      
    } else {
      mem = attach_write(shmid);
    }
    if (-1 == (long) mem) {
      perror("shmat");
      return NULL;
    }
    ring = ring_from_memory(mem, RB_SIZE);
  }
  return ring;
}
