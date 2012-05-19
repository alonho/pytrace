
def fib(n):
    if n in (1,2):
        return 1
    return fib(n - 1) + fib(n - 2)

if __name__ == '__main__':
    import cpytrace.tracer as tracer
    tracer.start()
    fib(30)
    tracer.stop()
    # 2.5 seconds run time with traces (1 second spent by record_pack)
    # 0.387 without