/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: record.proto */

/* Do not generate deprecated warnings for self */
#ifndef PROTOBUF_C__NO_DEPRECATED
#define PROTOBUF_C__NO_DEPRECATED
#endif

#include "record_pb.h"
void   argument__init
                     (Argument         *message)
{
  static Argument init_value = ARGUMENT__INIT;
  *message = init_value;
}
size_t argument__get_packed_size
                     (const Argument *message)
{
  assert(message->base.descriptor == &argument__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t argument__pack
                     (const Argument *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &argument__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t argument__pack_to_buffer
                     (const Argument *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &argument__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Argument *
       argument__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Argument *)
     protobuf_c_message_unpack (&argument__descriptor,
                                allocator, len, data);
}
void   argument__free_unpacked
                     (Argument *message,
                      ProtobufCAllocator *allocator)
{
  assert(message->base.descriptor == &argument__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
void   record__init
                     (Record         *message)
{
  static Record init_value = RECORD__INIT;
  *message = init_value;
}
size_t record__get_packed_size
                     (const Record *message)
{
  assert(message->base.descriptor == &record__descriptor);
  return protobuf_c_message_get_packed_size ((const ProtobufCMessage*)(message));
}
size_t record__pack
                     (const Record *message,
                      uint8_t       *out)
{
  assert(message->base.descriptor == &record__descriptor);
  return protobuf_c_message_pack ((const ProtobufCMessage*)message, out);
}
size_t record__pack_to_buffer
                     (const Record *message,
                      ProtobufCBuffer *buffer)
{
  assert(message->base.descriptor == &record__descriptor);
  return protobuf_c_message_pack_to_buffer ((const ProtobufCMessage*)message, buffer);
}
Record *
       record__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data)
{
  return (Record *)
     protobuf_c_message_unpack (&record__descriptor,
                                allocator, len, data);
}
void   record__free_unpacked
                     (Record *message,
                      ProtobufCAllocator *allocator)
{
  assert(message->base.descriptor == &record__descriptor);
  protobuf_c_message_free_unpacked ((ProtobufCMessage*)message, allocator);
}
static const ProtobufCFieldDescriptor argument__field_descriptors[3] =
{
  {
    "name",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(Argument, name),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "type",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(Argument, type),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "value",
    3,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(Argument, value),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned argument__field_indices_by_name[] = {
  0,   /* field[0] = name */
  1,   /* field[1] = type */
  2,   /* field[2] = value */
};
static const ProtobufCIntRange argument__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 3 }
};
const ProtobufCMessageDescriptor argument__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "Argument",
  "Argument",
  "Argument",
  "",
  sizeof(Argument),
  3,
  argument__field_descriptors,
  argument__field_indices_by_name,
  1,  argument__number_ranges,
  (ProtobufCMessageInit) argument__init,
  NULL,NULL,NULL    /* reserved[123] */
};
const ProtobufCEnumValue record__record_type__enum_values_by_number[5] =
{
  { "CALL", "RECORD__RECORD_TYPE__CALL", 0 },
  { "RETURN", "RECORD__RECORD_TYPE__RETURN", 1 },
  { "EXCEPTION", "RECORD__RECORD_TYPE__EXCEPTION", 2 },
  { "LOG", "RECORD__RECORD_TYPE__LOG", 3 },
  { "OVERFLOW", "RECORD__RECORD_TYPE__OVERFLOW", 4 },
};
static const ProtobufCIntRange record__record_type__value_ranges[] = {
{0, 0},{0, 5}
};
const ProtobufCEnumValueIndex record__record_type__enum_values_by_name[5] =
{
  { "CALL", 0 },
  { "EXCEPTION", 2 },
  { "LOG", 3 },
  { "OVERFLOW", 4 },
  { "RETURN", 1 },
};
const ProtobufCEnumDescriptor record__record_type__descriptor =
{
  PROTOBUF_C__ENUM_DESCRIPTOR_MAGIC,
  "Record.RecordType",
  "RecordType",
  "Record__RecordType",
  "",
  5,
  record__record_type__enum_values_by_number,
  5,
  record__record_type__enum_values_by_name,
  1,
  record__record_type__value_ranges,
  NULL,NULL,NULL,NULL   /* reserved[1234] */
};
static const ProtobufCFieldDescriptor record__field_descriptors[8] =
{
  {
    "type",
    1,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_ENUM,
    0,   /* quantifier_offset */
    offsetof(Record, type),
    &record__record_type__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "time",
    2,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_DOUBLE,
    0,   /* quantifier_offset */
    offsetof(Record, time),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "tid",
    3,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_INT64,
    0,   /* quantifier_offset */
    offsetof(Record, tid),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "depth",
    4,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Record, depth),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "module",
    5,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(Record, module),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "function",
    6,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_BYTES,
    0,   /* quantifier_offset */
    offsetof(Record, function),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "lineno",
    7,
    PROTOBUF_C_LABEL_REQUIRED,
    PROTOBUF_C_TYPE_INT32,
    0,   /* quantifier_offset */
    offsetof(Record, lineno),
    NULL,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
  {
    "arguments",
    8,
    PROTOBUF_C_LABEL_REPEATED,
    PROTOBUF_C_TYPE_MESSAGE,
    offsetof(Record, n_arguments),
    offsetof(Record, arguments),
    &argument__descriptor,
    NULL,
    0,             /* flags */
    0,NULL,NULL    /* reserved1,reserved2, etc */
  },
};
static const unsigned record__field_indices_by_name[] = {
  7,   /* field[7] = arguments */
  3,   /* field[3] = depth */
  5,   /* field[5] = function */
  6,   /* field[6] = lineno */
  4,   /* field[4] = module */
  2,   /* field[2] = tid */
  1,   /* field[1] = time */
  0,   /* field[0] = type */
};
static const ProtobufCIntRange record__number_ranges[1 + 1] =
{
  { 1, 0 },
  { 0, 8 }
};
const ProtobufCMessageDescriptor record__descriptor =
{
  PROTOBUF_C__MESSAGE_DESCRIPTOR_MAGIC,
  "Record",
  "Record",
  "Record",
  "",
  sizeof(Record),
  8,
  record__field_descriptors,
  record__field_indices_by_name,
  1,  record__number_ranges,
  (ProtobufCMessageInit) record__init,
  NULL,NULL,NULL    /* reserved[123] */
};
