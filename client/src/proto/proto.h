#if !defined(PROTO_H)
#define PROTO_H

#include "../net/net.h"
#include <stdint.h>

typedef size_t proto_id;
typedef size_t proto_time;

#define LIST_FOREACH(list, var) for ( \
	typeof(list) var = list; var != NULL; var = var->next)

#define LIST_FREE(list, cur) for ( \
	typeof(list) cur = list, cur##__0 = NULL; \
	cur != NULL; \
	free(cur##__0), cur##__0 = cur, cur = cur->next)

#define LIST_FIND(list, id_, idx, var0, var) { \
	idx = 0; var0 = var = NULL; \
	LIST_FOREACH(list, val) { \
		if (val->id == id_) var = val; \
		if (val->id >= id_) break; \
		else { idx++; var0 = val; } } }

#define LIST1_PUSH(list, val) { \
	if ((list)->vals == NULL) \
		(list)->vals = (list)->end = val; \
	else { \
		(list)->end->next = val; \
		(list)->end = val; } }

#define LIST1_DELETE(list, val0, val) { \
	if (val == (list)->vals) \
		(list)->vals = (list)->vals->next; \
	if (val == (list)->end) \
		(list)->end = val0; \
	if (val0 != NULL) val0->next = val->next; \
	free(val); }

typedef struct proto_ent_t {
	struct proto_ent_t* next;
	char* name; proto_id id;
} proto_ent_t;

typedef struct {
	proto_ent_t* vals;
	proto_ent_t* end;
} proto_ent1_t;

proto_ent_t* proto_ent_new(char* name, proto_id id);
void proto_ent_free(proto_ent_t* ents);

typedef struct proto_msg_t {
	struct proto_msg_t* next;
	char *msg, *uname; proto_time time;
} proto_msg_t;

typedef struct {
	proto_msg_t* vals;
	proto_msg_t* end;
} proto_msg1_t;

proto_msg_t* proto_msg_new
(char* msg_, char* uname, proto_time time);
void proto_msg_free(proto_msg_t* msgs);

typedef struct proto_ids {
	struct proto_ids* next; proto_id id;
} proto_id_t;

typedef struct { proto_id_t *vals, *end; } proto_id1_t;

proto_id_t* proto_id_new(proto_id id);
void proto_id_free(proto_id_t* ids);

json_t* proto_get_users();
json_t* proto_get_chats();

json_t* proto_chat_connect(proto_id chat);
json_t* proto_chat_disconnect();

// this consumes the `users` arg
json_t* proto_chat_new(const char* name, proto_id_t* users);

// this consumes the `users` arg
json_t* proto_chat_invite(proto_id chat, proto_id_t* users);
json_t* proto_chat_leave(proto_id chat);

json_t* proto_chat_get_users();
json_t* proto_chat_get_msgs();
json_t* proto_chat_send(const char* msg);

json_t* proto_delete_account();

typedef enum {
	PROTO_RES_ID = 7,

	PROTO_RES_USERS = 1,
	PROTO_RES_USER_NEW = 0,
	PROTO_RES_USER_DELETE = 10,

	PROTO_RES_CHATS = 2,
	PROTO_RES_CHAT_NEW = 3,

	PROTO_RES_CHAT_USERS = 6,
	PROTO_RES_CHAT_USER_JOIN = 8,
	PROTO_RES_CHAT_USER_LEAVE = 9,

	PROTO_RES_MSGS = 4,
	PROTO_RES_MSG_NEW = 5,
} proto_res_kind;

typedef struct {
	proto_res_kind kind;
	union {
		proto_ent_t* ent;
		proto_msg_t* msg;
		proto_id_t* ids; } val;
} proto_res_t;

// this consumes the `json` arg
proto_res_t* proto_res_parse(json_t* json);
void proto_res_free(proto_res_t* res, bool move);

#endif
