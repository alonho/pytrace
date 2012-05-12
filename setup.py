from setuptools import setup
from distutils.core import Extension

setup(name='cpytrace',
      packages=['cpytrace'],
      ext_modules=[Extension("cpytrace.tracer",
                             sources=["cpytrace/trace.c",
                                      "cpytrace/record.c"],
                             libraries=["protobuf-c"])])