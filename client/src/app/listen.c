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

void app_set_entries(app_t* app, bool user, proto_res_t* res) {
	proto_ent1_t* ents = user ? &app->users : &app->chats;
	proto_ent_free(ents->vals);
	ents->vals = ents->end = res->val.ent;

	if (!app->connd)
		gui_chats_refresh(app->gui.chats, user);

	proto_res_free(res, true); }

void app_add_entry(app_t* app, bool user, proto_res_t* res) {
	proto_ent_t* ent = res->val.ent;
	LIST1_PUSH(user ? &app->users : &app->chats, ent);

	if (!app->connd)
		gui_chats_add_entry(app->gui.chats, user, ent);

	proto_res_free(res, true); }

void app_on_message(void* data) {
	app_msg_t* msg = data;
	app_t* app = msg->app;

	switch (msg->res->kind) {
		case PROTO_RES_USERS:
			app_set_entries(app, true, msg->res); break;

		case PROTO_RES_NEW_USER:
			app_add_entry(app, true, msg->res); break;

		case PROTO_RES_CHATS:
			app_set_entries(app, false, msg->res); break;

		case PROTO_RES_CHAT_USERS: break;
		case PROTO_RES_NEW_CHAT:
			app_add_entry(app, false, msg->res); break;

		case PROTO_RES_MSGS: break;
		case PROTO_RES_NEW_MSG: break; }
	
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
