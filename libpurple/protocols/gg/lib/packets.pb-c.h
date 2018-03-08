/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: packets.proto */

#ifndef PROTOBUF_C_packets_2eproto__INCLUDED
#define PROTOBUF_C_packets_2eproto__INCLUDED

#include "protobuf.h"

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1000000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1000002 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _GG110LoginOK GG110LoginOK;
typedef struct _GG110Pong GG110Pong;
typedef struct _GG110Ack GG110Ack;
typedef struct _GG105Login GG105Login;
typedef struct _GG110MessageAckLink GG110MessageAckLink;
typedef struct _GG110MessageAck GG110MessageAck;
typedef struct _GG110Event GG110Event;
typedef struct _GG110RecvMessage GG110RecvMessage;
typedef struct _GG110SendMessage GG110SendMessage;
typedef struct _GG110Imtoken GG110Imtoken;
typedef struct _GG110ChatInfoUpdate GG110ChatInfoUpdate;
typedef struct _ProtobufKVP ProtobufKVP;
typedef struct _GG110Options GG110Options;
typedef struct _GG110AccessInfo GG110AccessInfo;
typedef struct _GG112TransferInfoUin GG112TransferInfoUin;
typedef struct _GG112TransferInfoFile GG112TransferInfoFile;
typedef struct _GG112TransferInfo GG112TransferInfo;
typedef struct _GG110MagicNotification GG110MagicNotification;


/* --- enums --- */

typedef enum _GG110Ack__Type {
  GG110_ACK__TYPE__MSG = 1,
  GG110_ACK__TYPE__CHAT = 2,
  GG110_ACK__TYPE__CHAT_INFO = 3,
  GG110_ACK__TYPE__MAGIC_NOTIFICATION = 5,
  GG110_ACK__TYPE__MPA = 6,
  GG110_ACK__TYPE__TRANSFER_INFO = 7
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(GG110_ACK__TYPE)
} GG110Ack__Type;
typedef enum _GG110Event__Type {
  GG110_EVENT__TYPE__XML = 0,
  GG110_EVENT__TYPE__JSON = 2
    PROTOBUF_C__FORCE_ENUM_TO_BE_INT_SIZE(GG110_EVENT__TYPE)
} GG110Event__Type;

/* --- messages --- */

struct  _GG110LoginOK
{
  ProtobufCMessage base;
  int32_t dummy1;
  char *dummyhash;
  uint32_t uin;
  uint32_t server_time;
};
#define GG110_LOGIN_OK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_login_ok__descriptor) \
    , 1, NULL, 0, 0 }


struct  _GG110Pong
{
  ProtobufCMessage base;
  uint32_t server_time;
};
#define GG110_PONG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_pong__descriptor) \
    , 0 }


struct  _GG110Ack
{
  ProtobufCMessage base;
  GG110Ack__Type type;
  uint32_t seq;
  uint32_t dummy1;
};
#define GG110_ACK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_ack__descriptor) \
    , 0, 0, 1u }


struct  _GG105Login
{
  ProtobufCMessage base;
  char *lang;
  ProtobufCBinaryData uin;
  ProtobufCBinaryData hash;
  char *client;
  uint32_t initial_status;
  char *initial_descr;
  char *supported_features;
  int32_t dummy1;
  uint32_t dummy2;
  uint32_t dummy3;
  ProtobufCBinaryData dummy4;
  int32_t dummy5;
  int32_t dummy6;
  protobuf_c_boolean has_dummy7;
  uint32_t dummy7;
  protobuf_c_boolean has_dummy8;
  int32_t dummy8;
  protobuf_c_boolean has_dummy10;
  uint32_t dummy10;
};
extern char gg105_login__initial_descr__default_value[];
#define GG105_LOGIN__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg105_login__descriptor) \
    , NULL, {0,NULL}, {0,NULL}, NULL, 8227u, gg105_login__initial_descr__default_value, NULL, 4, 65994615u, 198164u, {0,NULL}, 255, 100, 0,127u, 0,0, 0,0u }


struct  _GG110MessageAckLink
{
  ProtobufCMessage base;
  uint64_t id;
  char *url;
};
#define GG110_MESSAGE_ACK_LINK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_message_ack_link__descriptor) \
    , 0, NULL }


