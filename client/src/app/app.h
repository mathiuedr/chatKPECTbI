#if !defined(APP_H)
#define APP_H

#include "../proto/proto.h"
#include "../gui/gui.h"
#include "../net/net.h"

#include <pthread.h>
#include <stdbool.h>

typedef struct {
	pthread_mutex_t mtx;
	pthread_cond_t cond;
	size_t idx, idx1; } app_mutex_t;

typedef struct {
	net_sesn_t* sesn;
	app_mutex_t mtx; pthread_t listen;

	bool connd;
	union {
		gui_menu_t* menu;
		/* gui_chat_t* chat; */ } gui;

	struct {
		proto_ent1_t users, users0, chats;
		proto_id self; } state;
} app_t;

typedef struct {
	app_t* app; proto_res_t* res;
} app_msg_t;

void app_mutex_init(app_mutex_t* mtx);
void app_mutex_lock(app_mutex_t* mtx);
void app_mutex_unlock(app_mutex_t* mtx);
void app_mutex_free(app_mutex_t* mtx);

void* app_listen(void* data);

void app_init(); void app_run(); void app_cleanup();

#endif
