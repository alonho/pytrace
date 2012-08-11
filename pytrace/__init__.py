import os
import threading
from functools import wraps
from . import tracer
from .context_utils import reentrentcontext

DEFAULT_MODULES = os.getenv('TRACE_MODULES')
if DEFAULT_MODULES is not None:
    DEFAULT_MODULES = DEFAULT_MODULES.split(':')

def start(filter_modules=DEFAULT_MODULES):
    tracer.init()
    if filter_modules is not None:
        tracer.set_filter_modules(filter_modules)
    threading.settrace(thread_trace)
    tracer.start_dumper()
    tracer.install() 

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

def notrace(func):
    """
    This decorator returns a function pointing to a code object with a name the tracer knows
    to skip. even though wraps copies the function's __name__, the code object still preserves it's name. (that's because code objects are immutable)
    """
    @wraps(func)
    def PYTRACE_OFF(*a, **k):
        return func(*a, **k)
    return PYTRACE_OFF