struct  _GG110MessageAck
{
  ProtobufCMessage base;
  uint32_t msg_type;
  uint32_t seq;
  uint32_t time;
  protobuf_c_boolean has_msg_id;
  uint64_t msg_id;
  protobuf_c_boolean has_conv_id;
  uint64_t conv_id;
  size_t n_links;
  GG110MessageAckLink **links;
  uint32_t dummy1;
};
#define GG110_MESSAGE_ACK__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_message_ack__descriptor) \
    , 0, 0, 0, 0,0, 0,0, 0,NULL, 0u }


struct  _GG110Event
{
  ProtobufCMessage base;
  GG110Event__Type type;
  uint32_t seq;
  char *data;
  char *subtype;
  protobuf_c_boolean has_id;
  uint64_t id;
};
#define GG110_EVENT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_event__descriptor) \
    , 0, 0, NULL, NULL, 0,0 }


struct  _GG110RecvMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_sender;
  ProtobufCBinaryData sender;
  uint32_t flags;
  uint32_t seq;
  uint32_t time;
  char *msg_plain;
  char *msg_xhtml;
  protobuf_c_boolean has_data;
  ProtobufCBinaryData data;
  protobuf_c_boolean has_msg_id;
  uint64_t msg_id;
  protobuf_c_boolean has_chat_id;
  uint64_t chat_id;
  protobuf_c_boolean has_conv_id;
  uint64_t conv_id;
};
extern char gg110_recv_message__msg_plain__default_value[];
#define GG110_RECV_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_recv_message__descriptor) \
    , 0,{0,NULL}, 0, 0, 0, gg110_recv_message__msg_plain__default_value, NULL, 0,{0,NULL}, 0,0, 0,0, 0,0 }


struct  _GG110SendMessage
{
  ProtobufCMessage base;
  protobuf_c_boolean has_recipient;
  ProtobufCBinaryData recipient;
  uint32_t dummy1;
  uint32_t seq;
  char *msg_plain;
  char *msg_xhtml;
  char *dummy3;
  protobuf_c_boolean has_chat_id;
  uint64_t chat_id;
};
extern char gg110_send_message__dummy3__default_value[];
#define GG110_SEND_MESSAGE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_send_message__descriptor) \
    , 0,{0,NULL}, 8u, 0, NULL, NULL, gg110_send_message__dummy3__default_value, 0,0 }


struct  _GG110Imtoken
{
  ProtobufCMessage base;
  char *imtoken;
};
#define GG110_IMTOKEN__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_imtoken__descriptor) \
    , NULL }


struct  _GG110ChatInfoUpdate
{
  ProtobufCMessage base;
  ProtobufCBinaryData participant;
  ProtobufCBinaryData inviter;
  uint32_t update_type;
  uint32_t time;
  uint32_t dummy1;
  uint32_t version;
  uint32_t dummy2;
  uint64_t msg_id;
  uint64_t chat_id;
  uint64_t conv_id;
};
#define GG110_CHAT_INFO_UPDATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_chat_info_update__descriptor) \
    , {0,NULL}, {0,NULL}, 0, 0, 0, 0, 0, 0, 0, 0 }


struct  _ProtobufKVP
{
  ProtobufCMessage base;
  char *key;
  char *value;
};
#define PROTOBUF_KVP__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&protobuf_kvp__descriptor) \
    , NULL, NULL }


struct  _GG110Options
{
  ProtobufCMessage base;
  size_t n_options;
  ProtobufKVP **options;
  uint32_t dummy1;
};
#define GG110_OPTIONS__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_options__descriptor) \
    , 0,NULL, 0u }


struct  _GG110AccessInfo
{
  ProtobufCMessage base;
  uint32_t dummy1;
  uint32_t dummy2;
  uint32_t last_message;
  uint32_t last_file_transfer;
  uint32_t last_conference_ch;
};
#define GG110_ACCESS_INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_access_info__descriptor) \
    , 1u, 0, 0, 0, 0 }


