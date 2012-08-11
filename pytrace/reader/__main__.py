import urwid

from .trace_walker import TraceWalker
from .search import SearchBox, FilterBox

palette = [('time', '', '', '', '#9cf', ''),
           ('tid', '', '', '', '#f99', ''),
           ('module', '', '', '', '#9fc', ''),
           ('func', '', '', '', '#9cf', ''),
           ('name', '', '', '', '#9fc', ''),
           ('type', '', '', '', '#9ff', ''),
           ('value', '', '', '', '#f99', ''),
           
           ('search', '', '', '', '#f99', '#fff'),
           ('error', '', '', '', '#f99', ''),
           ('options', '', '', '', '#9cf', '')]

class LessLikeListBox(urwid.ListBox):

    def __init__(self, *a, **k):
        super(LessLikeListBox, self).__init__(*a, **k)
        self._edit_boxes = [SearchBox(self), FilterBox(self.body.contents)]
        
    def set_frame(self, frame):
        for box in self._edit_boxes:
            box.set_frame(frame)
        
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
        for box in self._edit_boxes:
            if key in box.FUNC_MAP:
                return box.FUNC_MAP[key](box)
        return super(LessLikeListBox, self).keypress(size, self.KEY_MAP.get(key, key))
        
def redraw():
    size = screen.get_cols_rows()
    screen.draw_screen(size, top.render(size))

def unhandled_input(key):
    if key == "q":
        raise urwid.ExitMainLoop()
    return key

try:
    content = TraceWalker()
    listbox = LessLikeListBox(content)
    listbox._invalidate()
    top = urwid.Frame(listbox)
    listbox.set_frame(top)
    screen = urwid.raw_display.Screen()
    screen.set_terminal_properties(colors=256)
    loop = urwid.MainLoop(top, palette, screen=screen, unhandled_input=unhandled_input)
    loop.run()
except:
    import pdb, sys
    pdb.post_mortem(sys.exc_info()[2])
    raise