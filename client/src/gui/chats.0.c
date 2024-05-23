#include <stdint.h>
#include <stdlib.h>
#include "gui.h"

void gui_chats0_do(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	proto_id1_t ids = {};

	char* name = uiEntryText(chats->title);
	
	LIST_FOREACH(chats->ui.users.vals, chk)
		if (uiCheckboxChecked(chk->chk)) {
			proto_id_t* id1 = calloc(1, sizeof(proto_id_t));
			id1->id = chk->id; LIST1_PUSH(&ids, id1); }

	chats->new_(name, ids.vals, chats->data);
	gui_window_close(chats->wnd1); }

void gui_chats0_add_user
(gui_chats_t* chats, const proto_ent_t* ent) {
	gui_check1_t* chks = &chats->ui.users;
	uiCheckbox* chk = uiNewCheckbox(ent->name);

	gui_check_t* chk1 = calloc(1, sizeof(gui_check_t));
	chk1->chk = chk; chk1->id = ent->id;
	LIST1_PUSH(chks, chk1);

	uiBoxAppend(chks->box, uiControl(chk), false); }

void gui_chats0_init(gui_chats_t* chats) {
	gui_check1_t* chks = &chats->ui.users;

	if (chks->box != NULL) {
		uiBoxDelete(chks->box0, 0);
		uiControlDestroy(uiControl(chks->box)); }

	chks->box = uiNewVerticalBox();
	uiBoxAppend(chks->box0, uiControl(chks->box), true);
	
	uiBoxSetPadded(chks->box, true);
	LIST_FOREACH(*chats->state.users, user)
		gui_chats0_add_user(chats, user); }

void gui_chats0_free(void* data) {
	gui_chats_t* chats = data;
	gui_check1_t* chks = &chats->ui.users;
	
	chks->box = NULL;
	gui_check_free(chks->vals);
	chks->vals = NULL; chks->end = NULL;
	
	uiControlEnable(uiControl(chats->ui.new_)); }

void gui_chats0(uiButton* _, void* data) {
	gui_chats_t* chats = data;

	uiControlDisable(uiControl(chats->ui.new_));

	uiGroup* grp = uiNewGroup("invite users");
	uiBox *box = uiNewVerticalBox(),
	      *inner = uiNewVerticalBox();

	uiForm *form = uiNewForm();

	chats->title = uiNewEntry();
	chats->ui.users.box0 = uiNewVerticalBox();

	uiButton* do_ = uiNewButton("new chat!");
	chats->wnd1 = gui_window_new(
		uiControl(box), gui_chats0_free, chats);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(form), false);
	uiBoxAppend(box, uiControl(grp), true);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));

	uiFormSetPadded(form, true);
	uiFormAppend(form, "title", uiControl(chats->title), false);
	
	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(chats->ui.users.box0), true);
	uiBoxAppend(inner, uiControl(do_), false);

	gui_chats0_init(chats);
	
	uiButtonOnClicked(do_, gui_chats0_do, chats);

	gui_window_init(chats->wnd1, NULL, NULL); }