struct  _GG112TransferInfoUin
{
  ProtobufCMessage base;
  uint32_t dummy1;
  ProtobufCBinaryData uin;
};
#define GG112_TRANSFER_INFO_UIN__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg112_transfer_info_uin__descriptor) \
    , 1u, {0,NULL} }


struct  _GG112TransferInfoFile
{
  ProtobufCMessage base;
  char *type;
  char *url;
  char *content_type;
  char *filename;
  uint32_t filesize;
  uint64_t msg_id;
};
extern char gg112_transfer_info_file__type__default_value[];
#define GG112_TRANSFER_INFO_FILE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg112_transfer_info_file__descriptor) \
    , gg112_transfer_info_file__type__default_value, NULL, NULL, NULL, 0, 0 }


struct  _GG112TransferInfo
{
  ProtobufCMessage base;
  uint32_t dummy1;
  GG112TransferInfoUin *peer;
  GG112TransferInfoUin *sender;
  uint32_t time;
  size_t n_data;
  ProtobufKVP **data;
  GG112TransferInfoFile *file;
  uint32_t seq;
  uint64_t msg_id;
  uint64_t conv_id;
};
#define GG112_TRANSFER_INFO__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg112_transfer_info__descriptor) \
    , 0, NULL, NULL, 0, 0,NULL, NULL, 0, 0, 0 }


struct  _GG110MagicNotification
{
  ProtobufCMessage base;
  int32_t dummy1;
  int32_t seq;
  int32_t dummy2;
  int32_t dummy3;
  ProtobufCBinaryData uin;
  char *dummy4;
};
extern char gg110_magic_notification__dummy4__default_value[];
#define GG110_MAGIC_NOTIFICATION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&gg110_magic_notification__descriptor) \
    , 2, 0, 1, 1, {0,NULL}, gg110_magic_notification__dummy4__default_value }


/* GG110LoginOK methods */
void   gg110_login_ok__init
                     (GG110LoginOK         *message);
size_t gg110_login_ok__get_packed_size
                     (const GG110LoginOK   *message);
size_t gg110_login_ok__pack
                     (const GG110LoginOK   *message,
                      uint8_t             *out);
size_t gg110_login_ok__pack_to_buffer
                     (const GG110LoginOK   *message,
                      ProtobufCBuffer     *buffer);
GG110LoginOK *
       gg110_login_ok__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_login_ok__free_unpacked
                     (GG110LoginOK *message,
                      ProtobufCAllocator *allocator);
/* GG110Pong methods */
void   gg110_pong__init
                     (GG110Pong         *message);
size_t gg110_pong__get_packed_size
                     (const GG110Pong   *message);
size_t gg110_pong__pack
                     (const GG110Pong   *message,
                      uint8_t             *out);
size_t gg110_pong__pack_to_buffer
                     (const GG110Pong   *message,
                      ProtobufCBuffer     *buffer);
GG110Pong *
       gg110_pong__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_pong__free_unpacked
                     (GG110Pong *message,
                      ProtobufCAllocator *allocator);
/* GG110Ack methods */
void   gg110_ack__init
                     (GG110Ack         *message);
size_t gg110_ack__get_packed_size
                     (const GG110Ack   *message);
size_t gg110_ack__pack
                     (const GG110Ack   *message,
                      uint8_t             *out);
size_t gg110_ack__pack_to_buffer
                     (const GG110Ack   *message,
                      ProtobufCBuffer     *buffer);
GG110Ack *
       gg110_ack__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_ack__free_unpacked
                     (GG110Ack *message,
                      ProtobufCAllocator *allocator);
/* GG105Login methods */
void   gg105_login__init
                     (GG105Login         *message);
size_t gg105_login__get_packed_size
                     (const GG105Login   *message);
size_t gg105_login__pack
                     (const GG105Login   *message,
                      uint8_t             *out);
size_t gg105_login__pack_to_buffer
                     (const GG105Login   *message,
                      ProtobufCBuffer     *buffer);
GG105Login *
       gg105_login__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg105_login__free_unpacked
                     (GG105Login *message,
                      ProtobufCAllocator *allocator);
/* GG110MessageAckLink methods */
void   gg110_message_ack_link__init
                     (GG110MessageAckLink         *message);
