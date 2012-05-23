from setuptools import setup
from distutils.core import Extension

setup(name='cpytrace',
      packages=['cpytrace'],
      ext_modules=[Extension("cpytrace.tracer",
                             sources=["cpytrace/trace.c",
                                      "cpytrace/write.c",
                                      "cpytrace/serial.c",
                                      "cpytrace/record.c",
                                      "cpytrace/rb/rb.c",
                                      "cpytrace/codec.c",
                                      "cpytrace/dump.c"],
                             libraries=["protobuf-c"])])