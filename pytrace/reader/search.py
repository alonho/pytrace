import re
import urwid
from .urwid_utils import LineEdit
from .query import Parser, ParseError

def highlight(widget, start, end):
    remove_highlight(widget)
        
    attrib = []
    count = 0
    it = iter(widget.attrib)
    for (attr, index) in it:
        count += index 
        if count < start:
            attrib.append((attr, index))
        else:
            init = start - (count - index)
            middle = end - start
            tail = index - middle - init
            attrib.append((attr, init))
            attrib.append(('search', middle))
            if count > end:
                attrib.append((attr, tail))
            break
    else:
        assert False
            
    for (attr, index) in it:
        count += index
        if count > end:
            attrib.append((attr, min(index, count - end)))
            break

    attrib.extend(it)
                
    widget._orig_attrib = widget.attrib
    widget._attrib = attrib
    widget._invalidate()

def matches(widget):
    return hasattr(widget, '_orig_attrib')
    
def remove_highlight(widget):
    if matches(widget):
        widget._attrib = widget._orig_attrib
        del widget._orig_attrib
        widget._invalidate()

def create_highlighter_for_pattern(pattern):
    p = re.compile(pattern)
    def highlight_on_match(widget):
        for sub_widget in widget.widget_list:
            m = p.search(sub_widget.text)
            if m is not None:
                highlight(sub_widget, m.start(), m.end())
            else:
                remove_highlight(sub_widget)
        return widget
    return highlight_on_match
            
class SearchBox(object):

    def __init__(self, listbox):
        self._listbox = listbox
        self._edit = LineEdit(caption=':')
        self._index = None
        self._pattern = None
        self._direction = None
        self._frame = None
        urwid.connect_signal(self._edit, 'done', self._search_callback)

    def set_frame(self, frame):
        self._frame = frame
        
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
        self._edit.edit_text = ''
        self._direction = 'forward'
        self._frame.set_footer(self._edit)
        self._frame.set_focus('footer')
        
    def search_backward(self):
        self._edit.set_caption(self.SYMBOL_SEARCH_BACKWARD)
        self._set_search_range(self._get_current(), 0)
        self._edit.edit_text = ''
        self._direction = 'backward'
        self._frame.set_footer(self._edit)
        self._frame.set_focus('footer')
        
    def _search_callback(self, pattern):
        self._pattern = pattern
        self._edit.edit_text = ''
        if self._pattern is None:
            self._edit.set_caption(':')
            self._frame.set_footer(None)
            self._frame.set_focus('body')
        else:
            self._set_index(self._search_pattern_in_range())
        
    def _set_index(self, index):
        self._index = index
        if self._index is None:
            self._edit.set_caption('Pattern not found')
            self._edit._invalidate()
        else:
            self._edit.set_caption(':')
            self._listbox.set_focus_valign('top')
            self._listbox.set_focus(self._index, 'below')
        self._frame.set_focus('body')
        
    def _search_pattern_in_range(self):
        highlighter = create_highlighter_for_pattern(self._pattern)
        content = self._get_content()
        content.set_prepare_callback(highlighter)
        return self._find_next_match()

    def _find_next_match(self):
        content = self._get_content()
        for i in xrange(self._search_range_start, self._search_range_end, self._search_range_step):
            if any(map(matches, content[i].widget_list)):
                return i

    def find_next(self):
        if self._direction is None:
            self._frame.set_footer(self._edit)
            self._edit.set_caption('No pattern defined.')
            return
        if self._direction == 'forward':
            self._set_search_range(self._get_current() + 1, len(self._get_content()))
        else:
            assert self._direction == 'backward'
            self._set_search_range(self._get_current() - 1, 0)
        self._set_index(self._find_next_match())

    def find_previous(self):
        if self._direction is None:
            self._frame.set_footer(self._edit)
            self._edit.set_caption('No pattern defined.')
            return
        if self._direction == 'forward':
            self._set_search_range(self._get_current() - 1, 0)
        else:
            assert self._direction == 'backward'
            self._set_search_range(self._get_current() + 1, len(self._get_content()))
        self._set_index(self._find_next_match())
        
    SYMBOL_SEARCH_BACKWARD = '?'
    SYMBOL_SEARCH_FORWARD = '/'
    FUNC_MAP = {SYMBOL_SEARCH_BACKWARD: search_backward,
                SYMBOL_SEARCH_FORWARD: search_forward,
                'n': find_next,
                'N': find_previous}

class FilterBox(object):
    
    def __init__(self, trace_walker):
        self._trace_walker = trace_walker
        self._frame = None
        self._parser = Parser(trace_walker.db.session)
        self._edit = LineEdit(caption='> ')
        self._hint = urwid.Text([('options', 'Example: '), "func == 'fib' and arg == 'return value' and not value == '1'"])
        self._options = urwid.Text('')
        self._set_options(self._parser.get_operand_strings())
        self._pile = urwid.Pile([self._hint, self._options, self._edit])
        urwid.connect_signal(self._edit, 'done', self._filter_callback)

    def _set_options(self, options):
        self._options.set_text([('options', 'Options: '), ' '.join(options)])

    def _set_error(self, error):
        self._hint.set_text(('error', error))
        
    def set_frame(self, frame):
        self._frame = frame

    def _filter_string(self, string):
        try:
            query = self._parser.string_to_filter(string)
        except ParseError as e:
            self._edit.set_edit_pos(e.col_offset)
            self._set_error(str(e))
            self._set_options(e.options)
        except SyntaxError:
            self._set_error("Invalid syntax")
        else:
            self._set_error('')
            self._set_options([])
            self._trace_walker.set_filter(query)
            self._frame.set_footer(None)
            self._frame.set_focus('body')
            
    def _filter_callback(self, query_string):
        if query_string is None:
            self._frame.set_footer(None)
            self._frame.set_focus('body')
        elif query_string == '':
            self._trace_walker.set_filter(None)
            self._frame.set_footer(None)
            self._frame.set_focus('body')
        else:
            self._filter_string(query_string)

    def filter(self):
        self._frame.set_footer(self._pile)
        self._frame.set_focus('footer')

    FUNC_MAP = {'r': filter}