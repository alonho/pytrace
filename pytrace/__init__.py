from contextlib import contextmanager
import threading
import tracer

def start(filter_modules=None):
    if filter_modules is not None:
        tracer.set_filter_modules(filter_modules)
    tracer.start_dumper()
    tracer.install() 
    threading.settrace(thread_trace)

def thread_trace(*_):
    tracer.install()
    
def stop():
    tracer.uninstall()
    tracer.stop_dumper()
    threading.settrace(None)

@contextmanager
def trace_context(*a, **k):
    start(*a, **k)
    try:
        yield
    finally:
        stop()