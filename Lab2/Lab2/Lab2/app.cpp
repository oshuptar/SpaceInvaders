#include "app.h"
#include <stdexcept>

std::wstring const app::s_class_name = L"Space Invaders Window Class";

app::app(HINSTANCE instance) : app_instance{ instance }, main_window{},
screen_size{GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)},
enemy_brush{CreateSolidBrush(RGB(70,70,255))},
player_brush{CreateSolidBrush(RGB(255, 0, 0))}
{
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN;
	DWORD ex_style = WS_EX_LAYERED | WS_EX_TOPMOST;

	/*DWORD enemy_style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;
	DWORD player_style = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS;*/

	register_class();
	main_window = create_window(style, ex_style);
	enemy_window = create_child_window(50, 40, window_height/6);
	player_window = create_child_window(50, 50, window_height - window_height/4);

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
	SetTimer(main_window,1, 50, nullptr);
	ShowWindow(main_window, show_window);
	ShowWindow(player_window, SW_SHOWNA);
	ShowWindow(enemy_window, SW_SHOWNA);
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


HWND app::create_window(DWORD style, DWORD ex_style, HWND parent)
{
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
		parent,
		nullptr,
		app_instance,
		this);

	return window;
}

HWND app::create_child_window(int width, int height, int y)
{
	return CreateWindowExW(0,
		L"STATIC",
		nullptr,
		WS_CHILD | WS_CLIPSIBLINGS,
		window_width/2 - width/2, y,
		width, height,
		main_window,
		nullptr,
		app_instance,
		nullptr);

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

void app::move_enemy_window()
{
	static int enemy_window_direction = 1; // 1 signs - right; 0 - signs left
	static int right_bound = window_width / 2 - 50/2 + 25;
	static int left_bound = window_width / 2 - 50/2 - 25;

	RECT enemy_rc;
	GetWindowRect(enemy_window, &enemy_rc);

	POINT client_area_coordinates{ enemy_rc.left, enemy_rc.top };
	ScreenToClient(main_window, &client_area_coordinates);

	if (client_area_coordinates.x >= right_bound)
		enemy_window_direction--;
	else if(client_area_coordinates.x <= left_bound)
		enemy_window_direction++;

	if (enemy_window_direction == 1)
		client_area_coordinates.x++;

	else if (enemy_window_direction == 0)
		client_area_coordinates.x--;

	int width = enemy_rc.right - enemy_rc.left;
	int height = enemy_rc.bottom - enemy_rc.top;
	MoveWindow(enemy_window, client_area_coordinates.x, client_area_coordinates.y, width, height, true);
}

void app::move_player_window(int direction)
{
	//1 is for right; 0 for left
	RECT player_rc;
	POINT new_pos;

		if (direction == 1) {
			GetWindowRect(player_window, &player_rc);

			/*POINT client_coordinates{ player_rc.left, player_rc.top };
			ScreenToClient(main_window, &client_coordinates)*/;

			new_pos.x = player_rc.left + 10;
			new_pos.y = player_rc.top;

			ScreenToClient(main_window, &new_pos); // converts creen coordinated relative to the window

			int player_width = player_rc.right - player_rc.left;

			if (new_pos.x + player_width < window_width - 10 - 10) // What is the problem here??
			{
				MoveWindow(player_window, new_pos.x, new_pos.y,
					player_width,
					player_rc.bottom - player_rc.top,
					true); //last parameter is responsible for repainting
			}
		}
		else if (direction == 0) {
			GetWindowRect(player_window, &player_rc);

			new_pos.x = player_rc.left - 10;
			new_pos.y = player_rc.top;

			ScreenToClient(main_window, &new_pos); // converts creen coordinated relative to the window

			if (new_pos.x > 10)
			{
				MoveWindow(player_window, new_pos.x, new_pos.y,
					player_rc.right - player_rc.left,
					player_rc.bottom - player_rc.top,
					true); //last parameter is responsible for repainting
			}
		}
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
	case WM_SETFOCUS:
		SetLayeredWindowAttributes(main_window, 0, 255, LWA_ALPHA);
		return DefWindowProc(window, message, wparam, lparam);
	case WM_KILLFOCUS:
		SetLayeredWindowAttributes(main_window, 0, 255 * 0.4 , LWA_ALPHA);
		return DefWindowProc(window, message, wparam, lparam);
	case WM_TIMER:
		move_enemy_window();
		break;
	case WM_KEYDOWN:
		switch (wparam)
		{
			case VK_LEFT:
				move_player_window(0);
				break;
			case VK_RIGHT:
				move_player_window(1);
				break;
			case VK_SPACE:
				create_bullets();
				break;
		}
		break;
	case WM_CTLCOLORSTATIC:
		HWND hwndStatic;
		hwndStatic = reinterpret_cast<HWND>(lparam);
		if (hwndStatic == enemy_window)
			return reinterpret_cast<INT_PTR>(enemy_brush);
		else if (hwndStatic == player_window)
			return reinterpret_cast<INT_PTR>(player_brush);
		else
			return reinterpret_cast<INT_PTR>(CreateSolidBrush(RGB(0, 0, 0)));
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
}

app::~app()
{
	DeleteObject(player_brush);
	DeleteObject(enemy_brush);
	KillTimer(main_window, 1);
}
