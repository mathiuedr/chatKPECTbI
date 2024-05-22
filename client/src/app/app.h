#if !defined(APP_H)
#define APP_H

#include "../proto/proto.h"
#include "../net/net.h"
#include <pthread.h>

typedef struct {
	net_sesn_t* sesn;
	pthread_mutex_t mutex;
	pthread_t listen; } app_t;

typedef struct {
	app_t* app; proto_res_t* res;
} app_msg_t;

// app.c

void app_init(); void app_cleanup();
void app_run();

// chat.c
void app_on_message(void* data);

#endif
