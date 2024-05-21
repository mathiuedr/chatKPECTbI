#if !defined(PROTO_H)
#define PROTO_H

#include "../net/net.h"
#include <stdint.h>

#define PROTO_FOREACH(list, var) \
	for ( \
		typeof(list) var = list; \
		var != NULL; var = var->next)

typedef size_t proto_id;
typedef size_t proto_time;

// req.c

// res.c

typedef struct proto_ent_t {
	struct proto_ent_t* next;
	char* name; proto_id id;
} proto_ent_t;

void proto_ent_free(proto_ent_t* ents);

typedef struct proto_msg_t {
	struct proto_msg_t* next;
	char* msg; char* uname;
	proto_time time; } proto_msg_t;

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

proto_res_t* proto_res_parse(json_t* json);
void proto_res_free(proto_res_t* res);

#endif
