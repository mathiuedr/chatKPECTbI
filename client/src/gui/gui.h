#if !defined(GUI_H)
#define GUI_H

#include "../../include/ui.h"
#include "../proto/proto.h"
#include <stdbool.h>
#include <stdint.h>

// gui.c

typedef struct gui_check_t {
	struct gui_check_t* next;
	uiCheckbox* chk; } gui_check_t;

typedef struct {
	uiBox* box; gui_check_t* vals;
	gui_check_t* end; } gui_check1_t;

typedef uiWindow gui_window;
typedef uiControl gui_ctrl;

typedef void (*gui_close_cb)(void* data);

typedef struct {
	gui_window* wnd; bool exit;
	gui_close_cb cb0; void* data0;
	gui_close_cb cb; void* data;
} gui_window_t;

bool gui_init(); void gui_cleanup();
void gui_run(); void gui_free_str(char* str);
void gui_check_free(gui_check_t* chk);

gui_window_t* gui_window_new(
    const char* title, gui_ctrl* ctrl,
    gui_close_cb close0_cb, void* data0);

void gui_window_init(
	gui_window_t* wnd, gui_close_cb close_cb,
	void* data, bool exit);

void gui_window_close(gui_window_t* wnd);

void gui_msg_box(
	gui_window* wnd, const char* title,
	const char* msg, uint32_t flags);

// login.c

typedef bool (*gui_login_cb)(
	gui_window* wnd, bool reg,
	char* uname, char* passwd,
	char* name, void* data);

gui_window_t* gui_login(gui_login_cb cb, void* data);

// chats.c

typedef bool (*gui_chat_connect_cb)
(proto_id chat, void* data);

typedef bool (*gui_chat_new_cb)
(char* name, proto_ids_t* ids, void* data);

typedef struct {
	gui_window_t *wnd, *wnd1;
	struct {
		uiCombobox* chats;
		gui_check1_t users; } ui;

	struct {
		proto_ent1_t chats;
		proto_ent1_t users; } state;

	gui_chat_connect_cb conn;
	gui_chat_new_cb new_; void* data;
} gui_chats_t;

gui_chats_t* gui_chats(
	gui_chat_connect_cb conn_cb,
	gui_chat_new_cb new_cb, void* data);

// this consumes the `name` arg
void gui_chats_add_entry(
    gui_chats_t* chats, bool user,
	char* name, proto_id id);

#endif
