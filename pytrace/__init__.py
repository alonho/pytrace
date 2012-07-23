import os
import threading
from . import tracer
from .context_utils import reentrentcontext

DEFAULT_MODULES = os.getenv('TRACE_MODULES')
if DEFAULT_MODULES is not None:
    DEFAULT_MODULES = DEFAULT_MODULES.split(':')

def start(filter_modules=DEFAULT_MODULES):
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

@reentrentcontext
def trace_context(*a, **k):
    start(*a, **k)
    try:
        yield
    finally:
        stop()