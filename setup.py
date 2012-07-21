from setuptools import setup
from distutils.core import Extension
import platform

OPTIMIZATIONS = True
extra_compile_args = [] if OPTIMIZATIONS else ["-O0"]

setup(name='pytrace',
      packages=['pytrace'],
      ext_modules=[Extension("pytrace.tracer",
                             sources=["ext/trace.c",
                                      "ext/serial.c",
                                      "ext/write.c",
                                      "ext/ring.c",
                                      "ext/dump.c",
                                      "ext/db.c",
                                      "ext/shared_ring.c",
                                      "ext/record.pb-c.c",
                                      ],
                             library_dirs=["libs"],
                             libraries=["protobuf-c",
                                        "sqlite3"],
                             extra_compile_args=extra_compile_args)])