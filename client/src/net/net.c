#include <stdio.h>
#include "net.h"

typedef struct curl_ws_frame net_frame0_t;
typedef struct {
	char* buf; size_t len;
	const net_frame0_t* frame;
} net_frame_t;

void net_frame_free(net_frame_t* frm) { free(frm->buf); }
bool net_init() {
	return curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK; }

void net_close(net_sesn_t* sesn) { curl_easy_cleanup(sesn); }
void net_cleanup() { curl_global_cleanup(); }

bool net_make_fd_set(net_sesn_t* sesn, fd_set* fds) {
	curl_socket_t sock;
	CURLcode err = curl_easy_getinfo(
		sesn, CURLINFO_ACTIVESOCKET, &sock);
	if (err != CURLE_OK) return false;

	FD_ZERO(fds); FD_SET(sock, fds); return true; }

bool net_recv_block(
	net_sesn_t* sesn, char* buf, size_t len,
	size_t* recv, const net_frame0_t** frame)
{
	CURLcode err = curl_ws_recv(sesn, buf, len, recv, frame);
	if (err == CURLE_AGAIN) {
		fd_set fds; net_make_fd_set(sesn, &fds);
		if (select(0, &fds, NULL, NULL, NULL) != 1)
			return false;

		err = curl_ws_recv(sesn, buf, len, recv, frame); }

	return err == CURLE_OK; }

bool net_send_block(
	net_sesn_t* sesn, const char* buf, size_t len,
	size_t* sent, size_t frag, uint32_t flags)
{
	CURLcode err = curl_ws_send(sesn, buf, len, sent, frag, flags);
	if (err == CURLE_AGAIN) {
		fd_set fds; net_make_fd_set(sesn, &fds);
		if (select(0, NULL, &fds, NULL, NULL) != 1)
			return false;

		err = curl_ws_send(sesn, buf, len, sent, frag, flags); }

	return err == CURLE_OK; }

bool net_recv_frame(net_sesn_t* sesn, net_frame_t* frm) {
	size_t len = 256, idx = 0, recv;
	frm->buf = malloc(len);
	bool recv_ok = false;

	while (net_recv_block(
		sesn, frm->buf + idx, len - idx,
		&recv, &frm->frame))
	{
		const size_t left = frm->frame->bytesleft;
		if (left > 0) {
			idx += recv; len += left;
			frm->buf = realloc(frm->buf, len);
			if (frm->buf == NULL) return false; }

		else { recv_ok = true; break; } }

	if (recv_ok) frm->len = idx + recv;
	else free(frm->buf);
	return recv_ok; }

bool net_send_pong(net_sesn_t* sesn) {
	size_t _sent;
	CURLcode err = net_send_block(
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

// this consumes the `obj` arg
bool net_send_json(net_sesn_t* sesn, json_t* obj) {
	size_t _sent;
	char* str = cJSON_Print(obj); cJSON_Delete(obj);
	CURLcode err = net_send_block(
		sesn, str, strlen(str), &_sent, 0, CURLWS_TEXT);

	free(str); return err != 0; }
