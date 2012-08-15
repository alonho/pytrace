
pytrace - a fast python tracer
==============================

pytrace records function calls, arguments and return values.

traces aid debugging, profiling and obviate logging.

Follow @alonhorev on twitter for updates.

Install
-------

pytrace relies on sqlite and a C implementation of protocol buffers.
on debian/ubuntu: `sudo apt-get install libsqlite3-dev libprotobuf-c0-dev`
on mac (sqlite is included): `brew install protobuf-c` or `port install protobuf-c`

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

The reader can filter traces using a python syntax:

1. strings support comparison using the sql 'like' syntax: `module == 'project.%'`.
2. numbers support numeric operators: `time > '2012/08/15 01:23:45'`.

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
