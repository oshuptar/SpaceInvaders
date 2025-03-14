#pragma once
#include <stdlib.h>
#include <windows.h>
#include <string>

class app
{
private:
	static std::wstring const s_class_name;
	HWND main_window;
	POINT screen_size;
	HINSTANCE app_instance;

	bool register_class();
	HWND create_window(DWORD style, DWORD ex_style);

	static LRESULT CALLBACK static_window_proc(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	LRESULT window_proc(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

public:
	app(HINSTANCE instance);
	int run(int show_window);
};