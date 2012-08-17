#include <pthread.h>
#include "serial.h"
#include "write.h"
#include "defs.h"

#define PYPRINT(obj) PyObject_Print(obj, stdout, Py_PRINT_RAW);

static Record *record; // pre-allocate a single record to be re-used
static Argument **arguments; // pre-allocate maximum number of arguments
static unsigned char *record_buf;
static pthread_key_t depth_key, no_trace_context_key;

static inline char *pyobj_to_cstr(PyObject *obj) {
  char *result;
  PyObject *string = PyObject_Repr(obj);
  if (NULL == string) {
    return "STR FAILED";
  }
  // an empty string in sqlite is interpreted as python None,
  // returning a pythonic empty string is prettier.
  result = PyString_AsString(string);
  if (result[0] == NULL) {
    return "''"; 
  }
  return result;
}

void init_serialize(void) {
  ASSERT(0 == pthread_key_create(&depth_key, NULL));
  ASSERT(0 == pthread_key_create(&no_trace_context_key, NULL));
  record_buf = malloc(MAX_RECORD_SIZE);
  record = malloc(sizeof(Record));
  ASSERT(NULL != record);
  record__init(record);
  arguments = malloc(sizeof(Argument*) * MAX_ARGS);
  ASSERT(NULL != arguments);
  record->arguments = arguments;
  int i;
  for (i = 0; i < MAX_ARGS; i++) {
    arguments[i] = malloc(sizeof(Argument));
    argument__init(arguments[i]);
  }
  init_writer();
}

void set_string(ProtobufCBinaryData *bin_data, const char *str) {
  bin_data->data = (unsigned char*) str;
  bin_data->len = min(strlen(str), MAX_STR_SIZE);
}

inline static int get_depth(void) {
  // if called before inc/dec will return NULL -> 0
  return (int) (long) pthread_getspecific(depth_key); 
}

inline static void increment_depth(void) {
  pthread_setspecific(depth_key, (void*) (long) (get_depth() + 1));
}  

inline static void decrement_depth(void) {
  pthread_setspecific(depth_key, (void*) (long) (get_depth() - 1));
}

#define DEPTH_MAGIC 10000

inline static void enter_no_trace_context(void) {
  pthread_setspecific(no_trace_context_key, 
		      (void*) (int) (long) get_depth() + DEPTH_MAGIC);
}

inline static void exit_no_trace_context(void) {
  pthread_setspecific(no_trace_context_key, NULL);
}

inline static int in_no_trace_context(void) {
  return NULL != pthread_getspecific(no_trace_context_key);
}

inline static int should_exit_no_trace_context(void) {
  return (get_depth() < pthread_getspecific(no_trace_context_key) - DEPTH_MAGIC);
}

int should_trace_frame(PyFrameObject *frame) {
  return !(0 == strncmp(PyString_AsString(frame->f_code->co_name), 
			DONT_TRACE_NAME, 
			strlen(DONT_TRACE_NAME)));
}

void handle_trace(PyFrameObject *frame, Record__RecordType record_type, int n_arguments) 
{
  record->type = record_type;
  record->n_arguments = n_arguments;
  record->time = floattime();
  record->tid = (long) pthread_self();
  record->depth = get_depth();
  set_string(&(record->module), PyString_AsString(frame->f_code->co_filename));
  set_string(&(record->function), PyString_AsString(frame->f_code->co_name));
  record->lineno = frame->f_code->co_firstlineno;
  record__pack(record, record_buf);
  write_record(record_buf, (unsigned long) record__get_packed_size(record));
}

void handle_call(PyFrameObject *frame) {  
  PyObject *name, *value;
  int i, argcount, count = 0;
  increment_depth();
  if (in_no_trace_context()) {
    return;
  }
  if (FALSE == should_trace_frame(frame)) {
    enter_no_trace_context();
    return;
  }

  argcount = frame->f_code->co_argcount;
  if (frame->f_code->co_flags & CO_VARARGS) {
    argcount++;
  }
  if (frame->f_code->co_flags & CO_VARKEYWORDS) {
    argcount++;
  }

  for (i = 0; i < min(argcount, MAX_ARGS); i++) {
    name = PyTuple_GetItem(frame->f_code->co_varnames, i);
    if (NULL == frame->f_locals) {
      value = frame->f_localsplus[i];
    } else {
      value = PyDict_GetItem(frame->f_locals, name);
    }
    if (NULL != value) { // happens when exec is used
      set_string(&(arguments[i]->name), PyString_AsString(name));
      set_string(&(arguments[i]->type), value->ob_type->tp_name);
      set_string(&(arguments[i]->value), pyobj_to_cstr(value));
      count++;
    } 
  }
  handle_trace(frame, RECORD__RECORD_TYPE__CALL, count);
}

void handle_return(PyFrameObject *frame, PyObject *value) {
  decrement_depth();
  if (in_no_trace_context()) {
    if (should_exit_no_trace_context()) {
      exit_no_trace_context();
    } 
    return;
  }

  set_string(&(arguments[0]->name), "return value");
  if (NULL == value) {
    value = Py_None;
  }
  set_string(&(arguments[0]->type), value->ob_type->tp_name);
  set_string(&(arguments[0]->value), pyobj_to_cstr(value));
  handle_trace(frame, RECORD__RECORD_TYPE__RETURN, 1);
}
    
void handle_exception(PyFrameObject *frame, PyObject *exc_info) {
  set_string(&(arguments[0]->name), "exception");
  set_string(&(arguments[0]->type), ((PyTypeObject*) PyTuple_GET_ITEM(exc_info, 0))->tp_name);
  set_string(&(arguments[0]->value), pyobj_to_cstr(PyTuple_GET_ITEM(exc_info, 1)));
  handle_trace(frame, RECORD__RECORD_TYPE__EXCEPTION, 1);
}

