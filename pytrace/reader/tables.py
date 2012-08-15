from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import relationship, exc, sessionmaker, joinedload
from sqlalchemy import (Table, Column, Integer, String, Enum, 
                        UniqueConstraint, ForeignKey, create_engine)

class DB(object):

    def __init__(self, uri="sqlite:///traces.sqlite", echo=False):
        engine = create_engine(uri, echo=echo)
        Base.metadata.create_all(engine)
        self.session = sessionmaker(bind=engine, autocommit=False)()

    def find(self, start_index=None, end_index=None, filter=None):
        # order_by(Trace.time) might be better for the UI but how will we show overflow than?
        q = self.query(filter)
        if start_index is not None:
            q = q.offset(start_index)
        if end_index is not None:
            if start_index is None:
                start_index = 0
            q = q.limit(end_index - start_index)
        return q

    def query(self, filter=None):
        query = self.session.query(Trace).outerjoin(Func).outerjoin(Module).outerjoin(association_table).outerjoin(Arg).outerjoin(Type).outerjoin(ArgName).outerjoin(ArgValue).options(joinedload('*')).group_by(Trace.id)
        if filter is not None:
            return query.filter(filter)
        return query

    def count(self, filter=None):
        return self.query(filter).group_by(Trace.id).count()
        
    def filter(self, filter):
        return self.query().filter(filter)
    
class Base(object):

    @classmethod
    def get_or_create(cls, session, **kwargs):
        try:
            return session.query(cls).filter_by(**kwargs).one()
        except exc.NoResultFound:
            instance = cls(**kwargs)
            session.add(instance)
            session.flush()
            return instance

    def get_all(cls, session):
        return session
            
Base = declarative_base(cls=Base)

association_table = Table('association',
                          Base.metadata,
                          Column('trace_id', Integer, ForeignKey('traces.id'), index=True),
                          Column('arg_id', Integer, ForeignKey('args.id')))
    
class Trace(Base):
    __tablename__ = "traces"

    id = Column(Integer, primary_key=True)
    type = Column(Enum("call", "return", "exception", "overflow", "log"))
    time = Column(Integer, index=True)
    depth = Column(Integer)
    tid = Column(Integer)
    func_id = Column(Integer, ForeignKey('funcs.id'))
    func = relationship("Func", backref="traces")
    args = relationship("Arg",
                        secondary=association_table,
                        backref="traces")

class Arg(Base):
    __tablename__ = "args"
    __table_args__ = (UniqueConstraint("type_id", "name_id", "value_id"),)
    
    id = Column(Integer, primary_key=True)
    type_id = Column(Integer, ForeignKey("types.id"), nullable=False)
    type = relationship("Type", backref="args")
    name_id = Column(Integer, ForeignKey("arg_names.id"), nullable=False)
    name = relationship("ArgName", backref="args")
    value_id = Column(Integer, ForeignKey("arg_values.id"), nullable=False)
    value = relationship("ArgValue", backref="args")

class ArgName(Base):
    __tablename__ = "arg_names"
    
    id = Column(Integer, primary_key=True)
    value = Column(String, unique=True)
    
class ArgValue(Base):
    __tablename__ = "arg_values"
    
    id = Column(Integer, primary_key=True)
    value = Column(String, unique=True)

class Type(Base):
    __tablename__ = "types"
    
    id = Column(Integer, primary_key=True)
    value = Column(String, unique=True)
    
class Module(Base):
    __tablename__ = "modules"
    
    id = Column(Integer, primary_key=True)
    value = Column(String, unique=True)
    
class Func(Base):
    __tablename__ = "funcs"
    __table_args__ = (UniqueConstraint("module_id", "lineno", "name"),)
    
    id = Column(Integer, primary_key=True)
    module_id = Column(Integer, ForeignKey("modules.id"), nullable=False)
    module = relationship("Module", backref="funcs")
    lineno = Column(Integer)
    name = Column(String)