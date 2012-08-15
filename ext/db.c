/*
sqlite3 ./db.sqlite "select traces.id, time, tid, type, depth, modules.value, funcs.name, types.value, arg_values.value, arg_names.value from traces join funcs on (traces.func_id=funcs.id) join modules on (funcs.module_id=modules.id) join association on (traces.id=association.trace_id) join args on (association.arg_id=args.id) join arg_names on (args.name_id=arg_names.id) join arg_values on (args.value_id=arg_values.id) join types on (args.type_id=types.id);"
 */
#include <sqlite3.h>
#include "record_pb.h"
#include "defs.h"
#include "db.h"

#define SQLITE_ASSERT(x) ASSERT(SQLITE_OK == (x))
#define SQLITE_DONE_OR_CONSTRAINT(x) ASSERT(SQLITE_DONE == (x) || SQLITE_CONSTRAINT == (x))
#define SQLITE_EXEC(query) SQLITE_ASSERT(sqlite3_exec(db, (query), NULL, NULL, NULL))
#define SQLITE_PREPARE(query, stmt) SQLITE_ASSERT(sqlite3_prepare_v2(db, (query), -1, (stmt), NULL))

sqlite3 *db;
sqlite3_stmt *stmt_modules_insert, *stmt_modules_select, *stmt_funcs_insert, *stmt_funcs_select, *stmt_types_insert, *stmt_types_select, *stmt_arg_names_insert, *stmt_arg_names_select, *stmt_arg_values_insert, *stmt_arg_values_select, *stmt_args_insert, *stmt_args_select, *stmt_traces_insert, *stmt_assoc_insert, *stmt_delete_args, *stmt_delete_types, *stmt_delete_assoc, *stmt_delete_funcs, *stmt_delete_traces, *stmt_delete_modules, *stmt_delete_arg_names, *stmt_delete_arg_values;

void db_begin(void) {
  SQLITE_EXEC("BEGIN");
}

#define COMMIT_RETRIES 10
void db_commit(void) {
  int i, status;
  for (i = 0; i < COMMIT_RETRIES; i++) {
    status = sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
    if (SQLITE_BUSY == status) {
      continue;
    } else {
      SQLITE_ASSERT(status);
      db_begin();
      return;
    }
  }
}

void db_truncate(int count) {
  int sqlite_status;
  sqlite3_reset(stmt_delete_traces);
  SQLITE_ASSERT(sqlite3_bind_int(stmt_delete_traces, 1, count));
  sqlite_status = sqlite3_step(stmt_delete_traces);
  SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
  if (sqlite3_changes(db) > 0) {
    sqlite3_reset(stmt_delete_funcs);
    sqlite_status = sqlite3_step(stmt_delete_funcs);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    sqlite3_reset(stmt_delete_modules);
    sqlite_status = sqlite3_step(stmt_delete_modules);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    sqlite3_reset(stmt_delete_assoc);
    sqlite_status = sqlite3_step(stmt_delete_assoc);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    sqlite3_reset(stmt_delete_args);
    sqlite_status = sqlite3_step(stmt_delete_args);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    sqlite3_reset(stmt_delete_types);
    sqlite_status = sqlite3_step(stmt_delete_types);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    sqlite3_reset(stmt_delete_arg_names);
    sqlite_status = sqlite3_step(stmt_delete_arg_names);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    sqlite3_reset(stmt_delete_arg_values);
    sqlite_status = sqlite3_step(stmt_delete_arg_values);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
  }
}

