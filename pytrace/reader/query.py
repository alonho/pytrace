import six
import ast
import time
import operator
from . import tables
from .trace_walker import TIME_FORMAT
from sqlalchemy import or_, and_, not_

class ClassCollector(type):
    classes = []
    def __new__(cls, name, bases, dct):
        new = super(ClassCollector, cls).__new__(cls, name, bases, dct)
        if not dct.get('ABSTRACT', False) and name != 'NewBase':
            cls.classes.append(new)
        return new
        
class Operand(six.with_metaclass(ClassCollector)):
    ABSTRACT = True

    def __init__(self, session):
        self.session = session
    def normalize(self, ast_node):
        return ast_node
    def get_options(self, filter=None):
        col = self.get_column()
        q = self.session.query(col.class_)
        if filter is not None:
            q = q.filter(filter)
        return [getattr(i, col.key) for i in q]
    def get_handlers_docs(self):
        for func in dir(self):
            if func.startswith('handle_'):
                yield getattr(self, func).__doc__
    get_supported_operators = get_handlers_docs
    def handle(self, operator, operand):
        operator_name = operator.__class__.__name__
        try:
            handler = getattr(self, 'handle_' + operator_name)
        except AttributeError:
            raise InvalidOperator("{} doesn't support {}".format(self.get_symbol(), operator_name),
                                  col_offset=operand.col_offset,
                                  options=list(self.get_supported_operators()))
        return handler(self.normalize(operand))
        
    def get_symbol(self):
        raise NotImplementedError()
    def get_column(self):
        raise NotImplementedError()

class StringOperand(Operand):
    ABSTRACT = True

    def normalize(self, ast_node):
        if not isinstance(ast_node, ast.Str):
            raise InvalidTypeError('{} can only compare with a string, not {}'.format(self.get_symbol(), ast_node.__class__.__name__),
                                   col_offset=ast_node.col_offset,
                                   options=self.get_options())
        return ast_node
    
    def handle_Eq(self, string_node):
        '''=='''
        options = self.get_options()
        q = self.get_column().like(string_node.s)
        if not self.get_options(filter=q):
            prefix_options = self.get_options(filter=self.get_column().like(string_node.s + '%'))
            if prefix_options:
                options = prefix_options
            raise ValueNotFoundError("{} not found: {}".format(self.get_symbol(), string_node.s),
                                     col_offset=string_node.col_offset,
                                     options=options)
        return q

def create_int_handler(python_op, symbol):
    def handler(self, num):
        if not isinstance(num, ast.Num):
            raise InvalidTypeError('Expected a number, got: {}'.format(num.__class__.__name__),
                                   col_offset=num.col_offset)
        return python_op(self.get_column(), num.n)
    handler.__doc__ = symbol
    return handler
    
class IntegerOperand(Operand):
    ABSTRACT = True

    handle_Eq = create_int_handler(operator.eq, '==')
    handle_Gt = create_int_handler(operator.gt, '>')
    handle_Lt = create_int_handler(operator.lt, '<')
    handle_GtE = create_int_handler(operator.ge, '>=')
    handle_LtE = create_int_handler(operator.le, '<=')
            
class ArgOp(StringOperand):
    def get_symbol(self):
        return 'arg'
    def get_column(self):
        return tables.ArgName.value

class ValueOp(StringOperand):
    def get_symbol(self):
        return 'value'
    def get_column(self):
        return tables.ArgValue.value

class TypeOp(StringOperand):
    def get_symbol(self):
        return 'type'
    def get_column(self):
        return tables.Type.value

class ModuleOp(StringOperand):
    def get_symbol(self):
        return 'module'
    def get_column(self):
        return tables.Module.value

class ModuleOp(StringOperand):
    def get_symbol(self):
        return 'func'
    def get_column(self):
        return tables.Func.name
        
class TidOp(IntegerOperand):
    def get_symbol(self):
        return 'tid'
    def get_column(self):
        return tables.Trace.tid
        
class TimeOp(IntegerOperand):
    def normalize(self, ast_node):
        if not isinstance(ast_node, ast.Str):
            raise InvalidTypeError('{} can only compare with a string, not {}'.format(self.get_symbol(), ast_node.__class__.__name__),
                                   col_offset=ast_node.col_offset)
        try:
            time_tuple = time.strptime(ast_node.s, TIME_FORMAT)
        except ValueError:
            raise InvalidValueError('Invalid time format ({})'.format(TIME_FORMAT),
                                    col_offset=ast_node.col_offset)
        return ast.Num(time.mktime(time_tuple))
    def get_symbol(self):
        return 'time'
    def get_column(self):
        return tables.Trace.time

class ParseError(Exception):
    def __init__(self, message, col_offset, options=[]):
        super(ParseError, self).__init__(message)
        self.col_offset = col_offset
        self.options = options

class InvalidOperand(ParseError): pass
class InvalidOperator(ParseError): pass
class InvalidComparatorNum(ParseError): pass
class InvalidTypeError(ParseError): pass
class InvalidValueError(ParseError): pass
class ValueNotFoundError(ParseError): pass
class IdentifierNotFound(ParseError): pass

class Parser(object):
    
    def __init__(self, session, operands=ClassCollector.classes):
        self._session = session
        self._symbol_to_op = {}
        for op_cls in operands:
            op = op_cls(session)
            self._symbol_to_op[op.get_symbol()] = op

    def get_operand_strings(self):
        return self._symbol_to_op.keys()
            
    def string_to_filter(self, s):
        ex = ast.parse(s, mode='eval')
        return self.handle(ex.body)

    def handle(self, thing):
        thing_name = thing.__class__.__name__
        try:
            handler = getattr(self, 'handle_' + thing_name)
        except AttributeError:
            raise ParseError("Illegal expression ({})".format(thing_name),
                             col_offset=thing.col_offset,
                             options=self.get_operand_strings())
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
        return self._handle_Name(left).handle(compare.ops[0], compare.comparators[0])

    # called explicitly
    def _handle_Name(self, name):
        if name.id not in self._symbol_to_op:
            raise IdentifierNotFound('Identifier not found: {}'.format(name.id),
                                     col_offset=name.col_offset,
                                     options=self._symbol_to_op.keys())
        return self._symbol_to_op[name.id]
            
    def handle_UnaryOp(self, op):
        return self.handle(op.op)(self.handle(op.operand))

    def handle_Not(self, not_node):
        return not_