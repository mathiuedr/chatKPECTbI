#include <stdlib.h>
#include "gui.h"

bool gui_init() {
	uiInitOptions opts = { 0 };
	return uiInit(&opts) == NULL; }

void gui_cleanup() { uiUninit(); }

typedef struct {
	bool exit;
	gui_close_cb cb; void* data;
} gui_window_ctx_t;

int gui_window_on_close(gui_window* wnd, void* data) {
	gui_window_ctx_t* ctx = data;
	if (ctx->cb != NULL) ctx->cb(ctx->data);

	uiControlDestroy(uiControl(wnd));
	if (ctx->exit) uiQuit();
	free(ctx); return 1; }

gui_window* gui_window_new(const char* title, gui_ctrl* ctrl) {
	gui_window* wnd = uiNewWindow(title, 0, 0, false);
	uiWindowSetChild(wnd, ctrl);
	uiWindowSetMargined(wnd, true);
	return wnd; }

void gui_window_init
(gui_window* wnd, gui_close_cb close_cb, void* data, bool exit) {
	gui_window_ctx_t* ctx = calloc(1, sizeof(gui_window_ctx_t));
	ctx->cb = close_cb; ctx->data = data; ctx->exit = exit;

	uiWindowOnClosing(wnd, gui_window_on_close, ctx);
	uiControlShow(uiControl(wnd)); }

void gui_run() { uiMain(); }

void gui_free_str(char* str) { uiFreeText(str); }
