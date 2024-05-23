#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "gui.h"

void gui_users_do(uiButton* _, void* data) {
	gui_users_t* chats = data;
	proto_id1_t ids = {};

	char* name = uiEntryText(chats->title);
	if (strlen(name) == 0) {
		gui_msg_box(
			chats->wnd->wnd, "chat-client",
			"please specify a title!", MB_ICONERROR);

		gui_str_free(name); return; }
	
	LIST_FOREACH(chats->gui.chks.vals, chk)
		if (uiCheckboxChecked(chk->chk)) {
			proto_id_t* id1 = calloc(1, sizeof(proto_id_t));
			id1->id = chk->id; LIST1_PUSH(&ids, id1); }

	chats->cb(name, ids.vals, chats->data);
	gui_window_close(chats->wnd, true); }

void gui_users_add(gui_users_t* users, const proto_ent_t* ent) {
	uiCheckbox* chk = uiNewCheckbox(ent->name);
	gui_check_t* chk1 = gui_check_new(chk, ent->id);

	LIST1_PUSH(&users->gui.chks, chk1);
	uiBoxAppend(users->gui.box, uiControl(chk), false); }

void gui_users_init(gui_users_t* users) {
	typeof(users->gui)* gui = &users->gui;
	if (gui->box != NULL) {
		uiBoxDelete(gui->box0, 0);
		uiControlDestroy(uiControl(gui->box));
		gui_window_resize(users->wnd); }

	gui->box = uiNewVerticalBox();
	uiBoxAppend(gui->box0, uiControl(gui->box), true);
	
	uiBoxSetPadded(gui->box, true);
	LIST_FOREACH(*users->state, user) gui_users_add(users, user); }

void gui_users_free(void* data) {
	gui_users_t* users = data;
	gui_check_free(users->gui.chks.vals); free(users); }
	
gui_users_t* gui_users(
    proto_ent_t** users_, bool title,
    gui_users_cb cb, void* data)
{
	gui_users_t* users = calloc(1, sizeof(gui_users_t));
	users->state = users_; users->cb = cb; users->data = data;

	uiGroup* grp = uiNewGroup("invite users");
	uiBox *box = uiNewVerticalBox(),
	      *inner = uiNewVerticalBox();

	uiForm *form = uiNewForm();

	users->title = uiNewEntry();
	users->gui.box0 = uiNewVerticalBox();

	uiButton* do_ = uiNewButton("new chat!");
	users->wnd = gui_window_new(
		uiControl(box), gui_users_free, users);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(form), false);
	uiBoxAppend(box, uiControl(grp), true);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));

	uiFormSetPadded(form, true);
	uiFormAppend(form, "title", uiControl(users->title), false);
	
	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(users->gui.box0), true);
	uiBoxAppend(inner, uiControl(do_), false);

	gui_users_init(users);
	if (!title) uiControlHide(uiControl(users->title));
	
	uiButtonOnClicked(do_, gui_users_do, users);

	return users; }

void gui_users_remove(gui_users_t* users, proto_id id) {
	size_t idx; gui_check_t *chk0, *chk;
	LIST_FIND(users->gui.chks.vals, id, idx, chk0, chk);

	uiBoxDelete(users->gui.box, idx);
	uiControlDestroy(uiControl(chk->chk));
	gui_window_resize(users->wnd);
	
	LIST1_DELETE(&users->gui.chks, chk0, chk); }
