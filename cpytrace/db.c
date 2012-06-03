#include <sqlite3.h>
#include "record.pb-c.h"

#define SQLITE_ASSERT(x) assert(SQLITE_OK == (x));
#define SQLITE_DONE_OR_CONSTRAINT(x) assert(SQLITE_DONE == (x) || SQLITE_CONSTRAINT == (x));
#define SQLITE_EXEC(query) SQLITE_ASSERT(sqlite3_exec(db, (query), NULL, NULL, NULL));
#define SQLITE_PREPARE(query, stmt) SQLITE_ASSERT(sqlite3_prepare_v2(db, (query), -1, (stmt), NULL));

sqlite3 *db;
sqlite3_stmt *stmt_modules_insert;
sqlite3_stmt *stmt_modules_select;
sqlite3_stmt *stmt_funcs_insert;
sqlite3_stmt *stmt_funcs_select;
int sqlite_status;

void db_init() {
  // config is effective only before open, avoid mutexes for performance
  SQLITE_ASSERT(sqlite3_config(SQLITE_CONFIG_SINGLETHREAD)); 
  // WAL is a fast journal
  SQLITE_ASSERT(sqlite3_open_v2("db.sqlite", 
			       &db, 
			       SQLITE_OPEN_CREATE | 
			       SQLITE_OPEN_READWRITE | 
			       SQLITE_OPEN_WAL, 
			       NULL));

  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS  modules (           \
        id INTEGER NOT NULL,				       \
        value VARCHAR,					       \
        PRIMARY KEY (id),				       \
        UNIQUE (value)					       \
)");
  
  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS funcs (                       \
        id INTEGER NOT NULL,						\
        module_id INTEGER NOT NULL,					\
        lineno INTEGER,							\
        name VARCHAR,							\
        PRIMARY KEY (id),						\
        UNIQUE (module_id, lineno, name),				\
        FOREIGN KEY(module_id) REFERENCES modules (id))");

  SQLITE_EXEC("BEGIN");

  SQLITE_PREPARE("INSERT INTO modules (value) VALUES (?)", &stmt_modules_insert);
  SQLITE_PREPARE("SELECT id FROM modules where value=?", &stmt_modules_select);
  SQLITE_PREPARE("INSERT INTO funcs (module_id, lineno, name) VALUES (?, ?, ?)", &stmt_funcs_insert);
  SQLITE_PREPARE("SELECT id FROM funcs where module_id=? and lineno=? and name=?", &stmt_funcs_select);
}

void db_close() {
  SQLITE_EXEC("COMMIT");
}

static int handle_module(char *module) {
  sqlite3_reset(stmt_modules_select);
  SQLITE_ASSERT(sqlite3_bind_text(stmt_modules_select, 1, module, -1, SQLITE_TRANSIENT));
  if (SQLITE_ROW == sqlite3_step(stmt_modules_select)) {
    return sqlite3_column_int(stmt_modules_select, 0);
  } else {
    sqlite3_reset(stmt_modules_insert);
    SQLITE_ASSERT(sqlite3_bind_text(stmt_modules_insert, 1, module, -1, SQLITE_TRANSIENT));
    sqlite_status = sqlite3_step(stmt_modules_insert);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    return sqlite3_last_insert_rowid(db);
  }
}

static int handle_function(int module_id, int lineno, char *function) {
  sqlite3_reset(stmt_funcs_select);
  SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_select, 1, module_id));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_select, 2, lineno));
  SQLITE_ASSERT(sqlite3_bind_text(stmt_funcs_select, 3, function, -1, SQLITE_TRANSIENT));
  if (SQLITE_ROW == sqlite3_step(stmt_funcs_select)) {
    return sqlite3_column_int(stmt_modules_select, 0);
  } else {
    sqlite3_reset(stmt_funcs_insert);
    SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_insert, 1, module_id));
    SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_insert, 2, lineno));
    SQLITE_ASSERT(sqlite3_bind_text(stmt_funcs_insert, 3, function, -1, SQLITE_TRANSIENT));
    sqlite_status = sqlite3_step(stmt_funcs_insert);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    return sqlite3_last_insert_rowid(db);
  }
}

int db_handle_record(Record *rec) {
  handle_function(handle_module(rec->module), rec->lineno, rec->function);
}

/*
CREATE TABLE modules (
        id INTEGER NOT NULL, 
        value VARCHAR, 
        PRIMARY KEY (id), 
        UNIQUE (value)
)

CREATE TABLE arg_names (
        id INTEGER NOT NULL, 
        value VARCHAR, 
        PRIMARY KEY (id), 
        UNIQUE (value)
)

CREATE TABLE arg_values (
        id INTEGER NOT NULL, 
        value VARCHAR, 
        PRIMARY KEY (id), 
        UNIQUE (value)
)

CREATE TABLE types (
        id INTEGER NOT NULL, 
        value VARCHAR, 
        PRIMARY KEY (id), 
        UNIQUE (value)
)

CREATE TABLE args (
        id INTEGER NOT NULL, 
        type_id INTEGER NOT NULL, 
        name_id INTEGER NOT NULL, 
        value_id INTEGER NOT NULL, 
        PRIMARY KEY (id), 
        UNIQUE (type_id, name_id, value_id), 
        FOREIGN KEY(type_id) REFERENCES types (id), 
        FOREIGN KEY(name_id) REFERENCES arg_names (id), 
        FOREIGN KEY(value_id) REFERENCES arg_values (id)
)

CREATE TABLE funcs (
        id INTEGER NOT NULL, 
        module_id INTEGER NOT NULL, 
        type_id INTEGER NOT NULL, 
        name VARCHAR, 
        PRIMARY KEY (id), 
        UNIQUE (module_id, type_id, name), 
        FOREIGN KEY(module_id) REFERENCES modules (id), 
        FOREIGN KEY(type_id) REFERENCES types (id)
)

CREATE TABLE traces (
        id INTEGER NOT NULL, 
        type VARCHAR(9), 
        time DATETIME, 
        depth INTEGER, 
        func_id INTEGER, 
        PRIMARY KEY (id), 
        CHECK (type IN ('call', 'return', 'exception')), 
        FOREIGN KEY(func_id) REFERENCES funcs (id)
)

CREATE INDEX ix_traces_time ON traces (time)


CREATE TABLE association (
        trace_id INTEGER, 
        arg_id INTEGER, 
        FOREIGN KEY(trace_id) REFERENCES traces (id), 
        FOREIGN KEY(arg_id) REFERENCES args (id)
)


PRAGMA journal_mode=WAL;
INSERT INTO modules (value) VALUES (?)
INSERT INTO types (value) VALUES (?)
INSERT INTO funcs (module_id, type_id, name) VALUES (?, ?, ?)
INSERT INTO arg_names (value) VALUES (?)
INSERT INTO arg_values (value) VALUES (?)
INSERT INTO args (type_id, name_id, value_id) VALUES (?, ?, ?)
INSERT INTO traces (type, time, depth, func_id) VALUES (?, ?, ?, ?)
INSERT INTO association (trace_id, arg_id) VALUES (?, ?)
 */
