#include "proto.h"

proto_ent_t* proto_ent_new(char* name, proto_id id) {
	proto_ent_t* ent = calloc(1, sizeof(proto_ent_t));
	ent->name = name; ent->id = id; return ent; }

void proto_ent_free(proto_ent_t* ents) {
	LIST_FREE(ents, ent) free(ent->name); }

proto_msg_t* proto_msg_new
(char* msg, char* uname, proto_time time) {
	proto_msg_t* msg1 = calloc(1, sizeof(proto_msg_t));
	msg1->msg = msg; msg1->uname = uname;
	msg1->time = time; return msg1; }

void proto_msg_free(proto_msg_t* msgs) {
	LIST_FREE(msgs, msg) { free(msg->msg); free(msg->uname); } }

proto_id_t* proto_id_new(proto_id id) {
	proto_id_t* id1 = calloc(1, sizeof(proto_id_t));
	id1->id = id; return id1; }

void proto_id_free(proto_id_t* ids) { LIST_FREE(ids, _) {} }

void proto_res_free(proto_res_t* res, bool move) {
	if (!move) switch (res->kind) {
		case PROTO_RES_USERS:
		case PROTO_RES_USER_NEW:
		case PROTO_RES_CHATS:
		case PROTO_RES_CHAT_USERS:
		case PROTO_RES_CHAT_NEW:
			proto_ent_free(res->val.ent); break;

		case PROTO_RES_MSGS:
		case PROTO_RES_MSG_NEW:
			proto_msg_free(res->val.msg); break;
		
		case PROTO_RES_ID:
		case PROTO_RES_USER_DELETE:
		case PROTO_RES_CHAT_USER_JOIN:
		case PROTO_RES_CHAT_USER_LEAVE:
			proto_id_free(res->val.ids.ids); break; }

	free(res); }
