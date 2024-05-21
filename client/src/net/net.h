#if !defined(NET_H)
#define NET_H

#include "../../deps/curl/curl.h"
#include "../../deps/cJSON.h"
#include <stdbool.h>
#include <stdint.h>

typedef CURL net_sesn_t;
typedef cJSON json_t;

typedef struct {
	enum { NET_INIT_LOGIN, NET_INIT_REG } kind;
	const char *url0, *uname, *passwd, *name;
} net_connect_t;

void net_init(); void net_cleanup();
void net_close(net_sesn_t* sesn);
net_sesn_t* net_connect(const net_connect_t* cfg);

json_t* net_recv_json(net_sesn_t* sesn);

// this consumes the `obj` arg
bool net_send_json(net_sesn_t* sesn, json_t* obj);

#endif
