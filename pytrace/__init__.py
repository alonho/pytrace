import tracer

def trace_context():
    tracer.start()
    try:
        yield
    finally:
        tracer.stop()