size_t gg110_message_ack_link__get_packed_size
                     (const GG110MessageAckLink   *message);
size_t gg110_message_ack_link__pack
                     (const GG110MessageAckLink   *message,
                      uint8_t             *out);
size_t gg110_message_ack_link__pack_to_buffer
                     (const GG110MessageAckLink   *message,
                      ProtobufCBuffer     *buffer);
GG110MessageAckLink *
       gg110_message_ack_link__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_message_ack_link__free_unpacked
                     (GG110MessageAckLink *message,
                      ProtobufCAllocator *allocator);
/* GG110MessageAck methods */
void   gg110_message_ack__init
                     (GG110MessageAck         *message);
size_t gg110_message_ack__get_packed_size
                     (const GG110MessageAck   *message);
size_t gg110_message_ack__pack
                     (const GG110MessageAck   *message,
                      uint8_t             *out);
size_t gg110_message_ack__pack_to_buffer
                     (const GG110MessageAck   *message,
                      ProtobufCBuffer     *buffer);
GG110MessageAck *
       gg110_message_ack__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_message_ack__free_unpacked
                     (GG110MessageAck *message,
                      ProtobufCAllocator *allocator);
/* GG110Event methods */
void   gg110_event__init
                     (GG110Event         *message);
size_t gg110_event__get_packed_size
                     (const GG110Event   *message);
size_t gg110_event__pack
                     (const GG110Event   *message,
                      uint8_t             *out);
size_t gg110_event__pack_to_buffer
                     (const GG110Event   *message,
                      ProtobufCBuffer     *buffer);
GG110Event *
       gg110_event__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_event__free_unpacked
                     (GG110Event *message,
                      ProtobufCAllocator *allocator);
/* GG110RecvMessage methods */
void   gg110_recv_message__init
                     (GG110RecvMessage         *message);
size_t gg110_recv_message__get_packed_size
                     (const GG110RecvMessage   *message);
size_t gg110_recv_message__pack
                     (const GG110RecvMessage   *message,
                      uint8_t             *out);
size_t gg110_recv_message__pack_to_buffer
                     (const GG110RecvMessage   *message,
                      ProtobufCBuffer     *buffer);
GG110RecvMessage *
       gg110_recv_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_recv_message__free_unpacked
                     (GG110RecvMessage *message,
                      ProtobufCAllocator *allocator);
/* GG110SendMessage methods */
void   gg110_send_message__init
                     (GG110SendMessage         *message);
size_t gg110_send_message__get_packed_size
                     (const GG110SendMessage   *message);
size_t gg110_send_message__pack
                     (const GG110SendMessage   *message,
                      uint8_t             *out);
size_t gg110_send_message__pack_to_buffer
                     (const GG110SendMessage   *message,
                      ProtobufCBuffer     *buffer);
GG110SendMessage *
       gg110_send_message__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_send_message__free_unpacked
                     (GG110SendMessage *message,
                      ProtobufCAllocator *allocator);
/* GG110Imtoken methods */
void   gg110_imtoken__init
                     (GG110Imtoken         *message);
size_t gg110_imtoken__get_packed_size
                     (const GG110Imtoken   *message);
size_t gg110_imtoken__pack
                     (const GG110Imtoken   *message,
                      uint8_t             *out);
size_t gg110_imtoken__pack_to_buffer
                     (const GG110Imtoken   *message,
                      ProtobufCBuffer     *buffer);
GG110Imtoken *
       gg110_imtoken__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_imtoken__free_unpacked
                     (GG110Imtoken *message,
                      ProtobufCAllocator *allocator);
/* GG110ChatInfoUpdate methods */
void   gg110_chat_info_update__init
                     (GG110ChatInfoUpdate         *message);
size_t gg110_chat_info_update__get_packed_size
                     (const GG110ChatInfoUpdate   *message);
size_t gg110_chat_info_update__pack
                     (const GG110ChatInfoUpdate   *message,
                      uint8_t             *out);
size_t gg110_chat_info_update__pack_to_buffer
                     (const GG110ChatInfoUpdate   *message,
                      ProtobufCBuffer     *buffer);
