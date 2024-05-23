#include <stdlib.h>
#include "gui.h"

bool gui_init() {
	uiInitOptions opts = { 0 };
	return uiInit(&opts) == NULL; }

void gui_run() { uiMain(); }
void gui_cleanup() { uiUninit(); }

void gui_str_free(char* str) { uiFreeText(str); }
void gui_quit_cb(void* data) { uiQuit(); }

int gui_window_on_close(gui_window* _, void* data) {
	gui_window_t* wnd = data;

	if (wnd->close.fn != NULL) wnd->close.fn(wnd->close.data);
	if (wnd->exit.fn != NULL) wnd->exit.fn(wnd->exit.data);

	free(wnd); return true; }

gui_window_t* gui_window_new
(gui_ctrl* ctrl, gui_close_cb close_cb, void* data) {
	gui_window_t* wnd = calloc(1, sizeof(gui_window_t));
	wnd->close.fn = close_cb; wnd->close.data = data;

	gui_window* wnd0 = uiNewWindow("chat-client", 0, 0, false);
	uiWindowSetChild(wnd0, ctrl);
	uiWindowSetMargined(wnd0, true);

	wnd->wnd = wnd0; return wnd; }

void gui_window_init
(gui_window_t* wnd, gui_close_cb close_cb, void* data) {
	wnd->exit.fn = close_cb; wnd->exit.data = data;

	uiWindowOnClosing(wnd->wnd, gui_window_on_close, wnd);
	uiControlShow(uiControl(wnd->wnd)); }

void gui_window_resize(gui_window_t* wnd) {
	uiWindowSetContentSize(wnd->wnd, 0, 0); }

void gui_window_close(gui_window_t* wnd, bool exit) {
	if (!exit) wnd->exit.fn = NULL;

	HWND wnd0 = (HWND)uiControlHandle(uiControl(wnd->wnd));
	SendMessageA(wnd0, WM_CLOSE, 0, 0); }

gui_check_t* gui_check_new(uiCheckbox* chk, proto_id id) {
	gui_check_t* chk1 = calloc(1, sizeof(gui_check_t));
	chk1->chk = chk; chk1->id = id; return chk1; }

void gui_check_free(gui_check_t* chk) { LIST_FREE(chk, _) {} }

void gui_msg_box(
	gui_window* wnd, const char* title,
	const char* msg, uint32_t flags)
{
	MessageBoxA(
		(HWND)uiControlHandle(uiControl(wnd)),
		msg, title, flags); }
