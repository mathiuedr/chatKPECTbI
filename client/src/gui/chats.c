#include <stdint.h>
#include <stdlib.h>
#include "gui.h"

void gui_chats_do(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	ssize_t idx = uiComboboxSelected(chats->gui.chats);

	if (idx == -1) {
		gui_msg_box(
			chats->wnd->wnd, "chat-client",
			"please select a chat!", MB_ICONEXCLAMATION);

		return; }

	const proto_ent_t* ent = *chats->state.chats;
	while (idx > 0) { ent = ent->next; idx--; }

	chats->cb.do_(ent->id, chats->cb.data);
	gui_window_close(chats->wnd); }

void gui_chats_free0(void* data) {
	gui_chats_t* chats = data; chats->child = NULL;
	uiControlEnable(uiControl(chats->gui.new_)); }

void gui_chats_new(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	uiControlDisable(uiControl(chats->gui.new_));
	
	chats->child = gui_chats0(
		chats->state.users,
		chats->cb.new_, chats->cb.data);

	gui_window_init(
		chats->child->wnd, gui_chats_free0, chats); }

void gui_chats_init(gui_chats_t* chats) {
	uiComboboxClear(chats->gui.chats);
	LIST_FOREACH(*chats->state.chats, chat)
		uiComboboxAppend(chats->gui.chats, chat->name); }

gui_chats_t* gui_chats(
	proto_ent_ptr users, proto_ent_ptr chats_,
	gui_chat_connect_cb conn_cb,
	gui_chat_new_cb new_cb, void* data)
{
	gui_chats_t* chats = calloc(1, sizeof(gui_chats_t));
	chats->state.users = users;
	chats->state.chats = chats_;

	chats->cb.do_ = conn_cb; chats->cb.new_ = new_cb;
	chats->cb.data = data;

	uiGroup* grp = uiNewGroup("select a chat");
	uiBox *box = uiNewVerticalBox(),
		  *inner = uiNewVerticalBox();

	chats->gui.chats = uiNewCombobox();
	uiButton* do_ = uiNewButton("connect!");
	chats->gui.new_ = uiNewButton("new chat...");

	chats->wnd = gui_window_new(uiControl(box), free, chats);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(grp), true);
	uiBoxAppend(box, uiControl(chats->gui.new_), false);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));

	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(chats->gui.chats), false);
	uiBoxAppend(inner, uiControl(do_), false);

	uiButtonOnClicked(do_, gui_chats_do, chats);
	uiButtonOnClicked(chats->gui.new_, gui_chats_new, chats);

	gui_chats_init(chats); return chats; }

void gui_chats_add_entry
(gui_chats_t* chats, bool user, const proto_ent_t* entry) {
	if (user && chats->child != NULL)
		gui_chats0_add_user(chats->child, entry);
	else uiComboboxAppend(chats->gui.chats, entry->name); }

void gui_chats_refresh(gui_chats_t* chats, bool user) {
	if (user && chats->child != NULL)
		gui_chats0_init(chats->child);
	else gui_chats_init(chats); }
