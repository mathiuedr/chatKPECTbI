#include "../gui/gui.h"
#include <stdbool.h>
#include <stdlib.h>
#include "app.h"

void app_on_message(void* data) {
	app_msg_t* msg = data; free(msg); }

void* app_listen(void* data) {
	app_t* app = data;
	while (true) {
		pthread_mutex_lock(&app->mutex);

		json_t* json = net_recv_json(app->sesn);
		proto_res_t* res = proto_res_parse(json);

		app_msg_t* msg = calloc(1, sizeof(app_msg_t));
		msg->app = app; msg->res = res;

		pthread_mutex_unlock(&app->mutex);
		uiQueueMain(app_on_message, res); }
	
	return NULL; }

void app_on_close(void* data) {
	app_t* app = data;
	pthread_cancel(app->listen);
	pthread_mutex_destroy(&app->mutex);
	net_close(app->sesn); }

bool app_on_login(
	gui_window* wnd, bool reg,
	char* uname, char* passwd,
	char* name, void* data)
{
	net_sesn_t* sesn = net_connect(
		"ws://127.0.0.1:1234", uname, passwd, name);

	gui_free_str(uname);
	gui_free_str(passwd);
	if (name != NULL) gui_free_str(name);

	if (sesn == NULL) {
		MessageBoxA(
			(HWND)uiControlHandle(uiControl(wnd)),
			"login failed!", "chat-client", MB_ICONERROR);

		return false; }

	app_t* app = calloc(1, sizeof(app_t));
	app->sesn = sesn;
	pthread_mutex_init(&app->mutex, NULL);
	pthread_create(&app->listen, NULL, app_listen, app);

	// TODO create a new window
	
	return true; }

void app_init() { gui_init(); net_init(); }
void app_run() {
	gui_window* wnd = gui_login(app_on_login, NULL);
	gui_window_init(wnd, NULL, NULL, true);
	gui_run(); }

void app_cleanup() { gui_cleanup(); net_cleanup(); }
