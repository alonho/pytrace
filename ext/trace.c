#include <stdlib.h>
#include "Python.h"
#include "frameobject.h"
#include "serial.h"
#include "dump.h"

#define MODULE_DOC PyDoc_STR("C extension for fast function tracing.")

static int
trace_func(PyObject *obj, PyFrameObject *frame, int what, PyObject *arg)
{
  switch (what) {
  case PyTrace_CALL:
    handle_call(frame);
    break;
  case PyTrace_RETURN:
    handle_return(frame, arg);
    break;
  case PyTrace_EXCEPTION: // setprofile translates exceptions to calls
    handle_exception(frame, arg);
  }
  return 0;
}

static PySetObject *module_set;

static PyObject*
start(PyObject *self, PyObject *args)
{
  if (!PyArg_ParseTuple(args, "O!", &PySet_Type, &module_set)){
    return;
  }
  PyEval_SetTrace((Py_tracefunc) trace_func,
		  (PyObject*) self);
  dump_main_in_thread();
  return Py_BuildValue("");
}

static PyObject*
stop(PyObject *self, PyObject *args)
{
  dump_stop();
  PyEval_SetTrace(NULL, NULL);
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
  init_serialize();
  Py_InitModule3("pytrace.tracer", methods, MODULE_DOC);
}
