#include <stdint.h>
#include <stdlib.h>
#include "gui.h"

void gui_chats_error(gui_chats_t* chats) {
	gui_msg_box(
		chats->wnd->wnd, "chat-client",
		"please select a chat!", MB_ICONEXCLAMATION); }

void gui_chats_do(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	ssize_t idx = uiComboboxSelected(chats->gui.chats);
	if (idx == -1) { gui_chats_error(chats); return; }

	const proto_ent_t* ent = chats->state.chats->vals;
	while (idx > 0) { ent = ent->next; idx--; }

	chats->cb.do_(ent->id, chats->cb.data);
	gui_window_close(chats->wnd, false); }

void gui_chats_leave(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	ssize_t idx = uiComboboxSelected(chats->gui.chats);
	if (idx == -1) { gui_chats_error(chats); return; }

	uiComboboxDelete(chats->gui.chats, idx);

	proto_ent_t* ent = chats->state.chats->vals;
	proto_ent_t* prev = NULL;
	
	while (idx > 0) { prev = ent; ent = ent->next; idx--; }
	const proto_id id = ent->id;

	LIST1_DELETE(chats->state.chats, prev, ent);
	chats->cb.leave(id, chats->cb.data); }

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

void gui_chats_del_acc(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	chats->cb.del_acc(0, chats->cb.data);
	gui_window_close(chats->wnd, true); }

void gui_chats_init(gui_chats_t* chats) {
	uiComboboxClear(chats->gui.chats);
	LIST_FOREACH(chats->state.chats->vals, chat)
		uiComboboxAppend(chats->gui.chats, chat->name); }

gui_chats_t* gui_chats(
	proto_ent1_t* users, proto_ent1_t* chats_,
	gui_chat_cb conn_cb, gui_chat_new_cb new_cb,
	gui_chat_cb leave_cb, gui_chat_cb del_acc_cb, void* data)
{
	gui_chats_t* chats = calloc(1, sizeof(gui_chats_t));
	chats->state.users = &users->vals;
	chats->state.chats = chats_;

	chats->cb.do_ = conn_cb; chats->cb.new_ = new_cb;
	chats->cb.leave = leave_cb; chats->cb.del_acc = del_acc_cb;
	chats->cb.data = data;

	uiGroup* grp = uiNewGroup("select a chat");
	uiBox *box = uiNewVerticalBox(),
		  *inner = uiNewVerticalBox();

	chats->gui.chats = uiNewCombobox();
	uiButton *do_ = uiNewButton("connect!"),
	         *leave = uiNewButton("leave chat"),
	         *del = uiNewButton("delete account");

	chats->gui.new_ = uiNewButton("new chat...");

	chats->wnd = gui_window_new(uiControl(box), free, chats);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(grp), true);
	uiBoxAppend(box, uiControl(chats->gui.new_), false);
	uiBoxAppend(box, uiControl(del), false);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));

	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(chats->gui.chats), false);
	uiBoxAppend(inner, uiControl(do_), false);
	uiBoxAppend(inner, uiControl(leave), false);

	uiButtonOnClicked(do_, gui_chats_do, chats);
	uiButtonOnClicked(chats->gui.new_, gui_chats_new, chats);
	uiButtonOnClicked(leave, gui_chats_leave, chats);
	uiButtonOnClicked(del, gui_chats_del_acc, chats);

	gui_chats_init(chats); return chats; }

void gui_chats_add_entry
(gui_chats_t* chats, bool user, const proto_ent_t* entry) {
	if (user && chats->child != NULL)
		gui_chats0_add_user(chats->child, entry);
	else uiComboboxAppend(chats->gui.chats, entry->name); }

void gui_chats_remove_user(gui_chats_t* chats, proto_id id) {
	if (chats->child != NULL)
		gui_chats0_remove_user(chats->child, id); }

void gui_chats_refresh(gui_chats_t* chats, bool user) {
	if (user && chats->child != NULL)
		gui_chats0_init(chats->child);
	else gui_chats_init(chats); }
