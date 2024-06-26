#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "gui.h"

void gui_users_do(uiButton* _, void* data) {
	gui_users_t* users = data;
	proto_id1_t ids = {};

	char* title = NULL;
	if (users->title) {
		title = uiEntryText(users->title);
		if (strlen(title) == 0) {
			gui_msg_box(
				users->wnd->wnd, "chat-client",
				"please specify a title!", MB_ICONERROR);

			gui_str_free(title); return; } }
	
	LIST_FOREACH(users->gui.chks.vals, chk)
		if (uiCheckboxChecked(chk->chk)) {
			proto_id_t* id1 = proto_id_new(chk->id);
			LIST1_PUSH(&ids, id1); }

	users->cb(title, ids.vals, users->data);
	gui_window_close(users->wnd, true); }

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

		gui_check_free(users->gui.chks.vals);
		users->gui.chks.vals = users->gui.chks.end = NULL;

		gui_window_resize(users->wnd); }

	gui->box = uiNewVerticalBox();
	uiBoxAppend(gui->box0, uiControl(gui->box), true);
	
	uiBoxSetPadded(gui->box, true);
	LIST_FOREACH(*users->state, user)
		gui_users_add(users, user); }

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

	users->gui.box0 = uiNewVerticalBox();

	uiButton* do_ = uiNewButton("invite!");
	users->wnd = gui_window_new(
		uiControl(box), gui_users_free, users);

	uiBoxSetPadded(box, true);
	if (title) {
		uiForm *form = uiNewForm();
		uiBoxAppend(box, uiControl(form), false);

		users->title = uiNewEntry();

		uiFormSetPadded(form, true);
		uiFormAppend(
		    form, "title",
		    uiControl(users->title), false); }

	uiBoxAppend(box, uiControl(grp), true);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));
	
	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(users->gui.box0), true);
	uiBoxAppend(inner, uiControl(do_), false);

	gui_users_init(users);
	uiButtonOnClicked(do_, gui_users_do, users);

	return users; }

void gui_users_remove(gui_users_t* users, proto_id id) {
	size_t idx; gui_check_t *chk0, *chk;
	LIST_FIND(users->gui.chks.vals, id, idx, chk0, chk);

	if (chk) {
		uiBoxDelete(users->gui.box, idx);
		uiControlDestroy(uiControl(chk->chk));
		gui_window_resize(users->wnd);
	
		LIST1_DELETE(&users->gui.chks, chk0, chk); } }
