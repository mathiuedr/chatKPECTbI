#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
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

	if (
		strlen(uname) == 0 || strlen(passwd) == 0 ||
		(name != NULL && strlen(name) == 0))
	{
		gui_msg_box(
		    ctx->wnd->wnd, "chat-client",
		    "please specify all fields!", MB_ICONERROR);

		return; }

	bool ok = ctx->cb(
		ctx->wnd->wnd, uname,
		passwd, name, ctx->data);

	if (ok) gui_window_close(ctx->wnd, false); }

void gui_auth_switch(uiButton* _, void* data) {
	gui_auth_ctx_t* ctx = data;
	ctx->reg = !ctx->reg;

	if (!ctx->reg) gui_auth_ctx_login(ctx);
	else gui_auth_ctx_register(ctx); }

gui_window_t* gui_auth(gui_auth_cb cb, void* data) {
	gui_auth_ctx_t* ctx = calloc(1, sizeof(gui_auth_ctx_t));
	ctx->cb = cb; ctx->data = data;

	uiBox *box = uiNewVerticalBox(),
	      *inner = uiNewVerticalBox();

	ctx->grp = uiNewGroup("...");
	uiForm* form = uiNewForm();

	ctx->wnd = gui_window_new(uiControl(box), free, ctx);

	ctx->uname = uiNewEntry();
	ctx->passwd = uiNewPasswordEntry();
	ctx->name = uiNewEntry();

	ctx->do_ = uiNewButton("...");
	ctx->switch_ = uiNewButton("...");
	
	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(ctx->grp), true);
	uiBoxAppend(box, uiControl(ctx->switch_), false);
	
	uiGroupSetMargined(ctx->grp, true);
	uiGroupSetChild(ctx->grp, uiControl(inner));

	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(form), true);
	uiBoxAppend(inner, uiControl(ctx->do_), false);
	
	uiFormSetPadded(form, true);
	uiFormAppend(form, "username", uiControl(ctx->uname), false);
	uiFormAppend(form, "password", uiControl(ctx->passwd), false);
	uiFormAppend(form, "name", uiControl(ctx->name), false);

	uiButtonOnClicked(ctx->do_, gui_auth_do, ctx);
	uiButtonOnClicked(ctx->switch_, gui_auth_switch, ctx);

	gui_auth_ctx_login(ctx); return ctx->wnd; }
