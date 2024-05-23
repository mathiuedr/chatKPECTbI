#include <stdbool.h>
#include <stdlib.h>
#include "gui.h"

typedef struct {
	gui_window_t* wnd; bool reg;
	uiGroup* grp; uiButton *do_, *switch_;

	uiEntry *uname, *passwd, *name;
	gui_auth_cb cb; void* data;
} gui_auth_ctx_t;

void gui_auth_ctx_login(gui_auth_ctx_t* ctx) {
	uiGroupSetTitle(ctx->grp, "login");

	uiButtonSetText(ctx->do_, "login!");
	uiButtonSetText(ctx->switch_, "register...");

	uiControlHide(uiControl(ctx->name)); }

void gui_auth_ctx_register(gui_auth_ctx_t* ctx) {
	uiGroupSetTitle(ctx->grp, "register");

	uiButtonSetText(ctx->do_, "register!");
	uiButtonSetText(ctx->switch_, "login...");

	uiControlShow(uiControl(ctx->name)); }

void gui_auth_do(uiButton* _, void* data) {
	gui_auth_ctx_t* ctx = data;

	char* uname = uiEntryText(ctx->uname);
	char* passwd = uiEntryText(ctx->passwd);
	char* name = ctx->reg ? uiEntryText(ctx->name) : NULL;

	bool ok = ctx->cb(
		ctx->wnd->wnd, uname,
		passwd, name, ctx->data);

	if (ok) {
		gui_window_close(ctx->wnd, false);
		free(ctx); } }

void gui_auth_switch(uiButton* _, void* data) {
	gui_auth_ctx_t* ctx = data;
	ctx->reg = !ctx->reg;

	if (!ctx->reg) gui_auth_ctx_login(ctx);
	else gui_auth_ctx_register(ctx); }

gui_window_t* gui_auth(gui_auth_cb cb, void* data) {
	gui_auth_ctx_t* ctx = calloc(1, sizeof(gui_auth_ctx_t));
	ctx->cb = cb; ctx->data = data;

	ctx->grp = uiNewGroup("...");
	uiBox* box = uiNewVerticalBox();
	uiForm* form = uiNewForm();

	ctx->wnd = gui_window_new(
		"chat-client: auth", uiControl(ctx->grp), NULL, ctx);

	ctx->uname = uiNewEntry();
	ctx->passwd = uiNewPasswordEntry();
	ctx->name = uiNewEntry();

	ctx->do_ = uiNewButton("...");
	ctx->switch_ = uiNewButton("...");

	uiGroupSetMargined(ctx->grp, true);
	uiGroupSetChild(ctx->grp, uiControl(box));
	
	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(form), true);
	uiBoxAppend(box, uiControl(ctx->do_), false);
	uiBoxAppend(box, uiControl(ctx->switch_), false);
	
	uiFormSetPadded(form, true);
	uiFormAppend(form, "username", uiControl(ctx->uname), false);
	uiFormAppend(form, "password", uiControl(ctx->passwd), false);
	uiFormAppend(form, "name", uiControl(ctx->name), false);

	uiButtonOnClicked(ctx->do_, gui_auth_do, ctx);
	uiButtonOnClicked(ctx->switch_, gui_auth_switch, ctx);

	gui_auth_ctx_login(ctx); return ctx->wnd; }
