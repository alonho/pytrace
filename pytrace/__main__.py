from pytrace import trace_context
from pytrace.reader import main as reader_main
import runpy
import sys

def pop_self_from_argv():
    sys.argv.pop(0)
    
def main():
    pop_self_from_argv()
    if len(sys.argv) == 0:
        reader_main()
    else:
        with trace_context():
            runpy.run_path(sys.argv[0], run_name='__main__')
    
if __name__ == '__main__':
    main()
