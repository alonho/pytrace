from setuptools import setup
from distutils.core import Extension

setup(name='cpytrace',
      packages=['cpytrace'],
      ext_modules=[Extension("cpytrace.tracer",
                             sources=["cpytrace/trace.c",
                                      "cpytrace/serial.c",
                                      "cpytrace/write.c",
                                      "cpytrace/record.pb-c.c",
                                      "cpytrace/ring.c",
                                      "cpytrace/shared_ring.c"],
                             libraries=["protobuf-c"])])