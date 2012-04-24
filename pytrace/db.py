from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from .tables import Base, Func, Module, Arg, ArgValue, ArgName, Type, Trace

engine = create_engine("sqlite:///:memory:", echo=True)
Base.metadata.create_all(engine)
Session = sessionmaker(bind=engine)

def DB(object):

    def __init__(self):
        pass

    def handle_call(self, type_, time, module, func, depth, args):
        session = Session()
        func = Func.get_or_create(session, module=Module.get_or_create(session, value=module))
        args = [Arg.get_or_create(session, type=Type.get_or_create(session, value=type(value)),
                                  name=ArgName.get_or_create(session, value=key),
                                  value=ArgValue.get_or_create(session, value=value))
                for key, value in args.iteritems()]
        session.add(Trace(type=type_,
                          time=time,
                          depth=depth,
                          func=func,
                          args=args))
        session.flush()
        
    def handle_return(self, *a, **k):
        pass

    def handle_exception(self, *a, **k):
        pass

