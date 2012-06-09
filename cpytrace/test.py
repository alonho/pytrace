
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
        
class A(object):

    def foo(self):
        return self
    
if __name__ == '__main__':
    import cpytrace.tracer as tracer
    tracer.start()
    fib(28)
    simple(15)
    default()
    vararg(1, 2, 3)
    kwargs(a1=0, a2=1, a3=2)
    exceptions()
    mix(100)
    simple("a" * 3000)
    tracer.stop()
