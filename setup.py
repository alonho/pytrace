from setuptools import setup
from distutils.core import Extension

setup(name='pytrace',
      packages=['pytrace'],
      ext_modules=[Extension("pytrace.tracer",
                             sources=["pytrace/tracer.c"])])