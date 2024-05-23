#include <stdint.h>
#include <stdlib.h>
#include "gui.h"

void gui_chats_do(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	ssize_t idx = uiComboboxSelected(chats->ui.chats);

	if (idx == -1) {
		gui_msg_box(
			chats->wnd->wnd, "chat-client",
			"please select a chat!", MB_ICONEXCLAMATION);

		return; }

	const proto_ent_t* ent = *chats->state.chats;
	while (idx > 0) { ent = ent->next; idx--; }

	chats->do_(ent->id, chats->data); free(chats); }

void gui_chats0_do(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	gui_msg_box(
		chats->wnd->wnd, "chat-client",
		"TODO :p", MB_ICONERROR);
	/* gui_chats_new_free(chats); */ }

void gui_chats0_add_user(gui_chats_t* chats, char* name) {
	gui_check1_t* chks = &chats->ui.users;
	uiCheckbox* chk = uiNewCheckbox(name);

	gui_check_t* chk1 = calloc(1, sizeof(gui_check_t));
	chk1->chk = chk; LIST1_PUSH(chks, chk1);

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
		gui_chats0_add_user(chats, user->name); }

void gui_chats0_free(void* data) {
	gui_chats_t* chats = data;
	uiControlDestroy(uiControl(chats->wnd1));

	gui_check_free(chats->ui.users.vals);
	uiControlEnable(uiControl(chats->ui.new_)); }

void gui_chats0(uiButton* _, void* data) {
	gui_chats_t* chats = data;

	uiControlDisable(uiControl(chats->ui.new_));

	uiGroup* grp = uiNewGroup("invite users");
	uiBox *box = uiNewVerticalBox(),
	      *inner = chats->ui.users.box0 = uiNewVerticalBox();

	uiButton* do_ = uiNewButton("new chat!");
	chats->wnd1 = gui_window_new(
		"chat-client: new chat", uiControl(grp),
		gui_chats0_free, chats);

	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(box));

	gui_chats0_init(chats);
	
	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(inner), true);
	uiBoxAppend(box, uiControl(do_), false);

	uiButtonOnClicked(do_, gui_chats0_do, chats);
	gui_window_init(
		chats->wnd1, gui_chats0_free, chats, false); }

void gui_chats_init(gui_chats_t* chats) {
	uiComboboxClear(chats->ui.chats);
	LIST_FOREACH(*chats->state.chats, chat)
		uiComboboxAppend(chats->ui.chats, chat->name); }

gui_chats_t* gui_chats(
	proto_ent_ptr users, proto_ent_ptr chats_,
	gui_chat_connect_cb conn_cb,
	gui_chat_new_cb new_cb, void* data)
{
	gui_chats_t* chats = calloc(1, sizeof(gui_chats_t));
	chats->state.users = users;
	chats->state.chats = chats_;

	chats->do_ = conn_cb; chats->new_ = new_cb;
	chats->data = data;

	uiGroup* grp = uiNewGroup("select a chat");
	uiBox* box = uiNewVerticalBox();

	chats->ui.chats = uiNewCombobox();
	gui_chats_init(chats);

	uiButton* do_ = uiNewButton("connect!");
	chats->ui.new_ = uiNewButton("new chat...");

	chats->wnd = gui_window_new(
		"chat-client: chats", uiControl(grp), NULL, chats);

	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(box));

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(chats->ui.chats), false);
	uiBoxAppend(box, uiControl(do_), false);
	uiBoxAppend(box, uiControl(chats->ui.new_), false);

	uiButtonOnClicked(do_, gui_chats_do, chats);
	uiButtonOnClicked(chats->ui.new_, gui_chats0, chats);

	return chats; }

// this consumes the `name` arg
void gui_chats_add_entry
(gui_chats_t* chats, bool user, const proto_ent_t* entry) {
	if (user && chats->wnd1 != NULL)
		gui_chats0_add_user(chats, entry->name);
	else uiComboboxAppend(chats->ui.chats, entry->name); }

void gui_chats_refresh(gui_chats_t* chats, bool user) {
	if (user && chats->wnd1 != NULL) gui_chats0_init(chats);
	else gui_chats_init(chats); }
