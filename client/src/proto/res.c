#include <stdlib.h>
#include "proto.h"

#define PROTO_PUSH(root, cur, val) { \
	if (root != NULL) { cur->next = val; cur = val; } \
	else root = cur = val; }

#define PROTO_PARSE_ARRAY(json, objs, fn, ...) { \
	json_t* json##__0; \
	typeof(objs) objs##__0 = NULL; \
	cJSON_ArrayForEach(json##__0, json) { \
		typeof(objs) obj = fn( \
			__VA_ARGS__ __VA_OPT__(,) json##__0); \
		PROTO_PUSH(objs, objs##__0, obj); } }

char* proto_string_extract(json_t* json, const char* key) {
	json_t* tgt = cJSON_DetachItemFromObject(json, key);
	if (tgt == NULL) return NULL;
	
	tgt->type |= cJSON_IsReference;
	char* str = cJSON_GetStringValue(tgt);
	cJSON_Delete(tgt); return str; }

proto_msg_t* proto_msg_parse0(json_t* json) {
	proto_msg_t* msg = calloc(1, sizeof(proto_msg_t));

	msg->msg = proto_string_extract(json, "text");
	msg->uname = proto_string_extract(json, "user_name");
	msg->time = cJSON_GetNumberValue(
		cJSON_GetObjectItem(json, "date"));

	return msg; }

proto_msg_t* proto_msg_parse(json_t* json) {
	proto_msg_t* msgs = NULL;
	if (cJSON_IsArray(json))
		PROTO_PARSE_ARRAY(json, msgs, proto_msg_parse0)
	else msgs = proto_msg_parse0(json);
	return msgs; }

void proto_msg_free(proto_msg_t* msgs) {
	PROTO_FREE_LIST(msgs, msg) {
		free(msg->msg); free(msg->uname); } }

proto_ent_t* proto_ent_parse0(bool user, json_t* json) {
	proto_ent_t* ent = calloc(1, sizeof(proto_ent_t));

	ent->id = (proto_id)cJSON_GetNumberValue(
		cJSON_GetObjectItem(
			json, user ? "user_id" : "chat_id"));

	ent->name = proto_string_extract(
		json, user ? "user_name" : "chat_name");

	return ent; }

proto_ent_t* proto_ent_parse(bool user, json_t* json) {
	proto_ent_t* ents = NULL;
	if (cJSON_IsArray(json))
		PROTO_PARSE_ARRAY(
			json, ents, proto_ent_parse0, user)

	else ents = proto_ent_parse0(user, json);
	return ents; }

void proto_ent_free(proto_ent_t* ents) {
	PROTO_FREE_LIST(ents, ent) free(ent->name); }

// this consumes the `json` arg
proto_res_t* proto_res_parse(json_t* json) {
	proto_res_t* res = calloc(1, sizeof(proto_res_t));
	res->kind = (proto_res_kind)cJSON_GetNumberValue(
		cJSON_GetObjectItem(json, "topic"));

	switch (res->kind) {
		case PROTO_RES_USERS:
		case PROTO_RES_CHAT_USERS:
			res->val.ent = proto_ent_parse(
				true, cJSON_GetObjectItem(json, "users"));
			break;

		case PROTO_RES_CHATS:
			res->val.ent = proto_ent_parse(
				false, cJSON_GetObjectItem(json, "chats"));
			break;

		case PROTO_RES_MSGS:
			res->val.msg = proto_msg_parse(
				cJSON_GetObjectItem(json, "messages"));
			break;

		case PROTO_RES_NEW_USER:
			res->val.ent = proto_ent_parse(true, json); break;
		case PROTO_RES_NEW_CHAT:
			res->val.ent = proto_ent_parse(false, json); break;
		case PROTO_RES_NEW_MSG:
			res->val.msg = proto_msg_parse(json); break; }

	cJSON_Delete(json); return res; }

void proto_res_free(proto_res_t* res) {
	switch (res->kind) {
		case PROTO_RES_USERS:
		case PROTO_RES_NEW_USER:
		case PROTO_RES_CHATS:
		case PROTO_RES_CHAT_USERS:
		case PROTO_RES_NEW_CHAT:
			proto_ent_free(res->val.ent); break;

		case PROTO_RES_MSGS:
		case PROTO_RES_NEW_MSG:
			proto_msg_free(res->val.msg); break; }

	free(res); }
