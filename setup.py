from setuptools import setup
from distutils.core import Extension

OPTIMIZATIONS = False # by default extension are compiled with O2
extra_compile_args = [] if OPTIMIZATIONS else ["-O0"]

setup(name='pytrace',
      version='0.2.1',
      description='pytrace is a fast python tracer. pytrace records function calls, arguments and return values. traces aid debugging, profiling and obviate logging.',
      author='Alon Horev',
      author_email='alonho@gmail.com',
      url='https://github.com/alonho/pytrace',
      classifiers = ["Development Status :: 3 - Alpha",
                     "Intended Audience :: Developers",
                     "Environment :: Console :: Curses",
                     "License :: OSI Approved :: GNU Library or Lesser General Public License (LGPL)",
                     "Operating System :: POSIX :: Linux",
                     "Operating System :: MacOS :: MacOS X",
                     "Programming Language :: Python :: 2.7",
                     "Topic :: Software Development :: Debuggers"],
      license='LGPL',
      packages=['pytrace', 'pytrace.reader'],
      install_requires=['sqlalchemy',
                        'urwid',
                        'six'],
      ext_modules=[Extension("pytrace.tracer",
                             sources=["ext/trace.c",
                                      "ext/serial.c",
                                      "ext/write.c",
                                      "ext/ring.c",
                                      "ext/dump.c",
                                      "ext/db.c",
                                      "ext/record_pb.c",
                                      "ext/shared_ring.c"],
                             include_dirs=["ext",
                                           "/opt/local/include"], # where macports installs protobuf
                             libraries=["protobuf-c", "sqlite3"],
                             extra_compile_args=extra_compile_args)],
      entry_points={'console_scripts': ['pytrace=pytrace.__main__:main']})
