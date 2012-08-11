import ast
from .tables import *
from sqlalchemy import or_, and_, not_

class ClassCollector(type):
    classes = []
    def __new__(cls, name, bases, dct):
        new = super(ClassCollector, cls).__new__(cls, name, bases, dct)
        if not dct.get('ABSTRACT', False):
            cls.classes.append(new)
        return new
        
class Operand(object):
    __metaclass__ = ClassCollector
    ABSTRACT = True

    def __init__(self, session):
        self.session = session
    def get_symbol(self):
        raise NotImplementedError()
    def get_column(self):
        raise NotImplementedError()
    def get_options(self, filter=None):
        col = self.get_column()
        q = self.session.query(col.class_)
        if filter is not None:
            q = q.filter(filter)
        return [getattr(i, col.key) for i in q]
    def handle(self, operator, operand):
        operator_name = operator.__class__.__name__
        try:
            handler = getattr(self, 'handle_' + operator_name)
        except AttributeError:
            raise ParseError("{} doesn't support {}".format(self.get_symbol(), operator_name),
                             col_offset=thing.col_offset)
        return handler(operand)
        
class StringOperand(Operand):
    ABSTRACT = True
    
    def handle_Eq(self, string):
        options = self.get_options()
        if not isinstance(string, ast.Str):
            raise InvalidTypeError("Expected string, got: {}".format(string),
                                   col_offset=string.col_offset,
                                   options=options)
        q = self.get_column().like(string.s)
        if not self.get_options(filter=q):
            raise ValueNotFoundError("Can't find {}".format(string.s),
                                     col_offset=string.col_offset,
                                     options=options)
        return q
    
class ArgOp(StringOperand):
    def get_symbol(self):
        return 'arg'
    def get_column(self):
        return ArgName.value

class ValueOp(StringOperand):
    def get_symbol(self):
        return 'value'
    def get_column(self):
        return ArgValue.value

class TypeOp(StringOperand):
    def get_symbol(self):
        return 'type'
    def get_column(self):
        return Type.value

class ModuleOp(StringOperand):
    def get_symbol(self):
        return 'module'
    def get_column(self):
        return Module.value

class TidOp(Operand):
    def get_symbol(self):
        return 'tid'
    def get_column(self):
        return Trace.tid
        
class TimeOp(Operand):
    def get_symbol(self):
        return 'time'
    def get_column(self):
        return Trace.time

class ParseError(Exception):
    def __init__(self, message, col_offset, options=[]):
        super(ParseError, self).__init__(message)
        self.col_offset = col_offset
        self.options = options

class InvalidOperand(ParseError): pass
class InvalidComparatorNum(ParseError): pass
class IdentifierNotFound(ParseError): pass
class InvalidTypeError(ParseError): pass
class ValueNotFoundError(ParseError): pass

class Parser(object):
    
    def __init__(self, session, operands=ClassCollector.classes):
        self._session = session
        self._symbol_to_op = {}
        for op_cls in operands:
            op = op_cls(session)
            self._symbol_to_op[op.get_symbol()] = op
        
    def string_to_filter(self, s):
        ex = ast.parse(s, mode='eval')
        return self.handle(ex.body)

    def handle(self, thing):
        thing_name = thing.__class__.__name__
        try:
            handler = getattr(self, 'handle_' + thing_name)
        except AttributeError:
            raise ParseError("Don't know how to parse {}".format(self.__class__.__name__, thing_name),
                             col_offset=thing.col_offset)
        return handler(thing)

    def handle_BoolOp(self, op):
        return self.handle(op.op)(*map(self.handle, op.values))

    def handle_And(self, op):
        return and_

    def handle_Or(self, op):
        return or_

    def handle_Compare(self, compare):
        left = compare.left
        if not isinstance(left, ast.Name):
            raise InvalidOperand('Invalid identifier',
                                 col_offset=left.col_offset,
                                 options=self._symbol_to_op.keys())
        if len(compare.comparators) != 1:
            raise InvalidComparatorNum('Invalid number of comparators: {}', col_offset=compare.comparators[1].col_offset)
        return self.handle(left).handle(compare.ops[0], compare.comparators[0])

    def handle_Name(self, name):
        if name.id not in self._symbol_to_op:
            raise IdentifierNotFound('Identifier not found: {}'.format(name.id),
                                     col_offset=name.col_offset,
                                     options=self._symbol_to_op.keys())
        return self._symbol_to_op[name.id]
            
    def handle_UnaryOp(self, op):
        return not_(self.handle(op.operand))

    def handle_Not(self, value):
        import pdb; pdb.set_trace()

