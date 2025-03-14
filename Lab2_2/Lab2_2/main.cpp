#include "appSpace.h"
#include <windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR command_line, int show_command)
{
	app my_app{ hInstance };
	my_app.run(show_command);
	return 0;
	//return 0;
}