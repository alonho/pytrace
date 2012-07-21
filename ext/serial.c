#include <pthread.h>
#include "serial.h"
#include "write.h"
#include "defs.h"

#define PYPRINT(obj) PyObject_Print(obj, stdout, Py_PRINT_RAW);

static Record *record; // pre-allocate a single record to be re-used
static Argument **arguments; // pre-allocate maximum number of arguments
static unsigned char *record_buf;
static pthread_key_t depth_key;

void init_serialize(void) {
  ASSERT(0 == pthread_key_create(&depth_key, NULL));
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

inline char *pyobj_to_cstr(PyObject *obj) {
  PyObject *string = PyObject_Str(obj);
  if (NULL == string) {
    return "STR FAILED";
  }
  return PyString_AsString(string);
}

void set_string(ProtobufCBinaryData *bin_data, char *str) {
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

inline void handle_call(PyFrameObject *frame) {  
  PyObject *name, *value;
  int i;
  increment_depth();
  for (i = 0; i < min(frame->f_code->co_argcount, MAX_ARGS); i++) {
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
    } 
  }
  handle_trace(frame, RECORD__RECORD_TYPE__CALL, i);
}

inline void handle_return(PyFrameObject *frame, PyObject *value) {
  decrement_depth();
  set_string(&(arguments[0]->name), "return value");
  if (NULL == value) {
    value = Py_None;
  }
  set_string(&(arguments[0]->type), value->ob_type->tp_name);
  set_string(&(arguments[0]->value), pyobj_to_cstr(value));
  handle_trace(frame, RECORD__RECORD_TYPE__RETURN, 1);
}
    
inline void handle_exception(PyFrameObject *frame, PyObject *exc_info) {
  set_string(&(arguments[0]->name), "exception");
  set_string(&(arguments[0]->type), ((PyTypeObject*) PyTuple_GET_ITEM(exc_info, 0))->tp_name);
  set_string(&(arguments[0]->value), pyobj_to_cstr(PyTuple_GET_ITEM(exc_info, 1)));
  handle_trace(frame, RECORD__RECORD_TYPE__EXCEPTION, 1);
}

