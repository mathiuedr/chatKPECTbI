#include <stdlib.h>
#include "app.h"

void app_mutex_init(app_mutex_t* mtx) {
	pthread_mutex_init(&mtx->mtx, NULL);
	pthread_cond_init(&mtx->cond, NULL);
	mtx->idx = mtx->idx1 = 0; }

void app_mutex_lock(app_mutex_t* mtx) {
	pthread_mutex_lock(&mtx->mtx);

	size_t idx = mtx->idx1++;
	while (mtx->idx != idx)
		pthread_cond_wait(&mtx->cond, &mtx->mtx);

	pthread_mutex_unlock(&mtx->mtx); }

void app_mutex_unlock(app_mutex_t* mtx) {
	pthread_mutex_lock(&mtx->mtx);
	mtx->idx++; pthread_cond_broadcast(&mtx->cond);
	pthread_mutex_unlock(&mtx->mtx); }

void app_mutex_free(app_mutex_t* mtx) {
	pthread_mutex_destroy(&mtx->mtx);
	pthread_cond_destroy(&mtx->cond); }

void app_set_entries
(app_t* app, bool user, proto_res_t* res) {
	proto_ent1_t* ents = user
		? &app->state.users : &app->state.chats;

	proto_ent_free(ents->vals);
	ents->vals = res->val.ent;
	LIST_FOREACH(ents->vals, ent) ents->end = ent;

	if (!app->connd)
		gui_menu_refresh(app->gui.menu, user);
	else if (user)
		gui_chat_refresh(app->gui.chat);

	proto_res_free(res, true); }

void app_add_entry
(app_t* app, bool user, proto_res_t* res) {
	proto_ent_t* ent = res->val.ent;
	LIST1_PUSH(
		user ? &app->state.users
		     : &app->state.chats, ent);

	if (!app->connd)
		gui_menu_add(app->gui.menu, user, ent);
	else if (user) {
		gui_users_t* child = app->gui.chat->child;
		if (child) gui_users_add(child, ent); }

	proto_res_free(res, true); }

void app_remove_user(app_t* app, proto_id id) {
	size_t _idx; proto_ent_t *ent0, *ent;
	LIST_FIND(app->state.users.vals, id, _idx, ent0, ent);

	if (ent) {
		LIST1_DELETE(&app->state.users, ent0, ent);
		gui_users_t* users = app->connd
			? app->gui.chat->child : app->gui.menu->child;

		if (users) gui_users_remove(users, id); } }

void app_set_users0(app_t* app, proto_res_t* res) {
	if (!app->connd) {
		proto_res_free(res, false); return; }

	proto_id1_t* mask = &app->gui.chat->state.mask;
	LIST_FOREACH(res->val.ent, ent) {
		proto_id_t* id1 = proto_id_new(ent->id);
		LIST1_PUSH(mask, id1); }

	gui_chat_refresh(app->gui.chat);
	proto_res_free(res, false); }

void app_render_msg(app_t* app, proto_res_t* res) {
	if (app->connd)
		gui_chat_add(app->gui.chat, res->val.msg);
	proto_res_free(res, true); }

void app_on_message(void* data) {
	app_msg_t* msg = data;
	app_t* app = msg->app;

	switch (msg->res->kind) {
		case PROTO_RES_ID:
		case PROTO_RES_USER_DELETE:
			app_remove_user(app, msg->res->val.id->id);
			proto_res_free(msg->res, false); break;

		case PROTO_RES_USERS:
			app_set_entries(app, true, msg->res); break;

		case PROTO_RES_USER_NEW:
			app_add_entry(app, true, msg->res); break;

		case PROTO_RES_CHATS:
			app_set_entries(app, false, msg->res); break;

		case PROTO_RES_CHAT_NEW:
			app_add_entry(app, false, msg->res); break;

		case PROTO_RES_CHAT_USERS:
			app_set_users0(app, msg->res); break;

		case PROTO_RES_CHAT_USER_JOIN:
			if (app->connd)
				gui_chat_join(app->gui.chat, msg->res->val.id);
			proto_res_free(msg->res, false); break;

		case PROTO_RES_CHAT_USER_LEAVE:
			if (app->connd)
				gui_chat_leave(app->gui.chat, msg->res->val.ent);
			proto_res_free(msg->res, true); break;

		case PROTO_RES_MSGS:
		case PROTO_RES_MSG_NEW:
			app_render_msg(app, msg->res); break; }
	
	free(msg); }

void* app_listen(void* data) {
	app_t* app = data;
	while (true) {
		app_mutex_lock(&app->mtx);
		json_t* json = net_recv_json(app->sesn);

		if (json != NULL) {
			proto_res_t* res = proto_res_parse(json);
			app_msg_t* msg = calloc(1, sizeof(app_msg_t));
			msg->app = app; msg->res = res;

			uiQueueMain(app_on_message, msg); }

		app_mutex_unlock(&app->mtx);
		pthread_testcancel(); }
	
	return NULL; }
