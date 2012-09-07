import time
import urwid
from .tables import DB

TIME_FORMAT = "%Y/%m/%d %H:%M:%S"
def prettify(trace):
    time_str = time.strftime(TIME_FORMAT + ",{:.6f}".format(trace.time - int(trace.time)),
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
        self._prepare_cb = prepare_cb
        self._filter = None
        self._db = DB()
        self.refresh_length()
        self._fetch(0, self.CACHE_SIZE)
        self._end_index = min(self.CACHE_SIZE, len(self))

    def iter_raw_text(self):
        for trace in self:
            yield trace.render((2000,)).text[0].strip()
        
    def set_prepare_callback(self, prepare_cb):
        self._prepare_cb = prepare_cb
        self._cache = list(map(self._prepare_cb, self._cache))

    def set_filter(self, filter=None):
        self._filter = filter
        self.refresh_length()
        self._fetch(0, self.CACHE_SIZE)
        
    def refresh_length(self):
        self.length = self._db.count(self._filter)

    def _prepare(self, trace):
        if trace.type == 'overflow':
            return urwid.Columns([urwid.Text('Traces lost. Consider excluding hot modules or functions.')])
        return self._prepare_cb(prettify(trace))
        
    def _fetch(self, start, end):
        self._cache = list(map(self._prepare, self._db.find(start, end, self._filter)))
        self._start_index = start
        self._end_index = end
        
    def __len__(self):
        return self.length
        
    def __getitem__(self, i):
        if not (self._start_index <= i < self._end_index):
            if i == self._end_index:
                end = min(self._end_index + int(self.CACHE_SIZE / 2), self.length)
                start = max(end - self.CACHE_SIZE, 0)
                self._cache = self._cache[-int(self.CACHE_SIZE / 2):] + map(self._prepare, self._db.find(start + int(self.CACHE_SIZE / 2), end, self._filter))
                self._start_index = start
                self._end_index = end
            else:
                self._fetch(max(0, i - int(self.CACHE_SIZE / 2)),
                            min(self.length, i + int(self.CACHE_SIZE / 2)))
        return self._cache[i - self._start_index]
