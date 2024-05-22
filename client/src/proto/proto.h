#if !defined(PROTO_H)
#define PROTO_H

#include "../net/net.h"
#include <stdint.h>

#define PROTO_FOREACH(list, var) for ( \
	typeof(list) var = list; var != NULL; var = var->next)

#define PROTO_FREE_LIST(list, cur) for ( \
	typeof(list) cur = list, cur##__0 = NULL; \
	cur != NULL; \
	free(cur##__0), cur##__0 = cur, cur = cur->next)

typedef size_t proto_id;
typedef size_t proto_time;

// req.c

typedef struct proto_ids {
	struct proto_ids* next; proto_id id;
} proto_ids;

proto_ids* proto_ids_new(proto_id id);
void proto_ids_free(proto_ids* ids);

json_t* proto_get_users();
json_t* proto_get_chats();

json_t* proto_chat_connect(proto_id chat);
json_t* proto_chat_disconnect();

// this consumes the `users` arg
json_t* proto_chat_new(const char* name, proto_ids* users);

// this consumes the `users` arg
json_t* proto_chat_invite(proto_id chat, proto_ids* users);
json_t* proto_chat_leave(proto_id chat);

json_t* proto_chat_get_users();
json_t* proto_chat_get_msgs();
json_t* proto_chat_send(const char* msg);

json_t* proto_delete_account();

// res.c

typedef struct proto_ent_t {
	struct proto_ent_t* next;
	char* name; proto_id id;
} proto_ent_t;

void proto_ent_free(proto_ent_t* ents);

typedef struct proto_msg_t {
	struct proto_msg_t* next;
	char *msg, *uname; proto_time time;
} proto_msg_t;

void proto_msg_free(proto_msg_t* msgs);

typedef enum {
	PROTO_RES_USERS = 1,
	PROTO_RES_NEW_USER = 0,

	PROTO_RES_CHATS = 2,
	PROTO_RES_CHAT_USERS = 6,
	PROTO_RES_NEW_CHAT = 3,

	PROTO_RES_MSGS = 4,
	PROTO_RES_NEW_MSG = 5,
} proto_res_kind;

typedef struct {
	proto_res_kind kind;
	union {
		proto_ent_t* ent;
		proto_msg_t* msg; } val;
} proto_res_t;

// this consumes the `json` arg
proto_res_t* proto_res_parse(json_t* json);
void proto_res_free(proto_res_t* res);

#endif
