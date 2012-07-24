from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from .tables import Base, Func, Module, Arg, ArgValue, ArgName, Type, Trace

class DB(object):

    ECHO = True
    COMMIT_INTERVAL = 100
    
    def __init__(self, uri):
        engine = create_engine(uri, echo=self.ECHO)
        Base.metadata.create_all(engine)
        self.session = sessionmaker(bind=engine)()
        self._interval = 0
        
    @classmethod
    def create_inmemory(cls):
        return cls(uri="sqlite:///:memory:")

    def _commit_if_needed(self):
        self._interval += 1
        if self._interval > self.COMMIT_INTERVAL:
            self._interval = 0
            #self.session.commit()

    def flush(self):
        self.session.commit()
        self._interval = 0
            

    def handle_trace(self, trace_type, time, module, func_name, lineno, depth, args):
        import time as tm
        stime = tm.time()
        func = Func.get_or_create(self.session,
                                  module=Module.get_or_create(self.session, value=module),
                                  lineno=lineno,
                                  name=func_name)
        args = [Arg.get_or_create(self.session, type=Type.get_or_create(self.session, value=str(type(value))),
                                  name=ArgName.get_or_create(self.session, value=key),
                                  value=ArgValue.get_or_create(self.session, value=str(value)))                for key, value in args.iteritems()]
        self.session.add(Trace(type=trace_type,
                               time=time,
                               depth=depth,
                               func=func,
                               args=args))
        self._commit_if_needed()
        print tm.time() - stime

    def handle_call(self, *a, **k):
        self.handle_trace("call", *a, **k)

    def handle_return(self, value, *a, **k):
        self.handle_trace("return", args=dict(return_value=value), *a, **k)

    def handle_exception(self, exception, *a, **k):
        self.handle_trace("exception", args=dict(exception=exception), *a, **k)
