#include <stdlib.h>
#include "gui.h"

bool gui_init() {
	uiInitOptions opts = { 0 };
	return uiInit(&opts) == NULL; }

void gui_cleanup() { uiUninit(); }
void gui_run() { uiMain(); }

void gui_free_str(char* str) { uiFreeText(str); }
void gui_check_free(gui_check_t* chk) { LIST_FREE(chk, _) {} }

int gui_window_on_close(gui_window* _, void* data) {
	gui_window_t* wnd = data;

	if (wnd->cb0 != NULL) wnd->cb0(wnd->data0);
	else free(wnd->data0);

	if (wnd->cb != NULL) wnd->cb(wnd->data);

	gui_window_close(wnd, true);
	if (wnd->exit) uiQuit(); return true; }

gui_window_t* gui_window_new(
	const char* title, gui_ctrl* ctrl,
	gui_close_cb close0_cb, void* data0)
{
	gui_window_t* wnd = calloc(1, sizeof(gui_window_t));
	wnd->cb0 = close0_cb; wnd->data0 = data0;

	gui_window* wnd0 = uiNewWindow(title, 0, 0, false);
	uiWindowSetChild(wnd0, ctrl);
	uiWindowSetMargined(wnd0, true);

	wnd->wnd = wnd0; return wnd; }

void gui_window_init(
	gui_window_t* wnd, gui_close_cb close_cb,
	void* data, bool exit)
{
	wnd->cb = close_cb; wnd->data = data; wnd->exit = exit;
	uiWindowOnClosing(wnd->wnd, gui_window_on_close, wnd);
	uiControlShow(uiControl(wnd->wnd)); }

void gui_window_close(gui_window_t* wnd, bool exit) {
	if (exit && wnd->exit) uiQuit();
	else uiControlDestroy(uiControl(wnd->wnd));
	free(wnd); }

void gui_msg_box(
	gui_window* wnd, const char* title,
	const char* msg, uint32_t flags)
{
	MessageBoxA(
		(HWND)uiControlHandle(uiControl(wnd)),
		msg, title, flags); }
