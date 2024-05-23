#include <stdint.h>
#include <stdlib.h>
#include "gui.h"

void gui_menu_error(gui_menu_t* menu) {
	gui_msg_box(
		menu->wnd->wnd, "chat-client",
		"please select a chat!", MB_ICONEXCLAMATION); }

void gui_menu_connect(uiButton* _, void* data) {
	gui_menu_t* menu = data;
	ssize_t idx = uiComboboxSelected(menu->gui.chats);
	if (idx == -1) { gui_menu_error(menu); return; }

	const proto_ent_t* ent = menu->state.chats->vals;
	while (idx > 0) { ent = ent->next; idx--; }

	menu->cb.conn(ent->id, menu->cb.data);
	gui_window_close(menu->wnd, false); }

void gui_menu_leave(uiButton* _, void* data) {
	gui_menu_t* menu = data;
	ssize_t idx = uiComboboxSelected(menu->gui.chats);
	if (idx == -1) { gui_menu_error(menu); return; }

	uiComboboxDelete(menu->gui.chats, idx);

	proto_ent_t* ent = menu->state.chats->vals;
	proto_ent_t* prev = NULL;
	
	while (idx > 0) { prev = ent; ent = ent->next; idx--; }
	const proto_id id = ent->id;

	LIST1_DELETE(menu->state.chats, prev, ent);
	menu->cb.leave(id, menu->cb.data); }

void gui_menu_free0(void* data) {
	gui_menu_t* menu = data; menu->child = NULL;
	uiControlEnable(uiControl(menu->gui.new_)); }

void gui_menu_new(uiButton* _, void* data) {
	gui_menu_t* menu = data;
	uiControlDisable(uiControl(menu->gui.new_));
	
	menu->child = gui_users(
		menu->state.users, true,
		menu->cb.new_, menu->cb.data);

	gui_window_init(
		menu->child->wnd, gui_menu_free0, menu); }

void gui_menu_del_acc(uiButton* _, void* data) {
	gui_menu_t* menu = data;
	menu->cb.del_acc(0, menu->cb.data);
	gui_window_close(menu->wnd, true); }

void gui_menu_init(gui_menu_t* menu) {
	uiComboboxClear(menu->gui.chats);
	LIST_FOREACH(menu->state.chats->vals, chat)
		uiComboboxAppend(menu->gui.chats, chat->name); }

gui_menu_t* gui_menu(
	proto_ent1_t* users, proto_ent1_t* chats_,
	gui_menu_cb conn_cb, gui_users_cb new_cb,
	gui_menu_cb leave_cb, gui_menu_cb del_acc_cb, void* data)
{
	gui_menu_t* menu = calloc(1, sizeof(gui_menu_t));
	menu->state.users = &users->vals;
	menu->state.chats = chats_;

	menu->cb.conn = conn_cb; menu->cb.new_ = new_cb;
	menu->cb.leave = leave_cb; menu->cb.del_acc = del_acc_cb;
	menu->cb.data = data;

	uiGroup* grp = uiNewGroup("select a chat");
	uiBox *box = uiNewVerticalBox(),
		  *inner = uiNewVerticalBox();

	menu->gui.chats = uiNewCombobox();
	uiButton *conn = uiNewButton("connect!"),
	         *leave = uiNewButton("leave chat"),
	         *del = uiNewButton("delete account");

	menu->gui.new_ = uiNewButton("new chat...");

	menu->wnd = gui_window_new(uiControl(box), free, menu);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(grp), true);
	uiBoxAppend(box, uiControl(menu->gui.new_), false);
	uiBoxAppend(box, uiControl(del), false);
	
	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(inner));

	uiBoxSetPadded(inner, true);
	uiBoxAppend(inner, uiControl(menu->gui.chats), false);
	uiBoxAppend(inner, uiControl(conn), false);
	uiBoxAppend(inner, uiControl(leave), false);

	uiButtonOnClicked(conn, gui_menu_connect, menu);
	uiButtonOnClicked(menu->gui.new_, gui_menu_new, menu);
	uiButtonOnClicked(leave, gui_menu_leave, menu);
	uiButtonOnClicked(del, gui_menu_del_acc, menu);

	gui_menu_init(menu); return menu; }

void gui_menu_add
(gui_menu_t* menu, bool user, const proto_ent_t* entry) {
	if (user && menu->child != NULL)
		gui_users_add(menu->child, entry);
	else if (!user)
		uiComboboxAppend(menu->gui.chats, entry->name); }

void gui_menu_refresh(gui_menu_t* menu, bool user) {
	if (user && menu->child != NULL)
		gui_users_init(menu->child);
	else if (!user) gui_menu_init(menu); }
