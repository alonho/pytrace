from pytrace import trace_context
import runpy
import sys

def remove_main_from_argv():
    sys.argv.remove(__file__)
    
def main():
    remove_main_from_argv()
    with trace_context():
        runpy.run_path(sys.argv[0], run_name='__main__')
    
if __name__ == '__main__':
    main()
