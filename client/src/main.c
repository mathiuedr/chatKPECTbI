#include <stdio.h>
#include "net/net.h"

int main() {
	net_init();
	net_init_t init = {
		"ws://0.0.0.0:1234",
		"username", "password" };
	
	net_sesn_t sesn = net_init_login(init);
	while (true) {
		printf("net_recv_json...\n");
		cJSON* obj = net_recv_json(sesn);

		printf("%s\n", cJSON_Print(obj));
		cJSON_Delete(obj); }

	net_cleanup(); return 0; }
