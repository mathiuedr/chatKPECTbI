#include <stdlib.h>
#include "app.h"

void app_start(); void app_menu(void* data);

void app_send(app_t* app, json_t* json) {
	app_mutex_lock(&app->mtx);
	net_send_json(app->sesn, json);
	app_mutex_unlock(&app->mtx); }

void app_on_send(char* msg, void* data) {	
	app_send((app_t*)data, proto_chat_send(msg));
	gui_str_free(msg); }

void app_on_invite(char* _, proto_id_t* ids, void* data) {
	app_t* app = data;
	app_send(app, proto_chat_invite(app->state.chat, ids));
	proto_id_free(ids); }

void app_on_chat0(char* name, proto_id_t* ids, void* data) {
	app_send((app_t*)data, proto_chat_new(name, ids));
	gui_str_free(name); proto_id_free(ids); }

void app_on_leave(proto_id id, void* data) {
	app_send((app_t*)data, proto_chat_leave(id)); };

void app_on_del_acc(proto_id _, void* data) {
	app_send((app_t*)data, proto_delete_account()); }

void app_on_close(void* data) {
	app_t* app = data;
	pthread_cancel(app->listen);
	pthread_join(app->listen, NULL);

	app_mutex_free(&app->mtx);
	net_close(app->sesn); }

void app_on_connect(proto_id chat, void* data) {
	app_t* app = data;
	app->state.chat = chat; app->connd = true;

	app->gui.chat = gui_chat(
		&app->state.users.vals,
		app_on_send, app_on_invite, app);

	gui_window_init(
		app->gui.menu->wnd, app_menu, app);

	app_send(app, proto_chat_connect(chat));
	app_send(app, proto_chat_get_msgs());
	app_send(app, proto_chat_get_users()); }

void app_menu_on_close(void* data) {
	app_on_close(data); app_start(); }

void app_menu(void* data) {
	app_t* app = data;
	app->gui.menu = gui_menu(
		&app->state.users, &app->state.chats,
		app_on_connect, app_on_chat0,
		app_on_leave, app_on_del_acc, app);

	gui_window_init(
		app->gui.menu->wnd, app_menu_on_close, app); }

bool app_on_auth(
	gui_window* wnd, char* uname,
	char* passwd, char* name, void* data)
{
	net_sesn_t* sesn = net_connect(
		"ws://127.0.0.1:1234", uname, passwd, name);

	gui_str_free(uname);
	gui_str_free(passwd);
	if (name != NULL) gui_str_free(name);

	if (sesn == NULL) {
		gui_msg_box(wnd, "chat-client", "auth failed!", MB_ICONERROR);
		return false; }

	app_t* app = calloc(1, sizeof(app_t));
	app->sesn = sesn; app_mutex_init(&app->mtx);
	pthread_create(&app->listen, NULL, app_listen, app);
	
	app_menu(app); return true; }

void app_init() { gui_init(); net_init(); }

void app_start() {
	gui_window_t* wnd = gui_auth(app_on_auth, NULL);
	gui_window_init(wnd, gui_quit_cb, NULL); }

void app_run() { app_start(); gui_run(); }
void app_cleanup() { gui_cleanup(); net_cleanup(); }
