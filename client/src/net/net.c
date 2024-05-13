#include <stdio.h>
#include "net.h"

void net_init() { curl_global_init(CURL_GLOBAL_ALL); }
void net_cleanup() { curl_global_cleanup(); }

CURL* net_connect(char* url) {
	CURL* sesn = curl_easy_init();
	curl_easy_setopt(sesn, CURLOPT_URL, url);
	curl_easy_setopt(sesn, CURLOPT_CONNECT_ONLY, 2L);

	CURLcode err = curl_easy_perform(sesn);
	return err == 0 ? sesn : NULL; }

void net_close(net_sesn_t sesn) { curl_easy_cleanup(sesn); }

net_sesn_t net_init_login(net_init_t init) {
	char* url = calloc(
		1, 20 + strlen(init.url0) +
		strlen(init.uname) + strlen(init.passwd));

	sprintf(
		url, "%s/?login=%s&password=%s",
		init.url0, init.uname, init.passwd);

	return net_connect(url); }

net_sesn_t net_init_reg(net_init_t init, const char* name) {
	char* url = calloc(
		1, 30 + strlen(init.url0) +
		strlen(init.uname) + strlen(init.passwd));

	sprintf(
		url, "%s/?login_reg=%s&password=%s&name=%s",
		init.url0, init.uname, init.passwd, name);

	return net_connect(url); }

cJSON* net_recv_json(CURL* sesn) {
	size_t recv; const struct curl_ws_frame *frame;
	size_t bufidx = 0, buflen = 256;
	char *buf = malloc(buflen);
	
	CURLcode err = CURLE_AGAIN;
	bool recv_ok = false;

	while (
		err == CURLE_OK || err == CURLE_AGAIN ||
		err == CURLE_GOT_NOTHING)
	{
		err = curl_ws_recv(
			sesn, buf + bufidx, buflen - bufidx,
			&recv, &frame);
		
		if (err == CURLE_OK) {
			const size_t left = frame->bytesleft;
			if (left > 0) {
				bufidx += recv; buflen += left;
				buf = realloc(buf, buflen);
				if (buf == NULL) return NULL; }

			else { recv_ok = true; break; } } }

	if (!recv_ok) return NULL;
	cJSON* obj = cJSON_ParseWithLength(buf, bufidx + recv);

	free(buf);
	if (cJSON_IsInvalid(obj)) return NULL;
	return obj; }

bool net_send_json(CURL* sesn, cJSON* obj) {
	size_t _sent;
	char* str = cJSON_Print(obj);
	CURLcode err = curl_ws_send(
		sesn, str, strlen(str),
		&_sent, 0, CURLWS_TEXT);

	free(str); return err != 0; }
