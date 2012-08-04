from pytrace import notrace

def fib(n):
    if n in (1,2):
        return 1
    return fib(n - 1) + fib(n - 2)

def simple(arg):
    pass

@notrace
def test_no_trace():
    pass
    
def default(arg=10):
    mix(10)

def vararg(*a):
    pass

def kwargs(**k):
    pass

def mix(a, b=1, *c, **d):
    pass

def raises(x):
    raise x

def exceptions():
    try:
        raises(ValueError("BLA"))
    except:
        pass
    else:
        assert False
    return "OK"
        
class A(object):

    def foo(self):
        return self
    
if __name__ == '__main__':
    import pytrace
    import time

    t = time.time()
    with pytrace.trace_context():
        simple(10)
        vararg(10, 20)
        kwargs(a=10, b=20)
        exceptions()
        A().foo()
        fib(20)
        mix(10, 20, 30, "bla", k="BLO")
        vararg(1, 2, 3)
        print time.time() - t
