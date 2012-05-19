#ifndef RB_H
#define RB_H

/* Define the open modes (bitfield) */
#define RB_MODE_RD 1
#define RB_MODE_WR 2

extern long rb_open(int id, unsigned long size, unsigned long flags);

extern int rb_close(long *p_desc);

extern int rb_write(long desc, char *buf, unsigned long len);

extern int rb_read(long desc, char *buf, unsigned long len);

#endif /* RB_H */
