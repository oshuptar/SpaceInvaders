#include "app.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prev_instance,
	LPWSTR command_line, int show_command)
{
	app my_app{ instance };
	return my_app.run(show_command);
}