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
    
def search_in_range(search_range, coming_from):
    def callback(string):
        if string is None:
            top.set_footer(None)
        else:
            try:
                for i in search_range:
                    for widget in content[i].widget_list:
                        if string in widget.text:
                            listbox.set_focus(i, coming_from)
                            top.set_footer(None)
                            top.set_focus("body")
                            return
            except KeyboardInterrupt:
                top.set_footer(urwid.Text("Search interrupted!"))
            else:
                top.set_footer(urwid.Text("Pattern not found"))
        top.set_focus("body")
    return callback

class LessLikeListBox(urwid.ListBox):

    def __init__(self, *a, **k):
        super(LessLikeListBox, self).__init__(*a, **k)
        self._completions = set([""])
    
    def start(self):
        self.set_focus(0)

    def end(self):
        content.refresh_length()
        self.set_focus(len(content))
        
    def search_front(self):
        e = LineEdit(caption="/", completions=self._completions)
        current = self.get_focus()[1]
        urwid.connect_signal(e, 'done', search_in_range(search_range=xrange(current, len(content)),
                                                        coming_from="below"))
        top.set_footer(e)
        top.set_focus("footer")

    def search_back(self):
        e = LineEdit(caption="?", completions=self._completions)
        current = self.get_focus()[1]
        urwid.connect_signal(e, 'done', search_in_range(search_range=xrange(current, 0, -1),
                                                        coming_from="below"))
        top.set_footer(e)
        top.set_focus("footer")
        
    MAP = {'f': 'page down',
           'b': 'page up'}

    FUNC_MAP = {'G': end,
                'p': start,
                'g': start,
                '?': search_back,
                '/': search_front}

    def keypress(self, size, key):
        if key in self.FUNC_MAP:
            return self.FUNC_MAP[key](self)
        return super(LessLikeListBox, self).keypress(size, self.MAP.get(key, key))
        
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