from contextlib import contextmanager

@contextmanager
def noop_context():
    yield
    
def reentrentcontext(func):
    context = contextmanager(func)
    entered = []
    def decorated(*a, **k):
        if entered == []:
            entered.append(True) # I miss nonlocal
            return context(*a, **k)
        return noop_context()
    return decorated
