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

typedef void (*gui_users_cb)
(char* name, proto_id_t* ids, void* data);

typedef void (*gui_menu_cb)(proto_id id, void* data);
typedef void (*gui_dummy_cb)(void* data);

typedef struct {
	gui_window_t* wnd; uiEntry* title;
	struct {
		uiBox *box0, *box;
		gui_check1_t chks; } gui;

	proto_ent_t** state;
	gui_users_cb cb; void* data;
} gui_users_t;

gui_users_t* gui_users(
	proto_ent_t** users, bool title,
	gui_users_cb cb, void* data);

void gui_users_add(gui_users_t* users, const proto_ent_t* ent);
void gui_users_remove(gui_users_t* users, proto_id id);
void gui_users_init(gui_users_t* users);

typedef struct {
	gui_window_t* wnd;
	gui_users_t* child;

	struct {
		uiCombobox* chats;
		uiButton* new_; } gui;

	struct {
		proto_ent_t** users;
		proto_ent1_t* chats; } state;

	struct {
		gui_menu_cb conn, leave, del_acc;
		gui_users_cb new_; void* data; } cb;
} gui_menu_t;

gui_menu_t* gui_menu(
	proto_ent1_t* users, proto_ent1_t* chats_,
	gui_menu_cb conn_cb, gui_users_cb new_cb,
	gui_menu_cb leave_cb, gui_menu_cb del_acc_cb, void* data);

void gui_menu_add
(gui_menu_t* menu, bool user, const proto_ent_t* ent);

void gui_menu_refresh(gui_menu_t* menu, bool user);

typedef void (*gui_chat_cb)(const char* str, void* data);

typedef struct {
	gui_window_t* wnd; gui_users_t* child;
	struct {
		uiBox* msgs; uiEntry* msg;
		uiButton* invite; } gui;
	
	struct {
		proto_ent_t** base;
		proto_ent1_t* mask; } state;

	struct {
		gui_chat_cb chat;
		gui_users_cb invite;
		void* data; } cb;
} gui_chat_t;

gui_chat_t* gui_chat(
    gui_chat_cb chat_cb,
    gui_users_cb invite_cb, void* data);

void gui_chat_add(gui_chat_t* chat, proto_msg_t* msgs);
void gui_chat_mask(gui_chat_t* chat);

void gui_chat_join(gui_chat_t* chat, proto_id id);
void gui_chat_leave(gui_chat_t* chat, proto_ent_t* ent);

#endif
