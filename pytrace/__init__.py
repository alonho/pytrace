from contextlib import contextmanager
import threading
import tracer

global_filter_modules = None
def start(filter_modules=None):
    global global_filter_modules
    global_filter_modules = filter_modules
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