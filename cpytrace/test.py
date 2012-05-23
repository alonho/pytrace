
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

def sanity(a, b=1, *c, **d):
    pass
    
if __name__ == '__main__':
    import cpytrace.tracer as tracer
    tracer.start()
    #fib(30)
    sanity(100000000000000000, [10]*200)
    tracer.stop()
    # 2.5 seconds run time with traces (1 second spent by record_pack)
    # 0.387 without