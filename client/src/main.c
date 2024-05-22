#include "gui/gui.h"
#include <stdbool.h>

void login_cb(char* a, char* b, char* c, void* d) {}

int main() {
	gui_init();
	gui_window* wnd = gui_login(login_cb, NULL);

	gui_window_init(wnd, NULL, NULL, true); gui_run();
	gui_cleanup(); return 0; }