void db_init(void) {
  // config is effective only before open, avoid mutexes for performance
  sqlite3_config(SQLITE_CONFIG_SINGLETHREAD); 
  // WAL is a fast journal
  SQLITE_ASSERT(sqlite3_open_v2("traces.sqlite", 
				&db, 
				SQLITE_OPEN_CREATE | 
				SQLITE_OPEN_READWRITE | 
				SQLITE_OPEN_WAL, 
				NULL));

  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS  modules (           \
        id INTEGER NOT NULL,				       \
        value VARCHAR,					       \
        PRIMARY KEY (id),				       \
        UNIQUE (value))");
  
  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS funcs (                       \
        id INTEGER NOT NULL,						\
        module_id INTEGER NOT NULL,					\
        lineno INTEGER,							\
        name VARCHAR,							\
        PRIMARY KEY (id),						\
        UNIQUE (module_id, lineno, name),				\
        FOREIGN KEY(module_id) REFERENCES modules (id))");

  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS arg_names ( \
        id INTEGER NOT NULL,			      \
        value VARCHAR,				      \
        PRIMARY KEY (id),			      \
        UNIQUE (value))");

  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS arg_values ( \
        id INTEGER NOT NULL,			       \
        value VARCHAR,				       \
        PRIMARY KEY (id),			       \
        UNIQUE (value))");

  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS types ( \
        id INTEGER NOT NULL,			  \
        value VARCHAR,				  \
        PRIMARY KEY (id),			  \
        UNIQUE (value))");
  
  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS args (        \
        id INTEGER NOT NULL,				\
        type_id INTEGER NOT NULL,			\
        name_id INTEGER NOT NULL,			\
        value_id INTEGER NOT NULL,			\
        PRIMARY KEY (id),				\
        UNIQUE (type_id, name_id, value_id),		\
        FOREIGN KEY(type_id) REFERENCES types (id),	\
        FOREIGN KEY(name_id) REFERENCES arg_names (id), \
        FOREIGN KEY(value_id) REFERENCES arg_values (id))");

  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS traces (       \
        id INTEGER NOT NULL,				 \
        type VARCHAR(9),				 \
        time REAL,					 \
        depth INTEGER,					 \
        tid INTEGER,					 \
        func_id INTEGER,				 \
        PRIMARY KEY (id),				 \
        FOREIGN KEY(func_id) REFERENCES funcs (id))");

  SQLITE_EXEC("CREATE INDEX IF NOT EXISTS ix_traces_time ON traces (time)");

  SQLITE_EXEC("CREATE TABLE IF NOT EXISTS association ( \
        trace_id INTEGER,			       \
        arg_id INTEGER,				       \
        FOREIGN KEY(trace_id) REFERENCES traces (id),  \
        FOREIGN KEY(arg_id) REFERENCES args (id))");

  SQLITE_PREPARE("INSERT INTO modules (value) VALUES (?)", &stmt_modules_insert);
  SQLITE_PREPARE("SELECT id FROM modules WHERE value=?", &stmt_modules_select);
  SQLITE_PREPARE("INSERT INTO funcs (module_id, lineno, name) VALUES (?, ?, ?)", &stmt_funcs_insert);
  SQLITE_PREPARE("SELECT id FROM funcs WHERE module_id=? AND lineno=? AND name=?", &stmt_funcs_select);
  SQLITE_PREPARE("INSERT INTO types (value) VALUES (?)", &stmt_types_insert);
  SQLITE_PREPARE("SELECT id FROM types WHERE value=?", &stmt_types_select);
  SQLITE_PREPARE("INSERT INTO arg_names (value) VALUES (?)", &stmt_arg_names_insert);
  SQLITE_PREPARE("SELECT id FROM arg_names WHERE value=?", &stmt_arg_names_select);
  SQLITE_PREPARE("INSERT INTO arg_values (value) VALUES (?)", &stmt_arg_values_insert);
  SQLITE_PREPARE("SELECT id FROM arg_values WHERE value=?", &stmt_arg_values_select);
  SQLITE_PREPARE("INSERT INTO args (type_id, name_id, value_id) VALUES (?, ?, ?)", &stmt_args_insert);
  SQLITE_PREPARE("SELECT id FROM args WHERE type_id=? AND name_id=? AND value_id=?", &stmt_args_select);
  SQLITE_PREPARE("INSERT INTO traces (type, time, depth, tid, func_id) VALUES (?, ?, ?, ?, ?)", &stmt_traces_insert);
  SQLITE_PREPARE("INSERT INTO association (trace_id, arg_id) VALUES (?, ?)", &stmt_assoc_insert);

  SQLITE_PREPARE("DELETE FROM traces WHERE id NOT IN (SELECT id FROM traces ORDER BY id DESC limit (?));", &stmt_delete_traces);
  SQLITE_PREPARE("DELETE FROM funcs WHERE id NOT IN (SELECT DISTINCT func_id FROM traces);", &stmt_delete_funcs);
  SQLITE_PREPARE("DELETE FROM modules WHERE id NOT IN (SELECT DISTINCT module_id FROM funcs);", &stmt_delete_modules);
  SQLITE_PREPARE("DELETE FROM association WHERE trace_id NOT IN (SELECT id FROM traces);", &stmt_delete_assoc);
  SQLITE_PREPARE("DELETE FROM args WHERE id NOT IN (SELECT arg_id FROM association);", &stmt_delete_args);
  SQLITE_PREPARE("DELETE FROM arg_names WHERE id NOT IN (SELECT name_id FROM args);", &stmt_delete_arg_names);
  SQLITE_PREPARE("DELETE FROM arg_values WHERE id NOT IN (SELECT value_id FROM args);", &stmt_delete_arg_values);
  SQLITE_PREPARE("DELETE FROM types WHERE id NOT IN (SELECT type_id FROM args);", &stmt_delete_types);

  db_begin();
}

static int get_or_create(sqlite3_stmt *select, sqlite3_stmt *insert, ProtobufCBinaryData *sym) {
  int sqlite_status;
  sqlite3_reset(select);
  SQLITE_ASSERT(sqlite3_bind_text(select, 1, (char*) sym->data, sym->len, SQLITE_TRANSIENT));
  if (SQLITE_ROW == sqlite3_step(select)) {
    return sqlite3_column_int(select, 0);
  } else {
    sqlite3_reset(insert);
    SQLITE_ASSERT(sqlite3_bind_text(insert, 1, (char*) sym->data, sym->len, SQLITE_TRANSIENT));
    sqlite_status = sqlite3_step(insert);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    return sqlite3_last_insert_rowid(db);
  }  
}

