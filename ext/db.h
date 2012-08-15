#ifndef DB_H
#define DB_H

#include "record_pb.h"

void db_init(void);
void db_commit(void);
void db_truncate(int count);
int db_handle_lost(void);
int db_handle_record(Record *rec);

#endif /* db */
