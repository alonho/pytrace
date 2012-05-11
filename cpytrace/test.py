
def g(x):
    return x * 2

def f():
    g(10)

if __name__ == '__main__':
    import cpytrace.tracer as tracer
    tracer.start()
    f()
    tracer.stop()