static int handle_module(ProtobufCBinaryData *module) {
  return get_or_create(stmt_modules_select, stmt_modules_insert, module);
}

static int handle_function(int module_id, int lineno, ProtobufCBinaryData *function) {
  int sqlite_status;
  sqlite3_reset(stmt_funcs_select);
  SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_select, 1, module_id));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_select, 2, lineno));
  SQLITE_ASSERT(sqlite3_bind_text(stmt_funcs_select, 3, (char*) function->data, function->len, SQLITE_TRANSIENT));
  if (SQLITE_ROW == sqlite3_step(stmt_funcs_select)) {
    return sqlite3_column_int(stmt_funcs_select, 0);
  } else {
    sqlite3_reset(stmt_funcs_insert);
    SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_insert, 1, module_id));
    SQLITE_ASSERT(sqlite3_bind_int(stmt_funcs_insert, 2, lineno));
    SQLITE_ASSERT(sqlite3_bind_text(stmt_funcs_insert, 3, (char*) function->data, function->len, SQLITE_TRANSIENT));
    sqlite_status = sqlite3_step(stmt_funcs_insert);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    return sqlite3_last_insert_rowid(db);
  }
}

static char *types[] = {"call", "return", "exception", "log", "overflow"};
static int handle_trace(Record__RecordType type, double time, int depth ,long tid, int func_id) {
  int sqlite_status;
  sqlite3_reset(stmt_traces_insert);
  SQLITE_ASSERT(sqlite3_bind_text(stmt_traces_insert, 1, types[type], -1, SQLITE_TRANSIENT));
  SQLITE_ASSERT(sqlite3_bind_double(stmt_traces_insert, 2, time));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_traces_insert, 3, depth));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_traces_insert, 4, tid));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_traces_insert, 5, func_id));
  sqlite_status = sqlite3_step(stmt_traces_insert);
  ASSERT(sqlite_status == SQLITE_DONE);
  return sqlite3_last_insert_rowid(db);
}


static int handle_type(ProtobufCBinaryData *type) {
  return get_or_create(stmt_types_select, stmt_types_insert, type);
}

static int handle_arg_name(ProtobufCBinaryData *name) {
  return get_or_create(stmt_arg_names_select, stmt_arg_names_insert, name);
}

static int handle_arg_value(ProtobufCBinaryData *name) {
  return get_or_create(stmt_arg_values_select, stmt_arg_values_insert, name);
}

static int handle_argument(Argument *arg) {
  int sqlite_status, type_id=handle_type(&arg->type), name_id=handle_arg_name(&arg->name), value_id=handle_arg_value(&arg->value);
  sqlite3_reset(stmt_args_select);
  SQLITE_ASSERT(sqlite3_bind_int(stmt_args_select, 1, type_id));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_args_select, 2, name_id));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_args_select, 3, value_id));
  if (SQLITE_ROW == sqlite3_step(stmt_args_select)) {
    return sqlite3_column_int(stmt_args_select, 0);
  } else {
    sqlite3_reset(stmt_args_insert);
    SQLITE_ASSERT(sqlite3_bind_int(stmt_args_insert, 1, type_id));
    SQLITE_ASSERT(sqlite3_bind_int(stmt_args_insert, 2, name_id));
    SQLITE_ASSERT(sqlite3_bind_int(stmt_args_insert, 3, value_id));
    sqlite_status = sqlite3_step(stmt_args_insert);
    SQLITE_DONE_OR_CONSTRAINT(sqlite_status);
    return sqlite3_last_insert_rowid(db);
  }
}

static void handle_trace_argument(int trace_id, Argument *arg) {
  int sqlite_status;
  sqlite3_reset(stmt_assoc_insert);
  SQLITE_ASSERT(sqlite3_bind_int(stmt_assoc_insert, 1, trace_id));
  SQLITE_ASSERT(sqlite3_bind_int(stmt_assoc_insert, 2, handle_argument(arg)));
  sqlite_status = sqlite3_step(stmt_assoc_insert);
  ASSERT(sqlite_status == SQLITE_DONE);
}

int db_handle_record(Record *rec) {
  int i;
  int func_id = handle_function(handle_module(&rec->module), rec->lineno, &rec->function);
  int trace_id = handle_trace(rec->type, rec->time, rec->depth, rec->tid, func_id);
  for (i=0; i < rec->n_arguments; i++) {
    handle_trace_argument(trace_id, rec->arguments[i]);
  }
  return trace_id;
}

int db_handle_lost(void) {
  return handle_trace(RECORD__RECORD_TYPE__OVERFLOW, floattime(), -1, -1, -1);
}
