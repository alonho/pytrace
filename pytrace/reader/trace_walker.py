import time
import urwid
from .tables import DB

def prettify(trace):
    time_str = time.strftime("%Y/%m/%d %H:%M:%S,{:.6f}".format(trace.time - int(trace.time)),
                             time.localtime(trace.time))
    func_prefix = trace.depth * ' ' + ('--> ' if trace.type == 'call' else ' <-- ')
    args = sum([[('name', arg.name.value),
                 ' = ',
                 ('type', arg.type.value),
                 ': ',
                 ('value', arg.value.value),
                 (', ')] for arg in trace.args], [])
    if args:
        args.pop()
        
    return [('time', time_str),
            ' ',
            ('tid', str(trace.tid)),
            ' ', 
            ('module', trace.func.module.value),
            ' ',
            func_prefix,
            ('func', trace.func.name),
            '('] + args + [')']

class TraceWalker(object):

    CACHE_SIZE = 2000 # records
    
    def __init__(self):
        self.db = DB()
        self.focus = 0
        self.start_index = 0
        self.end_index = self.CACHE_SIZE
        self.refresh_length()
        self._fill()
        self.end_index = min(self.CACHE_SIZE, len(self))

    def refresh_length(self):
        self.length = self.db.count()

    def _prepare(self, trace):
        if trace.type == 'overflow':
            return urwid.Text('TRACES LOST - CONSIDER EXCLUDING HOT FUNCTIONS')
        return urwid.Text(prettify(trace))
        
    def _fill(self):
        self.cache = map(self._prepare, self.db.find(self.start_index, self.end_index))

    def _refill(self, i):
        if i == self.end_index:
            self.end_index = min(self.end_index + self.CACHE_SIZE / 2, self.length)
            self.start_index = max(self.end_index - self.CACHE_SIZE, 0)
            self.cache = self.cache[-(self.CACHE_SIZE / 2):] + map(self._prepare, self.db.find(self.start_index, self.end_index))
            
        else:
            self.start_index = max(0, i - (self.CACHE_SIZE / 2))
            self.end_index = min(self.length, i + (self.CACHE_SIZE / 2))
            self._fill()
        
    def __len__(self):
        return self.length
        
    def __getitem__(self, i):
        if not (self.start_index <= i < self.end_index):
            self._refill(i)
        return self.cache[i - self.start_index]