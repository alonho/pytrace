from pytrace import Tracer

def fib(n):
    if n in (1,2):
        return 1
    return fib(n - 1) + fib(n - 2)
    
if __name__ == '__main__':
    n = 1
    with Tracer():
        import time
        s = time.time()
        fib(n)
        print "traced: ", time.time() - s
    s = time.time()
    fib(n)
    print "untraced: ", time.time() - s
    
        