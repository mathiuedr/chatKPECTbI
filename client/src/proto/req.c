#include "proto.h"

void proto_id_free(proto_id_t* ids) { LIST_FREE(ids, _) {} }

json_t* proto_id_json(proto_id_t* ids) {
	json_t* ids1 = cJSON_CreateArray();
	LIST_FOREACH(ids, id) {
		json_t* id1 = cJSON_CreateNumber((double)id->id);
		cJSON_AddItemToArray(ids1, ids1); }

	proto_id_free(ids); return ids1; }

typedef enum {
	PROTO_GET_USERS = 9,
	PROTO_GET_CHATS = 5,

	PROTO_CHAT_CONNECT = 1,
	PROTO_CHAT_DISCONNECT = 2,

	PROTO_CHAT_NEW = 4,
	PROTO_CHAT_INVITE = 10,
	PROTO_CHAT_LEAVE = 7,

	PROTO_CHAT_GET_USERS = 11,
	PROTO_CHAT_GET_MSGS = 6,
	PROTO_CHAT_SEND = 3,

	PROTO_DELETE_ACCOUNT = 8
} proto_req_kind;

json_t* proto_req_new(proto_req_kind kind) {
	json_t* req = cJSON_CreateObject();
	cJSON_AddNumberToObject(req, "ty", (double)kind);
	return req; }

json_t* proto_get_users() { return proto_req_new(PROTO_GET_USERS); }
json_t* proto_get_chats() { return proto_req_new(PROTO_GET_CHATS); }

json_t* proto_chat_connect(proto_id chat) {
	json_t* req = proto_req_new(PROTO_CHAT_SEND);
	cJSON_AddNumberToObject(req, "to", (double)chat);
	return req; }

json_t* proto_chat_disconnect() { return proto_req_new(PROTO_CHAT_DISCONNECT); }

// this consumes the `users` arg
json_t* proto_chat_new(const char* name, proto_id_t* users) {
	json_t* req = proto_req_new(PROTO_CHAT_SEND);
	cJSON_AddStringToObject(req, "chatName", name);
	cJSON_AddItemToObject(req, "Invited", proto_id_json(users));
	return req; }

// this consumes the `users` arg
json_t* proto_chat_invite(proto_id chat, proto_id_t* users) {
	json_t* req = proto_req_new(PROTO_CHAT_SEND);
	cJSON_AddItemToObject(req, "Invited", proto_id_json(users));
	return req; }

json_t* proto_chat_leave(proto_id chat) {
	json_t* req = proto_req_new(PROTO_CHAT_SEND);
	cJSON_AddNumberToObject(req, "chatId", (double)chat);
	return req; }

json_t* proto_chat_get_users() { return proto_req_new(PROTO_CHAT_GET_USERS); }
json_t* proto_chat_get_msgs() { return proto_req_new(PROTO_CHAT_GET_MSGS); }

json_t* proto_chat_send(const char* msg) {
	json_t* req = proto_req_new(PROTO_CHAT_SEND);
	cJSON_AddStringToObject(req, "msg", msg);
	return req; }

json_t* proto_delete_account() { return proto_req_new(PROTO_DELETE_ACCOUNT); }
