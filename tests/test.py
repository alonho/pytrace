
def fib(n):
    if n in (1,2):
        return 1
    return fib(n - 1) + fib(n - 2)

def simple(arg):
    pass

def default(arg=10):
    pass

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
        fib(10)
        print time.time() - t
