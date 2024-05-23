#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "gui.h"

void gui_chats0_do(uiButton* _, void* data) {
	gui_chats0_t* chats = data;
	proto_id1_t ids = {};

	char* name = uiEntryText(chats->title);
	if (strlen(name) == 0) {
		gui_msg_box(
		    chats->wnd->wnd, "chat-client",
		    "please specify a title!", MB_ICONERROR);

		return; }
	
	LIST_FOREACH(chats->gui.chks.vals, chk)
		if (uiCheckboxChecked(chk->chk)) {
			proto_id_t* id1 = calloc(1, sizeof(proto_id_t));
			id1->id = chk->id; LIST1_PUSH(&ids, id1); }

	chats->cb(name, ids.vals, chats->data);
	gui_window_close(chats->wnd, true); }

void gui_chats0_add_user
(gui_chats0_t* chats0, const proto_ent_t* ent) {
	uiCheckbox* chk = uiNewCheckbox(ent->name);
	gui_check_t* chk1 = gui_check_new(chk, ent->id);

	LIST1_PUSH(&chats0->gui.chks, chk1);
	uiBoxAppend(chats0->gui.box, uiControl(chk), false); }

void gui_chats0_init(gui_chats0_t* chats0) {
	typeof(chats0->gui)* gui = &chats0->gui;
	if (gui->box != NULL) {
		uiBoxDelete(gui->box0, 0);
		uiControlDestroy(uiControl(gui->box));
		gui_window_resize(chats0->wnd); }

	gui->box = uiNewVerticalBox();
	uiBoxAppend(gui->box0, uiControl(gui->box), true);
	
	uiBoxSetPadded(gui->box, true);
	LIST_FOREACH(*chats0->state, user)
		gui_chats0_add_user(chats0, user); }

void gui_chats0_free(void* data) {
	gui_chats0_t* chats0 = data;
	gui_check_free(chats0->gui.chks.vals); free(chats0); }
	
gui_chats0_t* gui_chats0
(proto_ent_t** users, gui_chat_new_cb cb, void* data) {
	gui_chats0_t* chats0 = calloc(1, sizeof(gui_chats0_t));
	chats0->state = users; chats0->cb = cb; chats0->data = data;

	uiGroup* grp = uiNewGroup("invite users");
	uiBox *box = uiNewVerticalBox(),
	      *inner = uiNewVerticalBox();

	uiForm *form = uiNewForm();

	chats0->title = uiNewEntry();
	chats0->gui.box0 = uiNewVerticalBox();

	uiButton* do_ = uiNewButton("new chat!");
	chats0->wnd = gui_window_new(
		uiControl(box), gui_chats0_free, chats0);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(form), false);
	uiBoxAppend(box, uiControl(grp), true);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));

	uiFormSetPadded(form, true);
	uiFormAppend(form, "title", uiControl(chats0->title), false);
	
	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(chats0->gui.box0), true);
	uiBoxAppend(inner, uiControl(do_), false);

	gui_chats0_init(chats0);
	
	uiButtonOnClicked(do_, gui_chats0_do, chats0);

	return chats0; }

void gui_chats0_remove_user
(gui_chats0_t* chats0, proto_id id) {
	size_t idx; gui_check_t *chk0, *chk;
	LIST_FIND(chats0->gui.chks.vals, id, idx, chk0, chk);

	uiBoxDelete(chats0->gui.box, idx);
	uiControlDestroy(uiControl(chk->chk));
	gui_window_resize(chats0->wnd);
	
	LIST1_DELETE(&chats0->gui.chks, chk0, chk); }
