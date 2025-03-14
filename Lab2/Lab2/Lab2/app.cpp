#include "app.h"
#include <stdexcept>

std::wstring const app::s_class_name = L"Space Invaders Window Class";

app::app(HINSTANCE instance) : app_instance{ instance }, main_window{},
screen_size{GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)}
{
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER;
	DWORD ex_style = WS_EX_LAYERED | WS_EX_TOPMOST;
	register_class();
	main_window = create_window(style, ex_style);
	SetLayeredWindowAttributes(main_window, 0, 255, LWA_ALPHA);
}

bool app::register_class()
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(app_instance, s_class_name.c_str(), &desc) != 0)
		return true;

	desc.cbSize = sizeof(WNDCLASSEXW);
	desc.cbWndExtra = sizeof(LONG_PTR);
	desc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.hInstance = app_instance;
	desc.lpfnWndProc = static_window_proc;
	desc.lpszClassName = s_class_name.c_str();

	return RegisterClassExW(&desc) != 0;
}

int app::run(int show_window)
{
	ShowWindow(main_window, show_window);

	MSG msg{};
	BOOL result = false;

	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1)
			return EXIT_FAILURE;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


HWND app::create_window(DWORD style, DWORD ex_style)
{
	int window_width = 800;
	int window_height = 600;

	POINT window_center
	{
		.x = (screen_size.x / 2) - (window_width / 2),
		.y = (screen_size.y / 2) - (window_height / 2),
	};

	HWND window = CreateWindowExW(
		ex_style,
		s_class_name.c_str(),
		L"Space Invaders",
		style,
		window_center.x, window_center.y,
		window_width, window_height,
		nullptr,
		nullptr,
		app_instance,
		this);
		
	return window;
}

LRESULT app::static_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	app* my_app = nullptr;
	if (message == WM_NCCREATE) {
		auto p = reinterpret_cast<LPCREATESTRUCTW>(lparam);
		my_app = static_cast<app*>(p->lpCreateParams); 
		SetWindowLongPtrW(window, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(my_app));
	}
	else
	{
		my_app = reinterpret_cast<app*>(GetWindowLongPtrW(window, GWLP_USERDATA));
	}
	if (my_app != nullptr)
	{
		return my_app->window_proc(window, message, wparam, lparam);
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT app::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{

	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		DefWindowProc(window, message, wparam, lparam);
	}
}
