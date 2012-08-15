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
           ('options', '', '', '', '#9ff', ''),
           
           ('key', '', '', '', '#f99', ''),
           ('action', '', '', '', '#9ff', '')]

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
        self.body.contents.refresh_length()
        self.set_focus(len(self.body.contents))

    def quit(self):
        raise urwid.ExitMainLoop()
        
    KEY_MAP = {'f': 'page down',
               'b': 'page up'}

    FUNC_MAP = {'G': end,
                'p': start,
                'g': start,
                'q': quit}
    
    def keypress(self, size, key):
        if key in self.FUNC_MAP:
            return self.FUNC_MAP[key](self)
        for box in self._edit_boxes:
            if key in box.FUNC_MAP:
                return box.FUNC_MAP[key](box)
        return super(LessLikeListBox, self).keypress(size, self.KEY_MAP.get(key, key))

    def get_keys_and_actions(self):
        keys_and_actions = self.KEY_MAP.items()
        for func_map in [self.FUNC_MAP] + [box.FUNC_MAP for box in self._edit_boxes]:
            for key, func in func_map.iteritems():
                keys_and_actions.append((key, func.__name__))
        return keys_and_actions

    def get_keys_and_actions_text(self):
        res = []
        for key, action in self.get_keys_and_actions():
            res.extend([('action', action.replace('_', ' ')), ' : ', ('key', key), ' '])
        return urwid.Text(res, wrap='clip')

def main():
    content = TraceWalker()
    listbox = LessLikeListBox(content)
    top = urwid.Frame(listbox, listbox.get_keys_and_actions_text())
    listbox.set_frame(top)
    screen = urwid.raw_display.Screen()
    screen.set_terminal_properties(colors=256)
    loop = urwid.MainLoop(top, palette, screen=screen)
    loop.run()

def debug_main():
    try:
        main()
    except:
        import pdb, sys
        pdb.post_mortem(sys.exc_info()[2])
        raise