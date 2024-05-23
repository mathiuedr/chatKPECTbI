#include <stdlib.h>
#include "proto.h"

#define PARSE_ARRAY(json, objs, fn, ...) { \
	json_t* json##__0; \
	cJSON_ArrayForEach(json##__0, json) { \
		typeof(objs.vals) obj = fn( \
			__VA_ARGS__ __VA_OPT__(,) json##__0); \
		LIST1_PUSH(&objs, obj); } }

char* proto_string_extract(json_t* json, const char* key) {
	json_t* tgt = cJSON_GetObjectItem(json, key);
	if (tgt == NULL) return NULL;
	
	tgt->type |= cJSON_IsReference;
	char* str = cJSON_GetStringValue(tgt);
	return str; }

proto_msg_t* proto_msg_parse0(json_t* json) {
	char* msg = proto_string_extract(json, "text");
	char* uname = proto_string_extract(json, "user_name");
	const proto_time time =
		(proto_time)cJSON_GetNumberValue(
			cJSON_GetObjectItem(json, "date"));

	return proto_msg_new(msg, uname, time); }

proto_msg_t* proto_msg_parse(json_t* json) {
	if (cJSON_IsArray(json)) {
		proto_msg1_t msgs = {};
		PARSE_ARRAY(json, msgs, proto_msg_parse0);
		return msgs.vals; }
	else return proto_msg_parse0(json); }

proto_ent_t* proto_ent_parse0(bool user, json_t* json) {
	char* name = proto_string_extract(
		json, user ? "user_name" : "chat_name");

	const proto_id id = (proto_id)cJSON_GetNumberValue(
		cJSON_GetObjectItem(
			json, user ? "user_id" : "chat_id"));

	return proto_ent_new(name, id); }

proto_ent_t* proto_ent_parse(bool user, json_t* json) {
	if (cJSON_IsArray(json)) {
		proto_ent1_t ents = {};
		PARSE_ARRAY(json, ents, proto_ent_parse0, user);
		return ents.vals; }
	else return proto_ent_parse0(user, json); }

proto_id_t* proto_id_parse0(json_t* json) {
	return proto_id_new((proto_id)cJSON_GetNumberValue(json)); }

proto_id_t* proto_id_parse(json_t* json) {
	if (cJSON_IsArray(json)) {
		proto_id1_t ids = {};
		PARSE_ARRAY(json, ids, proto_id_parse0);
		return ids.vals; }
	else return proto_id_parse0(json); }

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

		case PROTO_RES_USER_NEW:
			res->val.ent = proto_ent_parse(true, json); break;
		case PROTO_RES_CHAT_NEW:
			res->val.ent = proto_ent_parse(false, json); break;

		case PROTO_RES_MSGS:
			res->val.msg = proto_msg_parse(
				cJSON_GetObjectItem(json, "messages"));
			break;

		case PROTO_RES_MSG_NEW:
			res->val.msg = proto_msg_parse(json); break;

		case PROTO_RES_ID:
		case PROTO_RES_USER_DELETE:
			res->val.id = proto_id_parse(
				cJSON_GetObjectItem(json, "user_id"));
			break;

		case PROTO_RES_CHAT_USER_JOIN:
			res->val.id = proto_id_parse(
				cJSON_GetObjectItem(json, "users"));
			break;

		case PROTO_RES_CHAT_USER_LEAVE:
			res->val.ent = proto_ent_parse(true, json);
			break; }

	cJSON_Delete(json); return res; }
