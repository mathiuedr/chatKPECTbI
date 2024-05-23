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

	chats->do_(ent->id, chats->data);
	gui_window_close(chats->wnd); }

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
	uiBox *box = uiNewVerticalBox(),
	      *inner = uiNewVerticalBox();

	chats->ui.chats = uiNewCombobox();
	gui_chats_init(chats);

	uiButton* do_ = uiNewButton("connect!");
	chats->ui.new_ = uiNewButton("new chat...");

	chats->wnd = gui_window_new(uiControl(box), free, chats);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(grp), true);
	uiBoxAppend(box, uiControl(chats->ui.new_), false);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));

	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(chats->ui.chats), false);
	uiBoxAppend(inner, uiControl(do_), false);

	uiButtonOnClicked(do_, gui_chats_do, chats);
	uiButtonOnClicked(chats->ui.new_, gui_chats0, chats);

	return chats; }

// this consumes the `name` arg
void gui_chats_add_entry
(gui_chats_t* chats, bool user, const proto_ent_t* entry) {
	if (user && chats->wnd1 != NULL)
		gui_chats0_add_user(chats, entry);
	else uiComboboxAppend(chats->ui.chats, entry->name); }

void gui_chats_refresh(gui_chats_t* chats, bool user) {
	if (user && chats->wnd1 != NULL) gui_chats0_init(chats);
	else gui_chats_init(chats); }
