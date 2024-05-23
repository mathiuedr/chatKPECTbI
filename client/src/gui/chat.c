#include <stdbool.h>
#include <time.h>
#include "gui.h"

void gui_chat_free0(void* data) {
	gui_chat_t* chat = data; chat->child = NULL;
	uiControlEnable(uiControl(chat->gui.invite)); }

void gui_chat_mask(gui_chat_t* chat) {
	LIST_FOREACH(chat->state.mask.vals, id)
		gui_users_remove(chat->child, id->id); }

void gui_chat_invite(uiButton* _, void* data) {
	gui_chat_t* chat = data;
	uiControlDisable(uiControl(chat->gui.invite));

	chat->child = gui_users(
		chat->state.base, false,
		chat->cb.invite, chat->cb.data);

	gui_chat_mask(chat);
	gui_window_init(
		chat->child->wnd, gui_chat_free0, chat); }

void gui_chat_send(uiButton* _, void* data) {
	gui_chat_t* chat = data;

	char* msg = uiEntryText(chat->gui.msg);
	if (strlen(msg) == 0) { gui_str_free(msg); return; }

	uiEntrySetText(chat->gui.msg, "");
	chat->cb.chat(msg, chat->cb.data); }

void gui_chat_free(void* data) {
	gui_chat_t* chat = data;
	proto_id_free(chat->state.mask.vals);
	free(chat); }

gui_chat_t* gui_chat(
	proto_ent_t** users,
	gui_chat_cb chat_cb,
	gui_users_cb invite_cb, void* data)
{
	gui_chat_t* chat = calloc(1, sizeof(gui_chat_t));
	chat->state.base = users;

	chat->cb.chat = chat_cb;
	chat->cb.invite = invite_cb; chat->cb.data = data;

	uiGroup* grp = uiNewGroup("messages");
	uiBox *box = uiNewVerticalBox(),
	      *ctrls = uiNewHorizontalBox();
	      
	chat->gui.msg = uiNewEntry();
	chat->gui.msgs = uiNewVerticalBox();
	chat->gui.invite = uiNewButton("invite...");
	uiButton *send = uiNewButton("send!");
	
	chat->wnd = gui_window_new(
	    uiControl(box), gui_chat_free, chat);

	uiBoxSetPadded(box, true);
	uiBoxAppend(box, uiControl(grp), true);
	uiBoxAppend(box, uiControl(ctrls), false);

	uiGroupSetMargined(grp, true);
	uiGroupSetChild(grp, uiControl(chat->gui.msgs));

	uiBoxSetPadded(chat->gui.msgs, true);
	uiBoxSetPadded(ctrls, true);

	uiBoxAppend(ctrls, uiControl(chat->gui.invite), false);
	uiBoxAppend(ctrls, uiControl(chat->gui.msg), true);
	uiBoxAppend(ctrls, uiControl(send), false);

	uiButtonOnClicked(chat->gui.invite, gui_chat_invite, chat);
	uiButtonOnClicked(send, gui_chat_send, chat);
	
	return chat; }

void gui_chat_add(gui_chat_t* chat, proto_msg_t* msgs) {
	LIST_FOREACH(msgs, msg) {
		uiBox *top = uiNewHorizontalBox(),
		      *msg1 = uiNewVerticalBox();

		char date[18] = {};
		time_t time = (time_t)(msg->time / 1000);

		strftime(
			date, sizeof(date),
			"%d.%m.%y %H:%M:%S", gmtime(&time));
		
		uiBoxSetPadded(top, true);
		uiBoxAppend(top, uiControl(uiNewLabel(msg->uname)), false);
		uiBoxAppend(top, uiControl(uiNewLabel(date)), false);

		uiBoxAppend(msg1, uiControl(top), false);
		uiBoxAppend(msg1, uiControl(uiNewLabel(msg->msg)), false);
		
		uiBoxAppend(chat->gui.msgs, uiControl(msg1), false); }

	proto_msg_free(msgs); }

void gui_chat_refresh(gui_chat_t* chat) {
	if (chat->child != NULL) {
		gui_users_init(chat->child); gui_chat_mask(chat); } }

void gui_chat_join(gui_chat_t* chat, proto_id_t* ids) {
	LIST_FOREACH(ids, id) {
		LIST1_PUSH(&chat->state.mask, id);
		if (chat->child != NULL)
			gui_users_remove(chat->child, id->id); } }

void gui_chat_leave(gui_chat_t* chat, proto_ent_t* ent_) {
	proto_id1_t* mask = &chat->state.mask;
	size_t _idx; proto_id_t *ent0, *ent;

	LIST_FIND(mask->vals, ent_->id, _idx, ent0, ent);
	if (ent) {
		LIST1_DELETE(mask, ent0, ent);
		if (chat->child != NULL)
			gui_users_add(chat->child, ent_); } }
