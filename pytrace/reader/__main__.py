import urwid
from .trace_walker import TraceWalker
    
palette = [('header', 'white', 'black'),
           ('reveal focus', 'black', 'dark cyan', 'standout'),
           ('time', '', '', '', '#9cf', ''),
           ('tid', '', '', '', '#f99', ''),
           ('module', '', '', '', '#9fc', ''),
           ('func', '', '', '', '#9cf', ''),
           ('name', '', '', '', '#9fc', ''),
           ('type', '', '', '', '#9ff', ''),
           ('value', '', '', '', '#f99', '')]

class LessLikeListBox(urwid.ListBox):

    MAP = {'f': 'page down',
           'b': 'page up'}
    def keypress(self, size, key):
        if key == "G":
            content.refresh_length()
            self.set_focus(len(content))
        elif key == "p":
            self.set_focus(0)
        return super(LessLikeListBox, self).keypress(size, self.MAP.get(key, key))
        
content = TraceWalker()
listbox = LessLikeListBox(content)
show_key = urwid.Text(u"", wrap='clip')
head = urwid.AttrMap(show_key, 'header')
top = urwid.Frame(listbox, head)

def show_all_input(key, raw):
    show_key.set_text(u"Pressed: " + u" ".join([
        unicode(i) for i in key]))
    return key
    
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