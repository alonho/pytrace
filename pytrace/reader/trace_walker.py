import time
import urwid
from .tables import DB

def prettify(trace):
    time_str = time.strftime("%Y/%m/%d %H:%M:%S,{:.6f}".format(trace.time - int(trace.time)),
                             time.localtime(trace.time))
    func_prefix = (trace.depth + 3) * ' ' + ('--> ' if trace.type == 'call' else ' <-- ')
    args = sum([[('name', arg.name.value),
                 ' = ',
                 ('type', arg.type.value),
                 ': ',
                 ('value', arg.value.value),
                 (', ')] for arg in trace.args], [])
    if args:
        args.pop()
    return urwid.Columns([('fixed', 28, urwid.Text(('time', time_str))),
                          ('fixed', 10, urwid.Text(('tid', str(trace.tid)))),
                          ('fixed', 24, urwid.Text(('module', trace.func.module.value), wrap='clip', align='right')),
                          urwid.Text([func_prefix, ('func', trace.func.name), '('] + args + [')'], wrap='clip')],
                         dividechars=1)

class TraceWalker(object):

    CACHE_SIZE = 500 # records
    
    def __init__(self, prepare_cb=lambda x: x):
        self.prepare_cb = prepare_cb
        self.db = DB()
        self.refresh_length()
        self._fetch(0, self.CACHE_SIZE)
        self.end_index = min(self.CACHE_SIZE, len(self))

    def set_prepare_callback(self, prepare_cb):
        self.prepare_cb = prepare_cb
        self.refetch()
        
    def refresh_length(self):
        self.length = self.db.count()

    def _prepare(self, trace):
        if trace.type == 'overflow':
            return urwid.Columns([urwid.Text('Traces lost. Consider excluding hot modules or functions.')])
        return self.prepare_cb(prettify(trace))
        
    def _fetch(self, start, end):
        self.cache = map(self._prepare, self.db.find(start, end))
        self.start_index = start
        self.end_index = end

    def refetch(self):
        self.cache = map(self.prepare_cb, self.cache)
        
    def __len__(self):
        return self.length
        
    def __getitem__(self, i):
        if not (self.start_index <= i < self.end_index):
            if i == self.end_index:
                end = min(self.end_index + self.CACHE_SIZE / 2, self.length)
                start = max(end - self.CACHE_SIZE, 0)
                self.cache = self.cache[-(self.CACHE_SIZE / 2):] + map(self._prepare, self.db.find(start + self.CACHE_SIZE / 2, end))
                self.start_index = start
                self.end_index = end
            else:
                self._fetch(max(0, i - (self.CACHE_SIZE / 2)),
                            min(self.length, i + (self.CACHE_SIZE / 2)))
        return self.cache[i - self.start_index]
