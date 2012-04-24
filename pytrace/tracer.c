#include <stdlib.h>
#include "Python.h"
#include "frameobject.h"

#define MODULE_DOC PyDoc_STR("C extension for fast function tracing.")

#define LOG(...) printf(__VA_ARGS__);
#define MAX(x, y) (x) > (y) ? (x) : (y)
#define PYPRINT(obj) PyObject_Print(obj, stdout, Py_PRINT_RAW);

void print_args(PyFrameObject *frame) {
  int i;
  PyObject *obj;
  for (i = 0; i < frame->f_code->co_argcount; i++) {
    obj = PyTuple_GetItem(frame->f_code->co_varnames, i);
    LOG(" ");
    PYPRINT(obj);
    LOG(" = ");
    if (NULL == frame->f_locals) {
      obj = frame->f_localsplus[i];
    } else {
      obj = PyDict_GetItem(frame->f_locals, obj);
    }
    PYPRINT(obj);
  }
}

void inline print_char(char c, int times) {
  while (times--) {
    printf("%c", c);
  }
}

static int
trace_func(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
  static int depth = 0;

  switch (what) {
  case PyTrace_CALL:
    depth++;
    print_char('>', depth);
    LOG(" ");
    PYPRINT(frame->f_code->co_name);
    print_args(frame);
    LOG("\n");
    break;
  case PyTrace_RETURN:
    print_char('<', depth);
    depth--;
    if (depth >= 0) {
      LOG(" ");
      PYPRINT(arg);
      LOG("\n");
    }
    break;
  }
  return 0;
}

static PyObject *
start(PyObject *self, PyObject *args)
{
  PyEval_SetProfile((Py_tracefunc)trace_func,
		  (PyObject*)self);
  return Py_BuildValue("");
}

static PyObject *
stop(PyObject *self, PyObject *args_unused)
{
  PyEval_SetProfile(NULL, NULL);
  return Py_BuildValue("");
}

static PyMethodDef
methods[] = {
  {"start", (PyCFunction) start, METH_VARARGS, PyDoc_STR("Start the tracer")},
  {"stop", (PyCFunction) stop, METH_VARARGS, PyDoc_STR("Stop the tracer")},
  {NULL}
};

void
inittracer(void)
{
  Py_InitModule3("pytrace.tracer", methods, MODULE_DOC);
}
