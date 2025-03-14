
#include "app.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR command_line, int show_command)
{
	app my_app{ hInstance };
	return my_app.run(show_command);
}