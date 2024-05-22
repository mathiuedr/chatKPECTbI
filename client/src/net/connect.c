#include <stdio.h>
#include "net.h"

net_sesn_t* net_connect0(char* url) {
	net_sesn_t* sesn = curl_easy_init();
	curl_easy_setopt(sesn, CURLOPT_URL, url);
	curl_easy_setopt(sesn, CURLOPT_CONNECT_ONLY, 2L);

	CURLcode err = curl_easy_perform(sesn);
	return err == 0 ? sesn : NULL; }

net_sesn_t* net_init0_login(
	const char* url0, const char* uname,
	const char* passwd)
{
	char* url = calloc(
		1, 20 + strlen(url0) +
		strlen(uname) + strlen(passwd));

	sprintf(
		url, "%s/?login=%s&password=%s",
		url0, uname, passwd);

	return url; }

char* net_init0_reg(
	const char* url0, const char* uname,
	const char* passwd, const char* name)
{
	char* url = calloc(
		1, 30 + strlen(url0) +
		strlen(uname) + strlen(passwd));

	sprintf(
		url, "%s/?login_reg=%s&password=%s&name=%s",
		url0, uname, passwd, name);

	return url; }

net_sesn_t* net_connect(
	const char* url0, const char* uname,
	const char* passwd, const char* name)
{
	char* url1 = name == NULL
		? net_init0_login(url0, uname, passwd)
		: net_init0_reg(url0, uname, passwd, name);

	net_sesn_t* sesn = net_connect0(url1);
	free(url1); return sesn; }