GG110ChatInfoUpdate *
       gg110_chat_info_update__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_chat_info_update__free_unpacked
                     (GG110ChatInfoUpdate *message,
                      ProtobufCAllocator *allocator);
/* ProtobufKVP methods */
void   protobuf_kvp__init
                     (ProtobufKVP         *message);
size_t protobuf_kvp__get_packed_size
                     (const ProtobufKVP   *message);
size_t protobuf_kvp__pack
                     (const ProtobufKVP   *message,
                      uint8_t             *out);
size_t protobuf_kvp__pack_to_buffer
                     (const ProtobufKVP   *message,
                      ProtobufCBuffer     *buffer);
ProtobufKVP *
       protobuf_kvp__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   protobuf_kvp__free_unpacked
                     (ProtobufKVP *message,
                      ProtobufCAllocator *allocator);
/* GG110Options methods */
void   gg110_options__init
                     (GG110Options         *message);
size_t gg110_options__get_packed_size
                     (const GG110Options   *message);
size_t gg110_options__pack
                     (const GG110Options   *message,
                      uint8_t             *out);
size_t gg110_options__pack_to_buffer
                     (const GG110Options   *message,
                      ProtobufCBuffer     *buffer);
GG110Options *
       gg110_options__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_options__free_unpacked
                     (GG110Options *message,
                      ProtobufCAllocator *allocator);
/* GG110AccessInfo methods */
void   gg110_access_info__init
                     (GG110AccessInfo         *message);
size_t gg110_access_info__get_packed_size
                     (const GG110AccessInfo   *message);
size_t gg110_access_info__pack
                     (const GG110AccessInfo   *message,
                      uint8_t             *out);
size_t gg110_access_info__pack_to_buffer
                     (const GG110AccessInfo   *message,
                      ProtobufCBuffer     *buffer);
GG110AccessInfo *
       gg110_access_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_access_info__free_unpacked
                     (GG110AccessInfo *message,
                      ProtobufCAllocator *allocator);
/* GG112TransferInfoUin methods */
void   gg112_transfer_info_uin__init
                     (GG112TransferInfoUin         *message);
size_t gg112_transfer_info_uin__get_packed_size
                     (const GG112TransferInfoUin   *message);
size_t gg112_transfer_info_uin__pack
                     (const GG112TransferInfoUin   *message,
                      uint8_t             *out);
size_t gg112_transfer_info_uin__pack_to_buffer
                     (const GG112TransferInfoUin   *message,
                      ProtobufCBuffer     *buffer);
GG112TransferInfoUin *
       gg112_transfer_info_uin__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg112_transfer_info_uin__free_unpacked
                     (GG112TransferInfoUin *message,
                      ProtobufCAllocator *allocator);
/* GG112TransferInfoFile methods */
void   gg112_transfer_info_file__init
                     (GG112TransferInfoFile         *message);
size_t gg112_transfer_info_file__get_packed_size
                     (const GG112TransferInfoFile   *message);
size_t gg112_transfer_info_file__pack
                     (const GG112TransferInfoFile   *message,
                      uint8_t             *out);
size_t gg112_transfer_info_file__pack_to_buffer
                     (const GG112TransferInfoFile   *message,
                      ProtobufCBuffer     *buffer);
GG112TransferInfoFile *
       gg112_transfer_info_file__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg112_transfer_info_file__free_unpacked
                     (GG112TransferInfoFile *message,
                      ProtobufCAllocator *allocator);
/* GG112TransferInfo methods */
void   gg112_transfer_info__init
                     (GG112TransferInfo         *message);
size_t gg112_transfer_info__get_packed_size
                     (const GG112TransferInfo   *message);
size_t gg112_transfer_info__pack
                     (const GG112TransferInfo   *message,
                      uint8_t             *out);
size_t gg112_transfer_info__pack_to_buffer
                     (const GG112TransferInfo   *message,
                      ProtobufCBuffer     *buffer);
GG112TransferInfo *
       gg112_transfer_info__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg112_transfer_info__free_unpacked
                     (GG112TransferInfo *message,
                      ProtobufCAllocator *allocator);
/* GG110MagicNotification methods */
void   gg110_magic_notification__init
                     (GG110MagicNotification         *message);
