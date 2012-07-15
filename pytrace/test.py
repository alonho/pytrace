
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
    import pytrace.tracer as tracer
    import time
    t = time.time()
    tracer.start(['build'])
    fib(3)
    default()
    vararg(1, 2, 3)
    kwargs(a1=0, a2=1, a3=2)
    mix(100)
    simple("abcde" * 1000)
    simple(u'\x01\x1b[0;34m\x02In [{color.number}{count}{color.prompt}]: \x01\x1b[0m\x02')
    simple("")
    exceptions()
    print time.time() - t
    tracer.stop()
