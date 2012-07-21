import urwid
import operator
from urwid import raw_display
from db import DB

class Selectable(urwid.FlowWidget):
    def selectable(self):
        return True
    def keypress(self, size, key):
        pass

def get_data():
    return list(DB().fetch_pretty())
    
data = get_data()
palette = [('header', 'white', 'black'),
           ('reveal focus', 'black', 'dark cyan', 'standout'),
           ('time', '', '', '', '#9cf', ''),
           ('tid', '', '', '', '#f99', ''),
           ('module', '', '', '', '#9fc', ''),
           ('func', '', '', '', '#9cf', ''),
           ('name', '', '', '', '#9fc', ''),
           ('type', '', '', '', '#9ff', ''),
           ('value', '', '', '', '#f99', '')]
content = urwid.SimpleListWalker(map(urwid.Text, data))
listbox = urwid.ListBox(content)
show_key = urwid.Text(u"", wrap='clip')
head = urwid.AttrMap(show_key, 'header')
top = urwid.Frame(listbox, head)

def show_all_input(key, raw):
    show_key.set_text(u"Pressed: " + u" ".join([
        unicode(i) for i in key]))
    return key

class ScreenNavigator(object):

    def __init__(self, screen):
        self._screen = screen
        
    def keypress(self, key):
        if key == "G":
            content.set_focus(len(data) - 1)
        elif key == "p":
            content.set_focus(0)
        elif key in ("f", "b"):
            x, y = self._screen.get_cols_rows() # max_row might be faster
            _, index = content.get_focus()
            add_or_sub = dict(f=operator.add, b=operator.sub)
            content.set_focus(add_or_sub[key](index, y / 2))
        elif key == "T":
            new_data = get_data()
            if len(new_data) != len(data):
                for i in new_data[-(len(new_data)-len(data)):]:
                    content.append(urwid.Text([i]))
            content.set_focus(len(new_data) - 1)
        elif key == "q":
            raise urwid.ExitMainLoop()

screen = raw_display.Screen()
screen_nav = ScreenNavigator(screen)
screen.set_terminal_properties(colors=256)
loop = urwid.MainLoop(top, palette, screen=screen, input_filter=show_all_input, unhandled_input=screen_nav.keypress)
loop.run()