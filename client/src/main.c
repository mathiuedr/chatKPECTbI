#include "proto/proto.h"
#include "gui/gui.h"
#include "net/net.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
	net_sesn_t* sesn;
	pthread_mutex_t mtx;
	pthread_t listen;

	bool connd;
	union {
		gui_chats_t* chats;
		/* gui_chat_t* chat */ } gui;
	
	proto_ent1_t users;
	proto_ent1_t chats; } app_t;

typedef struct {
	app_t* app; proto_res_t* res;
} app_msg_t;

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
		pthread_mutex_lock(&app->mtx);

		json_t* json = net_recv_json(app->sesn);
		proto_res_t* res = proto_res_parse(json);

		app_msg_t* msg = calloc(1, sizeof(app_msg_t));
		msg->app = app; msg->res = res;

		pthread_mutex_unlock(&app->mtx);
		uiQueueMain(app_on_message, msg); }
	
	return NULL; }

void app_on_close(void* data) {
	app_t* app = data;
	pthread_cancel(app->listen);
	pthread_mutex_destroy(&app->mtx);
	net_close(app->sesn); }

void app_on_connect(proto_id chat, void* data) {
	app_t* app = data; }

void app_on_new_chat
(char* name, proto_ids_t* ids, void* data) {
	app_t* app = data; gui_free_str(name); }

bool app_on_auth(
	gui_window* wnd, char* uname,
	char* passwd, char* name, void* data)
{
	net_sesn_t* sesn = net_connect(
		"ws://127.0.0.1:1234", uname, passwd, name);

	gui_free_str(uname);
	gui_free_str(passwd);
	if (name != NULL) gui_free_str(name);

	if (sesn == NULL) {
		gui_msg_box(wnd, "chat-client", "auth failed!", MB_ICONERROR);
		return false; }

	app_t* app = calloc(1, sizeof(app_t));
	app->sesn = sesn;
	pthread_mutex_init(&app->mtx, NULL);
	pthread_create(&app->listen, NULL, app_listen, app);

	app->gui.chats = gui_chats(
		(proto_ent_ptr)&app->users.vals,
		(proto_ent_ptr)&app->chats.vals,
		app_on_connect, app_on_new_chat, app);

	gui_window_init(
	    app->gui.chats->wnd, NULL, NULL, false);
	
	return true; }

int main() {
	gui_init(); net_init();

	gui_window_t* wnd = gui_auth(app_on_auth, NULL);
	gui_window_init(wnd, NULL, NULL, true); gui_run();

	gui_cleanup(); net_cleanup(); return 0; }
