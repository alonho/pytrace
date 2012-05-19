#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#include "rb.h"

/* bit 0 -> enabled sanity check */
/* bit 1 -> extra traces */
/* bit 2 -> make write blocking for stress tests */
#define RB_DBGFLAGS 7

#if defined(RB_DBGFLAGS) && (RB_DBGFLAGS & 1)
#include <assert.h>
#else
#define assert(...)
#endif


typedef struct
{
    volatile unsigned long flags;
    volatile unsigned long r_idx;
    volatile unsigned long w_idx;
} RB_HDR;

typedef struct
{
    int shmid;
    int semid;
    unsigned long size;
    unsigned long flags;
    RB_HDR *hdr;
    char *shm;
#if defined(RB_DBGFLAGS) && (RB_DBGFLAGS & 1)
    char *shm_e;
#endif
} RB;


static int rb_createsem(key_t key)
{
    int semid, i;

    /* Get semaphore */
    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (semid >= 0)
    {
        /* Initialize the semaphore (set to one) */
        struct sembuf sb;
        sb.sem_num = 0;
        sb.sem_op = 1;
        sb.sem_flg = 0;
        if (semop(semid, &sb, 1) == -1)
        {
            perror("semop");
            goto rb_createsem_error_rmsem;
        }
    }
    else
    {
        struct semid_ds semds;
        union semun semu;
        if (errno != EEXIST)
        {
            perror("semget(EXCL)");
            goto rb_createsem_error;
        }
        /* Get the id */
        semid = semget(key, 1, 0);
        if (semid < 0)
        {
            perror("semget");
            goto rb_createsem_error;
        }
        /* Wait for creator initialization */
        semu.buf = &semds;
        for (i = 0; i < 5; i++)
        {
            semctl(semid, 1, IPC_STAT, semu);
            if (semu.buf->sem_otime != 0)
            {
                break;
            }
            else if (i == 4)
            {
                goto rb_createsem_error;
            }
            sleep(1);
        }
    }
    return semid;

rb_createsem_error_rmsem:
    semctl(semid, 0, IPC_RMID);
rb_createsem_error:
    return -1;
}

static void rb_destroysem(int semid)
{
    union semun arg;
    if (semctl(semid, 0, IPC_RMID, arg) == -1)
    {
        perror("semctl");
    }
}

static void rb_locksem(int semid)
{
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = -1;
    sb.sem_flg = SEM_UNDO;
    if (semop(semid, &sb, 1) == -1)
    {
        perror("semop(rb_locksem)");
    }
}

static void rb_unlocksem(int semid)
{
    struct sembuf sb;
    sb.sem_num = 0;
    sb.sem_op = 1;
    sb.sem_flg = SEM_UNDO;
    if (semop(semid, &sb, 1) == -1)
    {
        perror("semop(rb_locksem)");
    }
}

long rb_open(int id, unsigned long size, unsigned long flags)
{
    key_t key;
    RB *p_rb;
    char *p;
    struct shmid_ds shmds;
    RB_HDR *hdr;
    unsigned long totalsize;

    /* Sanity check: have to request a mode, can not be both simultaneously */
    assert((flags & (RB_MODE_RD | RB_MODE_WR)) &&
           ((flags & (RB_MODE_RD | RB_MODE_WR)) != (RB_MODE_RD | RB_MODE_WR)));

    /* Allocate control structure */
    p_rb = malloc(sizeof(*p_rb));
    if (p_rb == NULL)
    {
        perror("malloc");
        goto rb_open_error;
    }
    p_rb->flags = flags;
    p_rb->size = size;

    /* By default, use the /tmp file as key selector */
    key = ftok("/tmp", id);
    if (key == -1)
    {
        perror("ftok");
        goto rb_open_error_free;
    }

    /* Create the semaphone instance */
    p_rb->semid = rb_createsem(key);
    if (p_rb->semid < 0)
    {
        puts("rb_createsem: failed");
        goto rb_open_error_free;
    }

    /* Lock the access to the SHM */
    rb_locksem(p_rb->semid);

    /* Create the SHM segment: read/write mode */
    totalsize = size + (unsigned long) (((RB_HDR *)0)+1);
    p_rb->shmid = shmget(key, totalsize, IPC_CREAT | 0666);
    if (p_rb->shmid < 0)
    {
        perror("shmget");
        goto rb_open_error_releasesem;
    }

    /* Attach the segment */
    p = shmat(p_rb->shmid, NULL, 0);
    if (p == (char *) -1)
    {
        perror("shmat");
        goto rb_open_error_releasesem;
    }

    /* Retrieve the information about the segment */
    if (shmctl(p_rb->shmid, IPC_STAT, &shmds) < 0)
    {
        perror("shmctl");
        goto rb_open_error_releasesem;
    }

#if defined(RB_DBGFLAGS) && (RB_DBGFLAGS & 2)
    printf("Size = %d\n", shmds.shm_segsz);
    printf("Number of attachments = %d\n", (int)shmds.shm_nattch);
    printf("Buffer = 0x%08x\n", (unsigned int)(p + sizeof(*hdr)));
#endif

    /* Save the header pointer */
    hdr = p_rb->hdr = (RB_HDR *)p;
    p_rb->shm = (char *)(hdr + 1);
#if defined(RB_DBGFLAGS) && (RB_DBGFLAGS & 1)
    p_rb->shm_e = p + totalsize;
#endif
    /* Check if this is the first user of the SHM */
    if (shmds.shm_nattch == 1)
    {
        /* Initialize the header */
        hdr->flags = flags;
        hdr->r_idx = 0;
        hdr->w_idx = 0;
    }
    else
    {
        if (hdr->flags & flags)
        {
            puts("rb_open: same access already registered");
            goto rb_open_error_releasesem;
        }
    }
    /* Unlock the access to the SHM */
    rb_unlocksem(p_rb->semid);

    return (long) p_rb;

rb_open_error_releasesem:
    /* Unlock the access to the SHM */
    rb_unlocksem(p_rb->semid);

rb_open_error_free:
    free(p_rb);
rb_open_error:
    return -1;
}

