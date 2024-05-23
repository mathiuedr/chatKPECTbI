#if !defined(GUI_H)
#define GUI_H

#include "../../include/ui.h"
#include "../proto/proto.h"
#include <stdbool.h>
#include <stdint.h>

typedef uiWindow gui_window;
typedef uiControl gui_ctrl;

bool gui_init(); void gui_run(); void gui_cleanup();
void gui_str_free(char* str);

typedef void (*gui_close_cb)(void* data);
void gui_quit_cb(void* data);

typedef struct {
	gui_window* wnd;
	struct { gui_close_cb fn; void* data; } close, exit;
} gui_window_t;

gui_window_t* gui_window_new
(gui_ctrl* ctrl, gui_close_cb close_cb, void* data);

void gui_window_init
(gui_window_t* wnd, gui_close_cb exit_cb, void* data);

void gui_window_resize(gui_window_t* wnd);
void gui_window_close(gui_window_t* wnd, bool exit);

typedef struct gui_check_t {
	struct gui_check_t* next;
	uiCheckbox* chk; proto_id id; } gui_check_t;

typedef struct {
	gui_check_t* vals; gui_check_t* end;
} gui_check1_t;

gui_check_t* gui_check_new(uiCheckbox* chk, proto_id id);
void gui_check_free(gui_check_t* chk);

void gui_msg_box(
	gui_window* wnd, const char* title,
	const char* msg, uint32_t flags);

typedef bool (*gui_auth_cb)(
	gui_window* wnd, char* uname,
	char* passwd, char* name, void* data);

gui_window_t* gui_auth(gui_auth_cb cb, void* data);

typedef void (*gui_chat_cb)(proto_id chat, void* data);

typedef void (*gui_chat_new_cb)
(char* name, proto_id_t* ids, void* data);

typedef struct {
	gui_window_t* wnd; uiEntry* title;
	struct {
		uiBox *box0, *box;
		gui_check1_t chks; } gui;

	proto_ent_t** state;
	gui_chat_new_cb cb; void* data;
} gui_chats0_t;

gui_chats0_t* gui_chats0
(proto_ent_t** users, gui_chat_new_cb cb, void* data);

void gui_chats0_add_user
(gui_chats0_t* chats0, const proto_ent_t* ent);

void gui_chats0_remove_user
(gui_chats0_t* chats0, proto_id id);

void gui_chats0_init(gui_chats0_t* chats);

typedef struct {
	gui_window_t* wnd;
	gui_chats0_t* child;

	struct {
		uiCombobox* chats;
		uiButton* new_; } gui;

	struct {
		proto_ent_t** users;
		proto_ent1_t* chats; } state;

	struct {
		gui_chat_cb do_, leave, del_acc;
		gui_chat_new_cb new_;
		void* data; } cb;
} gui_chats_t;

gui_chats_t* gui_chats(
	proto_ent1_t* users, proto_ent1_t* chats_,
	gui_chat_cb conn_cb, gui_chat_new_cb new_cb,
	gui_chat_cb leave_cb, gui_chat_cb del_acc_cb, void* data);

void gui_chats_add_entry(
	gui_chats_t* chats, bool user,
	const proto_ent_t* ent);

void gui_chats_remove_user(gui_chats_t* chats, proto_id id);
void gui_chats_refresh(gui_chats_t* chats, bool user);

#endif
