import urwid

from .trace_walker import TraceWalker
from .search import SearchBox

palette = [('search', '', '', '', '#f99', '#fff'),
           ('time', '', '', '', '#9cf', ''),
           ('tid', '', '', '', '#f99', ''),
           ('module', '', '', '', '#9fc', ''),
           ('func', '', '', '', '#9cf', ''),
           ('name', '', '', '', '#9fc', ''),
           ('type', '', '', '', '#9ff', ''),
           ('value', '', '', '', '#f99', '')]

class LessLikeListBox(urwid.ListBox):

    def __init__(self, *a, **k):
        super(LessLikeListBox, self).__init__(*a, **k)
        self._search_box = SearchBox(self)

    def set_frame(self, frame):
        self._search_box.set_frame(frame)
        
    def start(self):
        self.set_focus(0)

    def end(self):
        content.refresh_length()
        self.set_focus(len(content))        
        
    KEY_MAP = {'f': 'page down',
               'b': 'page up'}

    FUNC_MAP = {'G': end,
                'p': start,
                'g': start}
    
    def keypress(self, size, key):
        if key in self.FUNC_MAP:
            return self.FUNC_MAP[key](self)
        if key in self._search_box.FUNC_MAP:
            return self._search_box.FUNC_MAP[key](self._search_box)
        return super(LessLikeListBox, self).keypress(size, self.KEY_MAP.get(key, key))
        
def redraw():
    size = screen.get_cols_rows()
    screen.draw_screen(size, top.render(size))

content = TraceWalker()
listbox = LessLikeListBox(content)
top = urwid.Frame(listbox)
listbox.set_frame(top)
    
def unhandled_input(key):
    if key == "q":
        raise urwid.ExitMainLoop()
    return key
    
screen = urwid.raw_display.Screen()
screen.set_terminal_properties(colors=256)
loop = urwid.MainLoop(top, palette, screen=screen, unhandled_input=unhandled_input)
try:
    loop.run()
except:
    import pdb, sys
    pdb.post_mortem(sys.exc_info()[2])
    raise