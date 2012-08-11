import operator
import sqlalchemy
from unittest import TestCase
from pytrace.reader import query, tables

class QueryTest(TestCase):

    @classmethod
    def setUpClass(cls):
        cls.db = tables.DB()
        cls.q = query.Parser(session=cls.db.session)

class StringOperandQueryTest(QueryTest):

    def setUp(self):
        self.db.session.query(tables.Type).delete()
        self.db.session.commit()
        
    def verify_types(self, types):
        for t in types:
            self.db.session.add(tables.Type(value=t))
        self.db.session.commit()
        
    def test_eq(self):
        self.verify_types(['int'])
        e = self.q.string_to_filter("type == 'int'")
        self.assertEqual(e.left, tables.Type.value)
        self.assertEqual(e.right.value, 'int')
        self.assertIsInstance(e.right.type, sqlalchemy.types.String)

    def test_or(self):
        self.verify_types(['int', 'str'])
        e = self.q.string_to_filter("type == 'int' or type == 'str'")
        self.assertEqual(e.operator, operator.or_)
        self.assertEqual(e.clauses[0].right.value, 'int')
        self.assertEqual(e.clauses[1].right.value, 'str')

    def test_not(self):
        self.verify_types(['int', 'intro'])
        e = self.q.string_to_filter("type == 'in%' and not type == 'intro'")
        res = self.db.session.query(tables.Type).filter(e).all()
        self.assertEqual(len(res), 1)
        self.assertEqual(res[0].value, 'int')
        
    def test_eq_negative(self):
        with self.assertRaises(query.InvalidOperand) as context1:
            self.q.string_to_filter("'type' == 'int'")
        with self.assertRaises(query.IdentifierNotFound) as context2:
            self.q.string_to_filter("typo == 'int'")
        with self.assertRaises(query.IdentifierNotFound) as context3:
            self.q.string_to_filter("t == 'int'")
        for context in [context1, context2, context3]:
            self.assertIn('type', context.exception.options)

        with self.assertRaises(query.InvalidOperator) as context:
            self.q.string_to_filter("type < 'int'")
        self.assertIn('==', context.exception.options)

    def test_eq_completion(self):
        self.verify_types(['bla'])
        with self.assertRaises(query.InvalidTypeError) as context1:
            self.q.string_to_filter("type == 1")
        with self.assertRaises(query.ValueNotFoundError) as context2:
            print self.q.string_to_filter("type == 'blue'")
        for context in [context1, context2]:
            self.assertIn('bla', context.exception.options)
        
class IntOperandQueryTest(QueryTest):

    def test_operators(self):
        self.assertIn('=', str(self.q.string_to_filter('tid == 1')))
        for operator in ['<', '>', '<=', '>=']:
            self.assertIn(operator, str(self.q.string_to_filter('tid {} 1'.format(operator))))
    def test_time(self):
        self.assertEqual(1344621150.0, self.q.string_to_filter('time == "2012/08/10 20:52:30"').right.value)