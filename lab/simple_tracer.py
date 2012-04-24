from sys import settrace, argv, exit
from inspect import getframeinfo

def format_frame(frame):
    return "{0.filename}:{0.lineno}:{0.function}:{0.code_context}".format(getframeinfo(frame))

class Tracer(object):
    
    def _trace(self, frame, event, arg):
        handler = getattr(self, 'on_' + event, None)
        if handler is not None:
            return handler(frame, arg)

    def install(self):
        settrace(self._trace)

    def uninstall(self):
        settrace(None)

    def __enter__(self):
        self.install()
        return self

    def __exit__(self, *e):
        self.uninstall()

class TracerExample(Tracer):

    def __init__(self):
        self._depth = 0

    def on_call(self, frame, _):
        print "+" * self._depth, format_frame(frame), "arguments: ", ", ".join('{}={}'.format(k, v) for k, v in frame.f_locals.iteritems())
        self._depth += 1
        return self._trace # means trace everything under this scope
        
    def on_return(self, frame, arg): 
        self._depth -= 1
        print "-" * self._depth,  format_frame(frame), "return: ", arg

def main():
    if len(argv) != 2:
        print "usage: python trace.py some_script.py"
        exit(1)
    
    with TracerExample():
        execfile(argv[1])
        
if __name__ == '__main__':
    main()
