import urwid
from .urwid_utils import LineEdit
from .trace_walker import TraceWalker

palette = [('time', '', '', '', '#9cf', ''),
           ('tid', '', '', '', '#f99', ''),
           ('module', '', '', '', '#9fc', ''),
           ('func', '', '', '', '#9cf', ''),
           ('name', '', '', '', '#9fc', ''),
           ('type', '', '', '', '#9ff', ''),
           ('value', '', '', '', '#f99', '')]
    
class SearchBox(object):

    def __init__(self, listbox):
        self._listbox = listbox
        self._edit = LineEdit(caption=":")
        self._index = None
        self._string = None
        self._direction = None
        urwid.connect_signal(self._edit, 'done', self._search_callback)
                             
    def _get_current(self):
        return self._listbox.body.focus

    def _get_content(self):
        return self._listbox.body.contents

    def _set_search_range(self, start, end):
        self._search_range_step = 1 if start < end else -1
        self._search_range_start = start
        self._search_range_end = end
        
    def search_forward(self):
        self._edit.set_caption(self.SYMBOL_SEARCH_FORWARD)
        self._set_search_range(self._get_current(), len(self._get_content()))
        self._edit.edit_text = ""
        self._direction = "forward"
        top.set_footer(self._edit)
        top.set_focus("footer")
        
    def search_backward(self):
        self._edit.set_caption(self.SYMBOL_SEARCH_BACKWARD)
        self._set_search_range(self._get_current(), 0)
        self._edit.edit_text = ""
        self._direction = "backward"
        top.set_footer(self._edit)
        top.set_focus("footer")
        
    def _search_callback(self, string):
        self._string = string
        self._edit.edit_text = ""
        if string is not None:
            try:
                self._index = self._search_string_in_range(string)
            except KeyboardInterrupt:
                self._edit.set_caption("Interrupted!")
            else:
                if self._index is None:
                    self._edit.set_caption("Pattern not found")
                else:
                    self._edit.set_caption(":")
                    self._listbox.set_focus(self._index, "below")
        else:
            self._edit.set_caption(":")
        top.set_focus("body")

    def _search_string_in_range(self, string):
        for i in xrange(self._search_range_start, self._search_range_end, self._search_range_step):
            for widget in self._get_content()[i].widget_list:
                if string in widget.text:
                    return i
        return None

    def find_next(self):
        if self._direction is None:
            self._edit.set_caption("No pattern defined.")
            return
        if self._direction == "forward":
            self._set_search_range(self._get_current() + 1, len(self._get_content()))
        else:
            assert self._direction == "backward"
            self._set_search_range(self._get_current() - 1, 0)
        self._search_callback(self._string)

    def find_previous(self):
        if self._direction is None:
            self._edit.set_caption("No pattern defined.")
            return
        if self._direction == "forward":
            self._set_search_range(self._get_current() - 1, 0)
        else:
            assert self._direction == "backward"
            self._set_search_range(self._get_current() + 1, len(self._get_content()))
        self._search_callback(self._string)
        
    SYMBOL_SEARCH_BACKWARD = "?"
    SYMBOL_SEARCH_FORWARD = "/"
    FUNC_MAP = {SYMBOL_SEARCH_BACKWARD: search_backward,
                SYMBOL_SEARCH_FORWARD: search_forward,
                'n': find_next,
                'N': find_previous}

class LessLikeListBox(urwid.ListBox):

    def __init__(self, *a, **k):
        super(LessLikeListBox, self).__init__(*a, **k)
        self._search_box = SearchBox(self)
        
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
        
content = TraceWalker()
listbox = LessLikeListBox(content)
top = urwid.Frame(listbox)
    
def unhandled_input(key):
    if key == "q":
        raise urwid.ExitMainLoop()
    return key

def redraw():
    size = screen.get_cols_rows()
    screen.draw_screen(size, top.render(size))
    
screen = urwid.raw_display.Screen()
screen.set_terminal_properties(colors=256)
loop = urwid.MainLoop(top, palette, screen=screen, unhandled_input=unhandled_input)
try:
    loop.run()
except:
    import pdb, sys
    pdb.post_mortem(sys.exc_info()[2])
    raise