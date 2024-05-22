#if !defined(GUI_H)
#define GUI_H

#include "../../deps/ui.h"
#include <stdbool.h>

// gui.c

typedef uiWindow gui_window;
typedef uiControl gui_ctrl;

typedef void (*gui_close_cb)(void* data);

bool gui_init(); void gui_cleanup();
void gui_run(); void gui_free_str(char* str);

gui_window* gui_window_new(const char* title, gui_ctrl* ctrl);

void gui_window_init(
    gui_window* wnd, gui_close_cb close_cb,
    void* data, bool exit);

// login.c

typedef bool (*gui_login_cb)(
    gui_window* wnd, bool reg,
    char* uname, char* passwd,
    char* name, void* data);

gui_window* gui_login(gui_login_cb cb, void* data);

#endif
