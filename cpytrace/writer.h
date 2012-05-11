#include <pthread.h>

#include "Python.h"
#include "record.h"
#include "bla.h"

#define MAX(x, y) (x) > (y) ? (x) : (y)
#define MIN(x, y) (x) < (y) ? (x) : (y)
#define PYPRINT(obj) PyObject_Print(obj, stdout, Py_PRINT_RAW);

#define MAX_ARGS 128

static Record *rec; // pre-allocate a single record to be re-used
static Argument *arguments; // pre-allocate maximum number of arguments

inline void init_writer() {
  int i;
  bla();
  rec = malloc(sizeof(Record)); // TODO: check malloc retval
  arguments = malloc(sizeof(Argument) * MAX_ARGS); // TODO: check malloc retval
  rec->arguments = arguments;
  for (i = 0; i < MAX_ARGS; i++) {
    argument__init(&arguments[i]);
  }
}

const char *pyobj_to_cstr(PyObject *obj) {
  PyObject *string = PyObject_Repr(obj);
  if (NULL == string) {
    return "REPR FAILED";
  }
  return PyString_AsString(string);
}

static double floattime()
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return (double) t.tv_sec + t.tv_usec * 0.000001;
}

void handle_trace(PyFrameObject *frame, Record__RecordType record_type, int n_arguments) 
{
  rec->type = record_type;
  rec->n_arguments = n_arguments;
  rec->time = floattime();
  rec->tid = (unsigned int) pthread_self();
  rec->function = frame->f_code->co_name;
  rec->module = frame->f_code->co_filename;
  rec->depth = 0;
}

void handle_call(PyFrameObject *frame) {  
  PyObject *name, *value;
  int i;
  for (i = 0; i < MIN(frame->f_code->co_argcount, MAX_ARGS); i++) {
    name = PyTuple_GetItem(frame->f_code->co_varnames, i);
    arguments[i].name = PyString_AsString(name);
    if (NULL == frame->f_locals) {
      value = frame->f_localsplus[i];
    } else {
      value = PyDict_GetItem(frame->f_locals, name);
    }
    arguments[i].type = pyobj_to_cstr(value->ob_type);
    arguments[i].value = pyobj_to_cstr(value);
  }
  handle_trace(frame, RECORD__RECORD_TYPE__CALL, i);
}

void handle_return(PyFrameObject *frame, PyObject *value) {
  arguments[0].name = "return_value";
  arguments[0].type = pyobj_to_cstr(value->ob_type);
  arguments[0].value = pyobj_to_cstr(value);
  handle_trace(frame, RECORD__RECORD_TYPE__RETURN, 1);
}
    
void handle_exception(PyFrameObject *frame, PyObject *exc_info) {
  arguments[0].name = "exception";
  arguments[0].type = pyobj_to_cstr(PyTuple_GET_ITEM(exc_info, 1));
  arguments[0].value = pyobj_to_cstr(PyTuple_GET_ITEM(exc_info, 2));
  handle_trace(frame, RECORD__RECORD_TYPE__EXCEPTION, 1);
}
