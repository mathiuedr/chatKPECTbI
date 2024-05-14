#include "net/net.h"
#include <stdio.h>

int main() {
	net_init();
	net_connect_t cfg = {
		NET_INIT_LOGIN,
		"ws://127.0.0.1:1234",
		"username", "password", NULL };
	
	net_sesn_t* sesn = net_connect(&cfg);
	while (true) {
		printf("net_recv_json...\n");
		json_t* obj = net_recv_json(sesn);

		printf("%s\n", cJSON_Print(obj));
		cJSON_Delete(obj); }

	net_cleanup(); return 0; }
