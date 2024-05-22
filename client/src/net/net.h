#if !defined(NET_H)
#define NET_H

#include "../../deps/curl/curl.h"
#include "../../deps/cJSON.h"
#include <stdbool.h>
#include <stdint.h>

typedef CURL net_sesn_t;
typedef cJSON json_t;

bool net_init(); void net_cleanup();
void net_close(net_sesn_t* sesn);
net_sesn_t* net_connect(
	const char* url0, const char* uname,
	const char* passwd, const char* name);

json_t* net_recv_json(net_sesn_t* sesn);

// this consumes the `obj` arg
bool net_send_json(net_sesn_t* sesn, json_t* obj);

#endif
