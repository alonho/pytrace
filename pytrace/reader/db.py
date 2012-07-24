from itertools import groupby, imap
import sqlite3
import time

def dictfetch(cur):
    col_names = [column[0] for column in cur.description]
    for line in cur:
        yield dict(zip(col_names, line))

class DB(object):

    def __init__(self, path="db.sqlite"):
        self._conn = sqlite3.connect(path)
        self._cur = self._conn.cursor()
        
    def fetch(self):
        rows = dictfetch(self._cur.execute("select traces.id as trace_id, time, tid, modules.value as module, traces.type as trace_type, depth, funcs.name as func, types.value as arg_type, arg_values.value as arg_value, arg_names.value as arg_name from traces join funcs on (traces.func_id=funcs.id) join modules on (funcs.module_id=modules.id) join association on (traces.id=association.trace_id) join args on (association.arg_id=args.id) join arg_names on (args.name_id=arg_names.id) join arg_values on (args.value_id=arg_values.id) join types on (args.type_id=types.id);"))

        for trace_id, traces in groupby(rows, lambda row: row["trace_id"]):
            final_trace = traces.next()
            final_trace["args"] = {final_trace.pop('arg_name'): (final_trace.pop('arg_type'), final_trace.pop('arg_value') or '')}
            for trace in traces:
                final_trace["args"][trace.pop('arg_name')] = (trace.pop('arg_type'), trace.pop('arg_value') or '')
            yield final_trace

    def fetch_pretty(self):
        return imap(self.prettify, self.fetch())

    def prettify(self, trace):
        time_str = time.strftime("%Y/%m/%d %H:%M:%S,{:.6f}".format(trace['time'] - int(trace['time'])),
                                 time.localtime(trace['time']))
        func_prefix = trace['depth'] * ' ' + ('--> ' if trace['trace_type'] == 'call' else ' <-- ')
        args = sum([[('name', arg_name),
                     ' = ',
                     ('type', arg_type),
                     ': ',
                     ('value', arg_value),
                     (', ')] for arg_name, (arg_type, arg_value) in trace['args'].iteritems()], [])
        args.pop()
        
        return [('time', time_str),
                ' ',
                ('tid', str(trace['tid'])),
                ' ', 
                ('module', trace['module']),
                ' ',
                func_prefix,
                ('func', trace['func']),
                '('] + args + [')']