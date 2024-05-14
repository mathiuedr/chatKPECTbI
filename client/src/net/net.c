#include <stdio.h>
#include "net.h"

typedef struct curl_ws_frame net_frame0_t;
typedef struct {
	char* buf; size_t len;
	const net_frame0_t* frame;
} net_frame_t;

void net_frame_free(net_frame_t* frm) { free(frm->buf); }

void net_init() { curl_global_init(CURL_GLOBAL_ALL); }
void net_cleanup() { curl_global_cleanup(); }

net_sesn_t* net_connect0(char* url) {
	net_sesn_t* sesn = curl_easy_init();
	curl_easy_setopt(sesn, CURLOPT_URL, url);
	curl_easy_setopt(sesn, CURLOPT_CONNECT_ONLY, 2L);

	CURLcode err = curl_easy_perform(sesn);
	return err == 0 ? sesn : NULL; }

char* net_init0_login(const net_connect_t* cfg) {
	char* url = calloc(
		1, 20 + strlen(cfg->url0) +
		strlen(cfg->uname) + strlen(cfg->passwd));

	sprintf(
		url, "%s/?login=%s&password=%s",
		cfg->url0, cfg->uname, cfg->passwd);

	return url; }

char* net_init0_reg(const net_connect_t* cfg) {
	char* url = calloc(
		1, 30 + strlen(cfg->url0) +
		strlen(cfg->uname) + strlen(cfg->passwd));

	sprintf(
		url, "%s/?login_reg=%s&password=%s&name=%s",
		cfg->url0, cfg->uname, cfg->passwd, cfg->name);

	return url; }

net_sesn_t* net_connect(const net_connect_t* cfg) {
	char* url = cfg->kind == NET_INIT_LOGIN
	    ? net_init0_login(cfg) : net_init0_reg(cfg);

	net_sesn_t* sesn = net_connect0(url);
	free(url); return sesn; }

void net_close(net_sesn_t* sesn) { curl_easy_cleanup(sesn); }

bool net_recv_frame(net_sesn_t* sesn, net_frame_t* frm) {
	size_t len = 256, idx = 0, recv;
	frm->buf = malloc(len);
	CURLcode err = CURLE_AGAIN;
	bool recv_ok = false;

	while (
		err == CURLE_OK || err == CURLE_AGAIN ||
		err == CURLE_GOT_NOTHING)
	{
		err = curl_ws_recv(
			sesn, frm->buf + idx, len - idx,
			&recv, &frm->frame);
		
		if (err == CURLE_OK) {
			const size_t left = frm->frame->bytesleft;
			if (left > 0) {
				idx += recv; len += left;
				frm->buf = realloc(frm->buf, len);
				if (frm->buf == NULL) return false; }

			else { recv_ok = true; break; } } }

	if (recv_ok) frm->len = idx + recv;
	else free(frm->buf);
	return recv_ok; }

bool net_send_pong(net_sesn_t* sesn) {
	size_t _sent;
	CURLcode err = curl_ws_send(
		sesn, NULL, 0, &_sent, 0, CURLWS_PONG);
	return err != 0; }

json_t* net_recv_json(net_sesn_t* sesn) {
	net_frame_t frm; bool recv_ok = false;
	while (!recv_ok && net_recv_frame(sesn, &frm)) {
		switch (frm.frame->flags) {
			case CURLWS_CLOSE: return NULL;
			case CURLWS_PING: net_send_pong(sesn); break;

			default: recv_ok = true; break; } }

	if (recv_ok) {
		json_t* obj = cJSON_ParseWithLength(frm.buf, frm.len);

		net_frame_free(&frm);
		return !cJSON_IsInvalid(obj) ? obj : NULL;
	} else return NULL; }

bool net_send_json(net_sesn_t* sesn, json_t* obj) {
	size_t _sent;
	char* str = cJSON_Print(obj);

	CURLcode err = curl_ws_send(
		sesn, str, strlen(str), &_sent, 0, CURLWS_TEXT);

	free(str); return err != 0; }
