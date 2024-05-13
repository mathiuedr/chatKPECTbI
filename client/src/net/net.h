#if !defined(NET_H)
#define NET_H

#include "../../deps/cJSON/cJSON.h"
#include "../../deps/curl/curl.h"
#include <stdbool.h>
#include <stdint.h>

typedef CURL* net_sesn_t;
typedef struct {
    const char *url0, *uname, *passwd;
} net_init_t;

void net_init(); void net_cleanup();
void net_close(net_sesn_t sesn);

net_sesn_t net_init_reg(net_init_t init, const char* name);
net_sesn_t net_init_login(net_init_t init);

cJSON* net_recv_json(net_sesn_t sesn);
bool net_send_json(net_sesn_t sesn, cJSON* obj);

#endif
