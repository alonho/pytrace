pytrace - a fast python tracer
==============================

pytrace records function calls, arguments and return values.

traces aid debugging, profiling and obviate logging.


pytrace has been tested on python 2.7 and python 3.2. (should support 2.6 and up)

pytrace has been tested on os x and several linux distributions.


Follow **@alonhorev** on **twitter** for updates.

Install
-------

pytrace depends on sqlite and a C implementation of protocol buffers.

on debian/ubuntu: `sudo apt-get install libsqlite3-dev libprotobuf-c0-dev`

on fedora: `sudo yum install libsqlite3x-devel protobuf-c-devel`

on mac (sqlite is included): `brew install protobuf-c` or `port install protobuf-c`

install pytrace:

	pip install pytrace

Usage
-----

Invoke pytrace with your script:

	$ pytrace foo.py --bar
	
Invoke the reader from the same directory by executing pytrace with no arguments:
	
	$ pytrace

The reader can be invoked while the script is running. providing 'online' debugging capabilities.

Reader
------

The collected data can be viewed in an interactive reader. The reader supports less-like key bindings.

![trace reader](https://github.com/alonho/pytrace/raw/master/pics/view.png)

The reader can search for regular expressions:

![trace search](https://github.com/alonho/pytrace/raw/master/pics/search.png)

The reader can filter traces using a python expressions. The following fields can be used for filters:

1. time - int
2. tid - int
3. module - string
4. func - string
5. arg (argument name) - string. special arguments are 'return value' and 'exception' used to filter function return values and exceptions.
6. value (argument value)  - string
7. type (argument type) - string

Field types:

1. int fields - supports algebric operators (>, <, >=, <=, ==). e.g: `time > '2012/08/15 01:23:45'`.
2. string fields: support string comparison (==, !=). strings comparison supports sql 'like' syntax. for example: `module == 'proj%'` filters modules starting with 'proj'.

![trace filter](https://github.com/alonho/pytrace/raw/master/pics/filter.png)

The reader corrects queries that don't match anything:

![filter correction](https://github.com/alonho/pytrace/raw/master/pics/correction.png)

The Database
------------

The database is saved in the current working directory and is named `traces.sqlite`.

In order to not run out of **disk space**, The database will be truncated to a fixed number of traces (currently hard coded to 10000). 

Reducing the overhead
---------------------

Hot functions can be skipped using a decorator:

	from pytrace import notrace
	
	@notrace
	def hot():
		pass

Trace specific packages:

	$ export TRACE_MODULES=/Users/alon/project

You can specify a colon `:` separated list of folders as well.

Architecture
------------

pytrace can be broken down to three parts:

1. a trace generator - translates function calls to binary trace records saved in memory using *protocol buffers* (http://code.google.com/p/protobuf-c/).
2. a trace dumper - runs in a separate thread/process, collects traces from memory and dumps them to a sqlite database. 
3. a trace reader - reads traces from the database.

The separation of trace generation and dumping has several advantages:

1. **Python utilizes two cores!** - the dumper thread does not touch python objects, only trace records that are saved as binary strings. therefore, it doesn't acquire the global interpreter lock.
2. The dumper can run in a separate process and aggregate traces from several processes - By using shared memory the trace data can be shared between a generator process and a dumper process.

TODO
----

1. Extract to configuration: 
   1. size of shared memory.
   2. db path.
   3. max traces.
2. Add an option to ignore modules under site-packages.
3. Explicit tracing (logging).
4. Sort the arguments.
5. Scroll horizontally.
6. Ignore traces created by the library.
7. Filter query autocompletion using tab/arrows.
8. Print '...' on overflown strings.
9. Document multiple process support. (one db, two processes)
