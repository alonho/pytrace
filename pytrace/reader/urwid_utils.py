import urwid

class LineEdit(urwid.Edit):

    __metaclass__ = urwid.signals.MetaSignals
    signals = ['done']

    def __init__(self, completions=None, *a, **k):
        if completions is None:
            completions = set()
        self._completions = completions
        self._index = 0
        self._screen = urwid.raw_display.Screen()
        super(LineEdit, self).__init__(*a, **k)

    def _get_cursor_position(self):
        cols, rows = self._screen.get_cols_rows()
        return self.get_cursor_coords((cols,))
        
    def _get_options(self):
        if self.edit_text != "":
            col, row = self._get_cursor_position()
            text = self.edit_text[:col - 1]
            return sorted(i for i in self._completions if i.startswith(text))
        else:
            return sorted(self._completions)
        
    def keypress(self, size, key):
        if key in ('up', 'down'):
            if len(self._completions) == 0:
                return
            options = self._get_options()
            if len(options) == 0:
                return
            self.edit_text = options[self._index % len(options)]
            if key == 'up':
                self._index += 1
            else:
                self._index -= 1
        elif key == 'enter':
            self._index = 0
            self._completions.add(self.edit_text)
            urwid.emit_signal(self, 'done', self.edit_text)
        elif key == 'esc':
            self._index = 0
            urwid.emit_signal(self, 'done', None)
        else:
            super(LineEdit, self).keypress(size, key)