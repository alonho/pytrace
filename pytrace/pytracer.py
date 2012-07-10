from sys import setprofile
from datetime import datetime
from inspect import getframeinfo
from pytrace.db import DB

def format_frame(frame):
    frame_info = getframeinfo(frame)
    code_context = frame_info.code_context[0].strip()
    return "{0.filename}:{0.lineno}:{0.function}:{1}".format(frame_info, code_context)

class BaseTracer(object):
    
    def _trace(self, frame, event, arg):
        handler = getattr(self, 'on_' + event, None)
        if handler is not None:
            return handler(frame, arg)

    def install(self):
        setprofile(self._trace)

    def uninstall(self):
        setprofile(None)

    def __enter__(self):
        self.install()
        return self

    def __exit__(self, *e):
        self.uninstall()

class Tracer(BaseTracer):

    def __init__(self):
        self._db = DB("sqlite:///db.sqlite")
        self._depth = 0

    def on_call(self, frame, _):
        self._db.handle_call(time=datetime.now(),
                             module=__name__,
                             func_name=getframeinfo(frame).function,
                             lineno=100,
                             depth=self._depth,
                             args=frame.f_locals)
                             
        #print "+" * self._depth, format_frame(frame), "arguments: ", ", ".join('{}={}'.format(k, v) for k, v in frame.f_locals.iteritems())
        self._depth += 1
        return self._trace # means trace everything under this scope
        
    def on_return(self, frame, arg): 
        self._depth -= 1
        #print "-" * self._depth,  format_frame(frame), "return: ", arg

    def __exit__(self, *e):
        super(Tracer, self).__exit__(*e)
        self._db.flush()