import ast
from .tables import *

class ClassCollector(type):
    symbol_to_op = {}
    def __new__(cls, name, bases, dct):
        op = super(ClassCollector, cls).__new__(cls, name, bases, dct)
        if not dct.get('ABSTRACT', False):
            cls.symbol_to_op[op.SYMBOL] = op
        return op
    
class Operand(object):
    __metaclass__ = ClassCollector
    ABSTRACT = True
    
    SYMBOL = None
    COLUMN = None

class ArgOp(Operand):
    SYMBOL = 'arg'
    COLUMN = ArgName.value

class ValueOp(Operand):
    SYMBOL = 'value'
    COLUMN = ArgValue.value

class TypeOp(Operand):
    SYMBOL = 'type'
    COLUMN = Type.value

class ModuleOp(Operand):
    SYMBOL = 'module'
    COLUMN = Module.value

class TidOp(Operand):
    SYMBOL = 'tid'
    COLUMN = Trace.tid

class TimeOp(Operand):
    SYMBOL = 'value'
    COLUMN = Trace.time

class Parser(object):
    
    def __init__(self, symbol_to_op):
        self._symbol_to_op = symbol_to_op
        
    def string_to_filter(self, s):
        ex = ast.parse(s, mode='eval')
        op = self._symbol_to_op[ex.body.left.id]
        
        

        
string_to_filter = Parser(ClassCollector.symbol_to_op).string_to_filter