int rb_close(long *p_desc)
{
    RB *p_rb = (RB *)(*p_desc);
    struct shmid_ds shmds;

    /* Check if close was already called */
    if (*p_desc == -1)
    {
        return -1;
    }

    /* Lock the access to the SHM */
    rb_locksem(p_rb->semid);

    /* Detach the segment */
    if (shmdt(p_rb->hdr) == -1)
    {
        perror("shmdt");
        goto rb_close_error_releasesem;
    }

    /* Retrieve the information about the segment */
    if (shmctl(p_rb->shmid, IPC_STAT, &shmds) < 0)
    {
        perror("shmctl");
        goto rb_close_error_releasesem;
    }
#if defined(RB_DBGFLAGS) && (RB_DBGFLAGS & 2)
    puts("shmctl:");
    printf("Size = %d\n", shmds.shm_segsz);
    printf("Number of attachments = %d\n", (int)shmds.shm_nattch);
#endif
    /* Check if this was the last user of the SHM */
    if (shmds.shm_nattch == 0)
    {
        /* Destroy the SHM */
        shmctl(p_rb->shmid, IPC_RMID, 0);

        /* Destroy the SEM */
        rb_destroysem(p_rb->semid);
    }
    else
    {
        /* Unlock the access to the SHM */
        rb_unlocksem(p_rb->semid);
    }

    /* Free the allocated control block */
    free(p_rb);

    /* Mark it as freed in case of subsequent calls */
    *p_desc = -1;
    return 0;

rb_close_error_releasesem:
    /* Unlock the access to the SHM */
    rb_unlocksem(p_rb->semid);
    return -1;
}

int rb_write(long desc, char *buf, unsigned long len)
{
    RB *p_rb = (RB *)desc;
    RB_HDR *hdr ;
    unsigned long remain;

    /* Sanity check */
    assert(len != 0);

    /* Check if close was already called */
    if (desc == -1)
    {
        return -1;
    }

    /* Check that data will not be thrown away */
    if (len > p_rb->size)
    {
        printf("rb_write: Too much data added at once (%lu), data will be lost\n", len);
    }

    /* Get the header : after checking the close was not called */
    hdr = p_rb->hdr;

    remain = len;
    while (remain)
    {
        unsigned long available;
        /* Compute the size available for write (always keep 1 element free) */
        available = (hdr->r_idx + p_rb->size - hdr->w_idx - 1) % p_rb->size;

        /* Check if the size available is greater than requested len */
        if (available > remain)
        {
            available = remain;
        }

        /* Check if the write is contiguous */
        if ((hdr->w_idx + available) <= p_rb->size)
        {
            /* Copy the contiguous memory */
            memcpy(&p_rb->shm[hdr->w_idx], buf, available);

            /* Move the pointer */
            hdr->w_idx += available;
            if (hdr->w_idx == p_rb->size)
            {
                hdr->w_idx = 0;
            }
        }
        else
        {
            unsigned long partlen;

            /* Copy the first part */
            partlen = p_rb->size - hdr->w_idx;
            memcpy(&p_rb->shm[hdr->w_idx], buf, partlen);

            /* Copy the second part */
            memcpy(p_rb->shm, buf + partlen, available - partlen);

            /* Move the pointer */
            hdr->w_idx = available - partlen;
        }
        /* Decrement the remaining size */
        remain -= available;

        assert(hdr->w_idx < p_rb->size);
        assert(p_rb->shm <= &p_rb->shm[hdr->w_idx]);
        assert(p_rb->shm_e > &p_rb->shm[hdr->w_idx]);
#if defined(RB_DBGFLAGS) && (RB_DBGFLAGS & 4)
        buf += available;
#else
        break;
#endif
    }
    return len-remain;
}

int rb_read(long desc, char *buf, unsigned long len)
{
    RB *p_rb = (RB *)desc;
    RB_HDR *hdr;
    unsigned long available;

    /* Sanity check */
    assert(len != 0);

    /* Check if close was already called */
    if (desc == -1)
    {
        return -1;
    }

    /* Get the header : after checking the close was not called */
    hdr = p_rb->hdr;

    /* Compute the size available for read */
    available = (hdr->w_idx + p_rb->size - hdr->r_idx) % p_rb->size;

    /* Check if the size available is greater than requested len */
    if (available > len) available = len;

    /* Check if the read is contiguous */
    if ((hdr->r_idx + available) <= p_rb->size)
    {
        /* Copy the contiguous memory */
        memcpy(buf, &p_rb->shm[hdr->r_idx], available);

        /* Move the pointer */
        hdr->r_idx += available;
        if (hdr->r_idx == p_rb->size)
            hdr->r_idx = 0;
    }
    else
    {
        unsigned long partlen;

        /* Copy the first part */
        partlen = p_rb->size - hdr->r_idx;
        memcpy(buf, &p_rb->shm[hdr->r_idx], partlen);

        /* Copy the second part */
        memcpy(buf + partlen, p_rb->shm, available - partlen);

        /* Move the pointer */
        hdr->r_idx = available - partlen;
    }
    assert(hdr->r_idx < p_rb->size);
    assert(p_rb->shm <= &p_rb->shm[hdr->r_idx]);
    assert(p_rb->shm_e > &p_rb->shm[hdr->r_idx]);
    return available;
}
