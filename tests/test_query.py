from unittest import TestCase
from sqlalchemy import types
from pytrace.reader import query, tables

class QueryTest(TestCase):

    def test_eq(self):
        e = query.string_to_filter("type == 'int'")
        self.assertEqual(e.left, tables.Type)
        self.assertEqual(e.right.value, 'int')
        self.assertIsInstance(e.right.type, types.String)