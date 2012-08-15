#ifndef SERIAL_H
#define SERIAL_H

#include "record_pb.h"
#include "Python.h"
#include "frameobject.h"

void init_serialize(void);
void handle_trace(PyFrameObject *frame, Record__RecordType record_type, int n_arguments);
void handle_call(PyFrameObject *frame);
void handle_return(PyFrameObject *frame, PyObject *value);
void handle_exception(PyFrameObject *frame, PyObject *exc_info);
int should_trace_frame(PyFrameObject *frame);

#endif /* serial */
