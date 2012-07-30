from setuptools import setup
from distutils.core import Extension

OPTIMIZATIONS = False # by default extension are compiled with O2
extra_compile_args = [] if OPTIMIZATIONS else ["-O0"]

setup(name='pytrace',
      packages=['pytrace', 'pytrace.reader'],
      install_requires=['sqlalchemy',
                        'urwid'],
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
                             include_dirs=["include"],
                             libraries=["protobuf-c", # should be found in 'libs'
                                        "sqlite3"],
                             extra_compile_args=extra_compile_args)])