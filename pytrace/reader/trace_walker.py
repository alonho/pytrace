import urwid
from .tables import DB

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
        return urwid.Text("{} {}".format(trace.id, repr(trace.time)))
        
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