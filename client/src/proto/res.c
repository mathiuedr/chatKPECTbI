#include <stdlib.h>
#include "proto.h"

#define PARSE_ARRAY(json, objs, fn, ...) { \
	json_t* json##__0; \
	cJSON_ArrayForEach(json##__0, json) { \
		typeof(objs.end) obj = fn( \
			__VA_ARGS__ __VA_OPT__(,) json##__0); \
		LIST1_PUSH(&objs, obj); } }

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
	if (cJSON_IsArray(json)) {
		proto_msg1_t msgs = {};
		PARSE_ARRAY(json, msgs, proto_msg_parse0);
		return msgs.vals; }
	else return proto_msg_parse0(json); }

void proto_msg_free(proto_msg_t* msgs) {
	LIST_FREE(msgs, msg) {
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
	if (cJSON_IsArray(json)) {
		proto_ent1_t ents = {};
		PARSE_ARRAY(json, ents, proto_ent_parse0, user);
		return ents.vals; }
	else return proto_ent_parse0(user, json); }

void proto_ent_free(proto_ent_t* ents) {
	LIST_FREE(ents, ent) free(ent->name); }

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

void proto_res_free(proto_res_t* res, bool move) {
	switch (res->kind) {
		case PROTO_RES_USERS:
		case PROTO_RES_NEW_USER:
		case PROTO_RES_CHATS:
		case PROTO_RES_CHAT_USERS:
		case PROTO_RES_NEW_CHAT:
			if (!move) proto_ent_free(res->val.ent);
			break;

		case PROTO_RES_MSGS:
		case PROTO_RES_NEW_MSG:
			if (!move) proto_msg_free(res->val.msg);
			break; }

	free(res); }
