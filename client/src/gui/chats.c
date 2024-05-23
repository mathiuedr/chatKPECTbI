#include <stdint.h>
#include <stdlib.h>
#include <windows.h>
#include "gui.h"

void gui_chats_free(void* data) {
	gui_chats_t* chats = data;

	uiControlDestroy(uiControl(chats->wnd));
	proto_ent_free(chats->state.chats.vals);
	proto_ent_free(chats->state.users.vals);

	free(chats); }

void gui_chats_connect(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	ssize_t idx = uiComboboxSelected(chats->ui.chats);

	if (idx == -1) {
		gui_msg_box(
		    chats->wnd->wnd, "chat-client",
		    "please select a chat!", MB_ICONEXCLAMATION);

		return; }

	proto_ent_t* ent = chats->state.chats.vals;
	while (idx > 0) { ent = ent->next; idx--; }

	chats->conn(ent->id, chats->data);
	gui_chats_free(chats); }

void gui_chats_new_free(void* data) {
	gui_chats_t* chats = data;
	uiControlDestroy(uiControl(chats->wnd1));
	gui_check_free(chats->ui.users.vals); }

void gui_chats_new2(uiButton* _, void* data) {
	gui_chats_t* chats = data;
	gui_msg_box(
	    chats->wnd->wnd, "chat-client",
	    "TODO :p", MB_ICONERROR); }
	// gui_chats_new_free(chats); }

void gui_chats_new_check(gui_chats_t* chats, char* name) {
	gui_check1_t* chks = &chats->ui.users;
	uiCheckbox* chk = uiNewCheckbox(name);

	gui_check_t* chk1 = calloc(1, sizeof(gui_check_t));
	chk1->chk = chk; LIST1_PUSH(chks, chk1);

	uiBoxAppend(chks->box, uiControl(chk), false); }

void gui_chats_new1(uiButton* _, void* data) {
	gui_chats_t* chats = data;

	uiGroup* grp = uiNewGroup("invite users");
	uiBox* box = uiNewVerticalBox();
	chats->ui.users.box = uiNewVerticalBox();

	uiButton* new_ = uiNewButton("new chat!");
	chats->wnd1 = gui_window_new(
		"chat-client: new chat", uiControl(grp),
		gui_chats_new_free, chats);

	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(box));

	uiBoxSetPadded(chats->ui.users.box, true);
	LIST_FOREACH(chats->state.users.vals, user)
		gui_chats_new_check(chats, user->name);
	
	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(chats->ui.users.box), true);
	uiBoxAppend(box, uiControl(new_), false);

	uiButtonOnClicked(new_, gui_chats_new2, chats);
	gui_window_init(chats->wnd1, gui_chats_new_free, chats, false); }

gui_chats_t* gui_chats(
	gui_chat_connect_cb conn_cb,
	gui_chat_new_cb new_cb, void* data)
{
	gui_chats_t* chats = calloc(1, sizeof(gui_chats_t));
	chats->conn = conn_cb;
	chats->new_ = new_cb; chats->data = data;

	uiGroup* grp = uiNewGroup("select a chat");
	uiBox* box = uiNewVerticalBox();

	chats->ui.chats = uiNewCombobox();
	uiButton* conn = uiNewButton("connect!");
	uiButton* new_ = uiNewButton("new chat...");

	chats->wnd = gui_window_new(
		"chat-client: chats", uiControl(grp),
		gui_chats_free, chats);

	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(box));

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(chats->ui.chats), false);
	uiBoxAppend(box, uiControl(conn), false);
	uiBoxAppend(box, uiControl(new_), false);

	uiButtonOnClicked(conn, gui_chats_connect, chats);
	uiButtonOnClicked(new_, gui_chats_new1, chats);

	return chats; }

void gui_chats_add_chat
(gui_chats_t* chats, char* name, proto_id id) {
	proto_ent_t* ent = calloc(1, sizeof(proto_ent_t));
	ent->name = name; ent->id = id;

	LIST1_PUSH(&chats->state.chats, ent);
	uiComboboxAppend(chats->ui.chats, name); }

void gui_chats_add_user
(gui_chats_t* chats, char* name, proto_id id) {
	proto_ent_t* ent = calloc(1, sizeof(proto_ent_t));
	ent->name = name; ent->id = id;

	LIST1_PUSH(&chats->state.chats, ent);
	if (chats->wnd1 != NULL)
		gui_chats_new_check(chats, name); }

// this consumes the `name` arg
void gui_chats_add_entry(
	gui_chats_t* chats, bool user,
	char* name, proto_id id)
{
	if (!user) gui_chats_add_chat(chats, name, id);
	else gui_chats_add_user(chats, name, id); }