size_t gg110_magic_notification__get_packed_size
                     (const GG110MagicNotification   *message);
size_t gg110_magic_notification__pack
                     (const GG110MagicNotification   *message,
                      uint8_t             *out);
size_t gg110_magic_notification__pack_to_buffer
                     (const GG110MagicNotification   *message,
                      ProtobufCBuffer     *buffer);
GG110MagicNotification *
       gg110_magic_notification__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   gg110_magic_notification__free_unpacked
                     (GG110MagicNotification *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*GG110LoginOK_Closure)
                 (const GG110LoginOK *message,
                  void *closure_data);
typedef void (*GG110Pong_Closure)
                 (const GG110Pong *message,
                  void *closure_data);
typedef void (*GG110Ack_Closure)
                 (const GG110Ack *message,
                  void *closure_data);
typedef void (*GG105Login_Closure)
                 (const GG105Login *message,
                  void *closure_data);
typedef void (*GG110MessageAckLink_Closure)
                 (const GG110MessageAckLink *message,
                  void *closure_data);
typedef void (*GG110MessageAck_Closure)
                 (const GG110MessageAck *message,
                  void *closure_data);
typedef void (*GG110Event_Closure)
                 (const GG110Event *message,
                  void *closure_data);
typedef void (*GG110RecvMessage_Closure)
                 (const GG110RecvMessage *message,
                  void *closure_data);
typedef void (*GG110SendMessage_Closure)
                 (const GG110SendMessage *message,
                  void *closure_data);
typedef void (*GG110Imtoken_Closure)
                 (const GG110Imtoken *message,
                  void *closure_data);
typedef void (*GG110ChatInfoUpdate_Closure)
                 (const GG110ChatInfoUpdate *message,
                  void *closure_data);
typedef void (*ProtobufKVP_Closure)
                 (const ProtobufKVP *message,
                  void *closure_data);
typedef void (*GG110Options_Closure)
                 (const GG110Options *message,
                  void *closure_data);
typedef void (*GG110AccessInfo_Closure)
                 (const GG110AccessInfo *message,
                  void *closure_data);
typedef void (*GG112TransferInfoUin_Closure)
                 (const GG112TransferInfoUin *message,
                  void *closure_data);
typedef void (*GG112TransferInfoFile_Closure)
                 (const GG112TransferInfoFile *message,
                  void *closure_data);
typedef void (*GG112TransferInfo_Closure)
                 (const GG112TransferInfo *message,
                  void *closure_data);
typedef void (*GG110MagicNotification_Closure)
                 (const GG110MagicNotification *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor gg110_login_ok__descriptor;
extern const ProtobufCMessageDescriptor gg110_pong__descriptor;
extern const ProtobufCMessageDescriptor gg110_ack__descriptor;
extern const ProtobufCEnumDescriptor    gg110_ack__type__descriptor;
extern const ProtobufCMessageDescriptor gg105_login__descriptor;
extern const ProtobufCMessageDescriptor gg110_message_ack_link__descriptor;
extern const ProtobufCMessageDescriptor gg110_message_ack__descriptor;
extern const ProtobufCMessageDescriptor gg110_event__descriptor;
extern const ProtobufCEnumDescriptor    gg110_event__type__descriptor;
extern const ProtobufCMessageDescriptor gg110_recv_message__descriptor;
extern const ProtobufCMessageDescriptor gg110_send_message__descriptor;
extern const ProtobufCMessageDescriptor gg110_imtoken__descriptor;
extern const ProtobufCMessageDescriptor gg110_chat_info_update__descriptor;
extern const ProtobufCMessageDescriptor protobuf_kvp__descriptor;
extern const ProtobufCMessageDescriptor gg110_options__descriptor;
extern const ProtobufCMessageDescriptor gg110_access_info__descriptor;
extern const ProtobufCMessageDescriptor gg112_transfer_info_uin__descriptor;
extern const ProtobufCMessageDescriptor gg112_transfer_info_file__descriptor;
extern const ProtobufCMessageDescriptor gg112_transfer_info__descriptor;
extern const ProtobufCMessageDescriptor gg110_magic_notification__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_packets_2eproto__INCLUDED */