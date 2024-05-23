#include <stdbool.h>
#include <time.h>
#include "gui.h"

void gui_chat_free0(void* data) {
	gui_chat_t* chat = data; chat->child = NULL;
	uiControlEnable(uiControl(chat->gui.invite)); }

void gui_chat_invite(uiButton* _, void* data) {
	gui_chat_t* chat = data;
	uiControlDisable(uiControl(chat->gui.invite));

	chat->child = gui_users(
		chat->state.base, false,
		chat->cb.invite, chat->cb.data);

	gui_window_init(
		chat->child->wnd, gui_chat_free0, chat); }

void gui_chat_send(uiButton* _, void* data) {
	gui_chat_t* chat = data;

	char* msg = uiEntryText(chat->gui.msg);
	if (strlen(msg) == 0) { gui_str_free(msg); return; }

	chat->cb.chat(msg, chat->cb.data); }

gui_chat_t* gui_chat(
    gui_chat_cb chat_cb,
    gui_users_cb invite_cb, void* data)
{
	gui_chat_t* chat = calloc(1, sizeof(gui_chat_t));
	chat->cb.chat = chat_cb;
	chat->cb.invite = invite_cb; chat->cb.data = data;

	uiGroup* grp = uiNewGroup("messages");
	uiBox *box = uiNewVerticalBox(),
	      *msgs = uiNewVerticalBox(),
	      *ctrls = uiNewHorizontalBox();

	chat->gui.msg = uiNewEntry();
	chat->gui.invite = uiNewButton("invite users...");
	uiButton *send = uiNewButton("send!");
	
	chat->wnd = gui_window_new(uiControl(box), free, chat);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(grp), true);
	uiBoxAppend(box, uiControl(ctrls), false);

	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(msgs));

	uiBoxSetPadded(msgs, true);
	uiBoxSetPadded(ctrls, true);

	uiBoxAppend(ctrls, uiControl(chat->gui.invite), true);
	uiBoxAppend(ctrls, uiControl(chat->gui.msg), true);
	uiBoxAppend(ctrls, uiControl(send), false);

	uiButtonOnClicked(chat->gui.invite, gui_chat_invite, data);
	uiButtonOnClicked(send, gui_chat_send, data);
	
	return chat; }

void gui_chat_add(gui_chat_t* chat, proto_msg_t* msgs) {
	LIST_FOREACH(msgs, msg) {
		uiBox *top = uiNewHorizontalBox(),
		      *msg1 = uiNewVerticalBox();

		char date[18] = {};
		time_t time = (time_t)(msg->time / 1000);

		strftime(
			date, sizeof(date),
			"dd.mm.yy hh:mm:ss", gmtime(&time));
		
		uiBoxSetPadded(top, true);
		uiBoxSetPadded(msg1, true);
		
		uiBoxAppend(top, uiControl(uiNewLabel(msg->uname)), false);
		uiBoxAppend(top, uiControl(uiNewLabel(date)), false);

		uiBoxAppend(msg1, uiControl(top), false);
		uiBoxAppend(msg1, uiControl(uiNewLabel(msg->msg)), false);
		
		uiBoxAppend(chat->gui.msgs, uiControl(msg1), false); }

	proto_msg_free(msgs); }

void gui_chat_mask(gui_chat_t* chat) {
	LIST_FOREACH(chat->state.mask->vals, id)
		gui_users_remove(chat->child, id->id); }

void gui_chat_join(gui_chat_t* chat, proto_id id) {
	proto_ent1_t* mask = chat->state.mask;
	size_t _idx; proto_ent_t *ent0, *ent;

	LIST_FIND(mask->vals, id, _idx, ent0, ent);
	if (ent) {
		LIST1_DELETE(mask, ent0, ent);
		if (chat->child != NULL)
			gui_users_remove(chat->child, ent->id); } }

void gui_chat_leave(gui_chat_t* chat, proto_ent_t* ent) {
	LIST1_PUSH(chat->state.mask, ent);
	if (chat->child != NULL)
		gui_users_add(chat->child, ent); }
