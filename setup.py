from setuptools import setup
from distutils.core import Extension

setup(name='pytrace',
      packages=['pytrace'],
      ext_modules=[Extension("pytrace.tracer",
                             sources=["ext/trace.c",
                                      "ext/serial.c",
                                      "ext/write.c",
                                      "ext/ring.c",
                                      "ext/shared_ring.c",
                                      "ext/record.pb-c.c"],
                             libraries=["protobuf-c"])])