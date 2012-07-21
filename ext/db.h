#ifndef DB_H
#define DB_H

#include "record.pb-c.h"

void db_init(void);
void db_commit(void);
int db_handle_lost(void);
int db_handle_record(Record *rec);

#endif /* db */
