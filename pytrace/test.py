
def g(x):
    return x * 2

def f():
    g(10)

if __name__ == '__main__':
    import pytrace.tracer
    pytrace.tracer.start()
    f()
    pytrace.tracer.stop()