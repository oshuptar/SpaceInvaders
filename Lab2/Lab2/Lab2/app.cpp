#include "app.h"
#include <stdexcept>
#include "resource.h"

//By default, Alt + F4 works on your window because the Windows operating system sends a WM_CLOSE message when the user presses Alt + F4.Your window automatically processes this message unless you override it.

std::wstring const app::s_class_name = L"Space Invaders Window Class";
std::wstring const app::s_enemy_class_name = L"Enemy";
std::wstring const app::s_player_class_name = L"Player";

COLORREF color_to_exclude = RGB(255, 0, 255);

int app::enemy_window_direction = 1;

app::app(HINSTANCE instance) : app_instance{ instance }, main_window{},
screen_size{GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)},
enemy_brush{CreateSolidBrush(RGB(70,70,255))},
player_brush{ CreateSolidBrush(RGB(255, 0, 0)) },
background_brush{CreateSolidBrush(RGB(255, 255, 255))},
bullet_brush{ CreateSolidBrush(RGB(0, 0, 0)) },
plane_bitmap{LoadBitmapW(app_instance, MAKEINTRESOURCEW(ID_PLANE_BITMAP))},
enemy_bitmap{LoadBitmapW(app_instance, MAKEINTRESOURCEW(ID_ENEMIES_BITMAP))}
{
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER;// | WS_CLIPCHILDREN;
	DWORD ex_style = WS_EX_LAYERED | WS_EX_TOPMOST;

	register_class();
	register_class_enemy();
	register_class_player();

	main_window = create_window(style, ex_style);
	/*enemy_window = create_child_window(enemy_width, enemy_height, window_width/2 - enemy_width/2, window_height/6);
	player_window = create_child_window(player_width, player_height, window_width/2 - player_width/2 , window_height - window_height / 4 - player_height);*/
	DWORD ex_style_other = 0;
	DWORD style_other = WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE;// | SS_BITMAP;

	enemy_window = create_window_other(style_other,
		ex_style_other, window_width / 2 - enemy_width / 2,
		window_height / 6, enemy_width, enemy_height, s_enemy_class_name.c_str(),
		main_window);
	player_window = create_window_other(style_other,
		ex_style_other, window_width / 2 - player_width / 2, 
		window_height - window_height / 4 - player_height, player_width, player_height, 
		s_player_class_name.c_str(),
		main_window);

	SetLayeredWindowAttributes(main_window, 0, 255, LWA_ALPHA);

}

bool app::register_class()
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(app_instance, s_class_name.c_str(), &desc) != 0)
		return true;

	desc.cbSize = sizeof(WNDCLASSEXW);
	desc.cbWndExtra = sizeof(LONG_PTR);
	desc.hbrBackground = background_brush;
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.hInstance = app_instance;
	desc.lpfnWndProc = static_window_proc;
	desc.lpszClassName = s_class_name.c_str();
	desc.lpszMenuName = MAKEINTRESOURCEW(ID_MENU);
	desc.hIcon = LoadIconW(app_instance, MAKEINTRESOURCEW(ID_ICON));

	return RegisterClassExW(&desc) != 0;
}

bool app::register_class_player()
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(app_instance, s_player_class_name.c_str(), &desc) != 0)
		return true;

	desc.cbSize = sizeof(WNDCLASSEXW);
	desc.cbWndExtra = sizeof(LONG_PTR);
	desc.hbrBackground = player_brush;
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.hInstance = app_instance;
	desc.lpfnWndProc = static_window_proc_player;
	desc.lpszClassName = s_player_class_name.c_str();
	desc.lpszMenuName = nullptr;
	desc.hIcon = nullptr;
	//desc.style = CS_HREDRAW | CS_VREDRAW;

	return RegisterClassExW(&desc) != 0;
}

bool app::register_class_enemy()
{
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(app_instance, s_enemy_class_name.c_str(), &desc) != 0)
		return true;

	desc.cbSize = sizeof(WNDCLASSEXW);
	desc.cbWndExtra = sizeof(LONG_PTR);
	desc.hbrBackground = nullptr;//enemy_brush;
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.hInstance = app_instance;
	desc.lpfnWndProc = static_window_proc_enemy;
	desc.lpszClassName = s_enemy_class_name.c_str();
	desc.lpszMenuName = nullptr;
	desc.hIcon = nullptr;
	//desc.style = CS_HREDRAW | CS_VREDRAW;

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
	HMENU menu = GetMenu(main_window);
	MENUINFO menu_info;

	GetMenuInfo(menu, &menu_info);

	menu_info.fMask |= MIM_STYLE;
	menu_info.dwStyle |= MNS_AUTODISMISS | MNS_CHECKORBMP;
	SetMenuInfo(menu, &menu_info);

	HACCEL shortcuts = LoadAcceleratorsW(app_instance, MAKEINTRESOURCEW(ID_SHORTCUTS));
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1)
			return EXIT_FAILURE;

		// If non-zero then the accelerators are processed
		if (TranslateAcceleratorW(msg.hwnd, shortcuts, &msg) == 0) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

HWND app::create_window_other(DWORD style, DWORD ex_style, int x, int y, int width, int height, LPCWSTR class_name ,HWND parent)
{
	
	HWND window = CreateWindowExW(
		ex_style,
		class_name,
		nullptr,
		style,
		x, y,
		width, height,
		parent,
		nullptr,
		app_instance,
		this);

	return window;
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
		LoadMenuW(app_instance, MAKEINTRESOURCEW(ID_MENU)),
		app_instance,
		this);

	return window;
}

HWND app::create_child_window(int width, int height,int x, int y)
{
	return CreateWindowExW(
		0,
		L"STATIC",
		nullptr,
		WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
		x, y,
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

LRESULT app::static_window_proc_player(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
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
		return my_app->window_proc_player(window, message, wparam, lparam);
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT app::static_window_proc_enemy(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
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
		return my_app->window_proc_enemy(window, message, wparam, lparam);
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT app::window_proc_player(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HDC dc;
	PAINTSTRUCT ps;
	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT:
		dc = BeginPaint(window, &ps);;
		render_plane_sprites(dc);
		EndPaint(window, &ps);
		return 0;
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
}

LRESULT app::window_proc_enemy(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	HDC dc;
	PAINTSTRUCT ps;
	switch (message)
	{
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_PAINT:
		dc = BeginPaint(window, &ps);;
		render_enemy_sprites(dc);
		EndPaint(window, &ps);
		return 0;
	default:
		return DefWindowProc(window, message, wparam, lparam);
	}
}

void app::move_enemy_window()
{
	int right_bound = window_width / 2 - enemy_width/2 + 25;
	int left_bound = window_width / 2 - enemy_width/2 - 25;

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

	GetClientRect(enemy_window,&enemy_rc);

	//InvalidateRect(main_window, &enemy_rc , true);//changed from false to true
}

void app::move_player_window(int direction)
{
	//1 is for right; 0 for left
	RECT player_rc;
	RECT main_rc;
	POINT new_pos;
	GetWindowRect(player_window, &player_rc);

	new_pos.x = player_rc.left;
	new_pos.y = player_rc.top;

	ScreenToClient(main_window, &new_pos); // converts creen coordinated relative to the window

	if (direction == 1) {
		new_pos.x = new_pos.x + 10;

		GetClientRect(main_window, &main_rc);

		if (new_pos.x + player_width < main_rc.right - main_rc.left - 10) // What is the problem here??
		{
			MoveWindow(player_window, new_pos.x, new_pos.y,
					player_width,
					player_height,
					true); //last parameter is responsible for repainting
		}
	}
	else if (direction == 0) {

		new_pos.x = new_pos.x - 10;

		if (new_pos.x > 10)
		{
			MoveWindow(player_window, new_pos.x, new_pos.y,
					player_width,
					player_height,
					true); //last parameter is responsible for repainting
		}
	}

	InvalidateRect(main_window, nullptr, false); 
}

void app::update_score()
{
	std::wstring score_text; // initialisation bypasses case label fix
	HDC dc = GetDC(main_window);
	SetBkMode(dc, TRANSPARENT);

	score_text = L"Score: " + std::to_wstring(score);
	TextOutW(dc, 10, 10, score_text.c_str(), score_text.length());

	ReleaseDC(main_window, dc);
}

void app::create_bullets()
{
	if (std::chrono::system_clock::now() - last_bullet_time < std::chrono::milliseconds(150))
		return;

	RECT player_rc;
	GetWindowRect(player_window, &player_rc);

	POINT client_coordinates{ player_rc.left, player_rc.top };
	ScreenToClient(main_window, &client_coordinates);

	int x = client_coordinates.x + player_width / 2 - bullet_width / 2;
	int y = client_coordinates.y;
	HWND bullet_window = create_child_window(bullet_width, bullet_height, x, y);
	bullets.push_back(bullet(bullet_window, x, y));

	last_bullet_time = std::chrono::system_clock::now();
}

void app::move_bullets()
{
	RECT bullet_rc, enemy_rc, result_rc;
	for (int i = 0; i < bullets.size(); i++)
	{
		bullets[i].y -= 15;

		GetWindowRect(bullets[i].bullet_window, &bullet_rc);
		GetWindowRect(enemy_window, &enemy_rc);

		if (bullets[i].y + bullet_height <= 0)
		{
			DestroyWindow(bullets[i].bullet_window);
			bullets.erase(bullets.begin() + i); // retrives the iterator and removes the bullet from the position i from the vector
			i--; // because all of the values are shifted
		}
		else if (IntersectRect(&result_rc, &bullet_rc, &enemy_rc) != 0)
		{
			score++;
			DestroyWindow(bullets[i].bullet_window);
			bullets.erase(bullets.begin() + i);
			i--;

			RECT score_rect = { 10, 10, 50, 50 };  
			InvalidateRect(main_window, &score_rect, true); // ensures that we erase the previous result
			InvalidateRect(main_window, nullptr, false); // fix this
			//update_score();
		}
		else
			MoveWindow(bullets[i].bullet_window, bullets[i].x, bullets[i].y, bullet_width, bullet_height, true);

	}
}

void app::on_command(WORD id)
{
	switch (id)
	{
	case ID_ABOUT:
		break;
	case ID_EXIT:
		DestroyWindow(main_window);
		break;
	case ID_NEW_GAME:
		reset_game();
		break;
	case ID_SMALL_SIZE:
		resize_window(0.8);
		break;
	case ID_MEDIUM_SIZE:
		resize_window(1);
		break;
	case ID_LARGE_SIZE:
		resize_window(1.2);
		break;
	default:
		checkbox_handle(id);
		break;
	}
}

void app::checkbox_handle(WORD id)
{
	HMENU menu = GetMenu(main_window);
		switch (id)
		{
		case ID_BACKGROUND_COLOR:
			CheckMenuItem(menu, ID_BACKGROUND_IMAGE, MF_UNCHECKED);
			choose_background_color_dialog();
			break;
		case ID_BACKGROUND_IMAGE:
			CheckMenuItem(menu, ID_BACKGROUND_COLOR, MF_UNCHECKED);
			choose_background_color_file();
			break;

		case ID_CENTER:
			CheckMenuItem(menu, ID_FILL, MF_UNCHECKED);
			CheckMenuItem(menu, ID_TILE, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FIT, MF_UNCHECKED);
			is_centered = true;
			is_tiled = false;
			is_filled = false;
			is_fit = false;
			InvalidateRect(main_window, nullptr, false);
			break;
		case ID_FILL:
			CheckMenuItem(menu, ID_CENTER, MF_UNCHECKED);
			CheckMenuItem(menu, ID_TILE, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FIT, MF_UNCHECKED);
			is_centered = false;
			is_tiled = false;
			is_filled = true;
			is_fit = false;
			InvalidateRect(main_window, nullptr, false);
			//todo
			break;
		case ID_TILE:
			CheckMenuItem(menu, ID_CENTER, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FILL, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FIT, MF_UNCHECKED);
			is_tiled = true;
			is_centered = false;
			is_filled = false;
			is_fit = false;
			InvalidateRect(main_window, nullptr, false);
			break;
		case ID_FIT:
			CheckMenuItem(menu, ID_CENTER, MF_UNCHECKED);
			CheckMenuItem(menu, ID_TILE, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FILL, MF_UNCHECKED);
			is_centered = false;
			is_tiled = false;
			is_filled = false;
			is_fit = true;
			InvalidateRect(main_window, nullptr, false);
			break;
		}
		CheckMenuItem(menu, id, MF_CHECKED);
	DrawMenuBar(main_window);
}

LRESULT app::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{  
	std::wstring score_text; // initialisation bypasses case label fix
	PAINTSTRUCT ps;
	HBRUSH brush;
	HDC dc, memory_context;
	HBITMAP bitmap, old_bitmap;
	HBITMAP output_bitmap;
	HDC output_buffer;

	RECT main_rc;

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
		move_bullets();
		InvalidateRect(player_window, nullptr, true);
		InvalidateRect(enemy_window, nullptr, true);
		InvalidateRect(main_window, nullptr, true);
		return 0;
	case WM_PAINT:
		// Implement more throughouly
		dc = BeginPaint(window, &ps);

		GetClientRect(main_window, &main_rc);
		output_buffer = CreateCompatibleDC(dc);
		output_bitmap = CreateCompatibleBitmap(output_buffer, window_width, window_height);

		old_bitmap = (HBITMAP)SelectObject(output_buffer, output_bitmap);
		FillRect(output_buffer, &main_rc, background_brush);

		if (is_bitmap_set)
		{
			if (is_filled)
				fill_bitmap(dc);
			else if (is_fit)
				fit_bitmap(dc);
			else if (is_centered)
				center_bitmap(dc);
			else if (is_tiled)
				tile_bitmap(dc);
			else
				render_bitmap(dc);
		}

		SetBkMode(dc, TRANSPARENT);

		score_text = L"Score: " + std::to_wstring(score);
		//TextOutW(output_buffer, 10, 10, score_text.c_str(), score_text.length());

		TextOutW(dc, 10, 10, score_text.c_str(), score_text.length());

		DeleteObject(output_bitmap);
		DeleteDC(output_buffer);
		EndPaint(window, &ps);
		return 0;

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
		return 0;
	// WM_COMMAND IS RECEIVED WHENEVER THE ID ITEM IS CHOSEN
	case WM_COMMAND:
		on_command(LOWORD(wparam));// the lower 16 bits of the DWORD wparam contains the identifier for the command
		return 0;
	case WM_CTLCOLORSTATIC:
		return reinterpret_cast<INT_PTR>(bullet_brush);
	default:
 		return DefWindowProc(window, message, wparam, lparam);
	}
}


void app::choose_background_color_file()
{
	wchar_t file_name[100];
	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = main_window;
	ofn.hInstance = app_instance;
	ofn.lpstrFilter = NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = file_name;
	ZeroMemory(ofn.lpstrFile, 100 * sizeof(wchar_t));
	ofn.lpstrFile[0] = L'\0';
	ofn.nMaxFile = sizeof(file_name);
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = L"C:\\";
	ofn.lpstrTitle = NULL;
	ofn.Flags = OFN_CREATEPROMPT | OFN_ENABLESIZING | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
	{
		set_background(ofn.lpstrFile);
	}
}

void app::choose_background_color_dialog()
{
	CHOOSECOLOR cc;
	static COLORREF custColors[16] = { 0 };
	cc.lStructSize = sizeof(cc);
	cc.hwndOwner = main_window;
	cc.lpCustColors = custColors;
	cc.Flags = CC_FULLOPEN | CC_SOLIDCOLOR;
	cc.rgbResult = RGB(255, 255, 255);
	// IF user click teh OK button -return value is nonzero
	if (ChooseColor(&cc))
	{
		set_background_color(cc.rgbResult);
	}
}

void app::set_background_color(COLORREF color)
{
	if(background_brush != nullptr)
		DeleteObject(background_brush);

	background_brush = CreateSolidBrush(RGB(GetRValue(color), GetGValue(color), GetBValue(color)));

	SetClassLongPtr(main_window, GCLP_HBRBACKGROUND, (LONG_PTR)background_brush); // sets background color
	is_bitmap_set = false;

	//redraw_background();

	InvalidateRect(main_window, nullptr, true);  
	//UpdateWindow(main_window); // immediate redraw  
}

void app::set_background(LPWSTR file_name)
{
	HBITMAP bitmap = (HBITMAP)LoadImageW(nullptr, file_name, IMAGE_BITMAP, 0 , 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
	background_bitmap = bitmap;
	is_bitmap_set = true;
	InvalidateRect(main_window, nullptr, false);

	//DeleteObject(bitmap);
}

void app::render_bitmap(HDC dc)
{
	HDC memory_context = CreateCompatibleDC(dc);
	HBITMAP old_bitmap = (HBITMAP)SelectObject(memory_context, background_bitmap);
	BitBlt(dc, 0, 0, window_width, window_height, memory_context, 0, 0, SRCCOPY);
	SelectObject(dc, old_bitmap);

	DeleteDC(memory_context);
}

void app::center_bitmap(HDC dc)
{
	HDC memory_context = CreateCompatibleDC(dc);
	HBITMAP old_bitmap = nullptr;
	RECT main_rc;
	BITMAP bitmap_info;
	if (background_bitmap != nullptr) {
		old_bitmap = (HBITMAP)SelectObject(memory_context, background_bitmap);
		GetClientRect(main_window, &main_rc);
		int width = main_rc.right - main_rc.left;
		int height = main_rc.bottom - main_rc.top;
		GetObject(background_bitmap, sizeof(BITMAP), &bitmap_info);
		if (bitmap_info.bmHeight < window_height && bitmap_info.bmWidth < window_width) {
			BitBlt(dc, width / 2 - bitmap_info.bmWidth / 2,
				height / 2 - bitmap_info.bmHeight / 2, window_width, window_height,
				memory_context,
				0, 0,
				SRCCOPY);
		}
		else
		{
			BitBlt(dc, 0,
				0, window_width, window_height,
				memory_context,
				bitmap_info.bmWidth/2 - window_width/2, bitmap_info.bmHeight/2 - window_height/2,
				SRCCOPY);
		}
		SelectObject(dc, old_bitmap);
	}

	DeleteDC(memory_context);
}

void app::fill_bitmap(HDC dc)
{
	HDC memory_context = CreateCompatibleDC(dc);
	HBITMAP old_bitmap = nullptr;
	BITMAP bitmap_info;

	//BITMAP structure extracts the info about the bitmap
	if (background_bitmap != nullptr) {
		old_bitmap = (HBITMAP)SelectObject(memory_context, background_bitmap);
		GetObject(background_bitmap, sizeof(BITMAP), &bitmap_info);
		StretchBlt(dc, 0, 0, window_width, window_height, memory_context, 0, 0,
			bitmap_info.bmWidth, bitmap_info.bmHeight, SRCCOPY);
	}

	if (old_bitmap != nullptr)
		SelectObject(dc, old_bitmap);

	DeleteDC(memory_context);
}

void app::fit_bitmap(HDC dc)
{
	// we need o match the dimensions of the image, so that it would not be distorted
	// prone to rounding errors
	HDC memory_context = CreateCompatibleDC(dc);
	HBITMAP old_bitmap = nullptr;
	BITMAP bitmap_info;
	RECT main_rc;

	if (background_bitmap != nullptr) {
		old_bitmap = (HBITMAP)SelectObject(memory_context, background_bitmap);
		GetObject(background_bitmap, sizeof(BITMAP), &bitmap_info);

		GetClientRect(main_window, &main_rc);
		int width = main_rc.right - main_rc.left;
		int height = main_rc.bottom - main_rc.top;

		float factor_x = (float) width / (float) bitmap_info.bmWidth;
		float factor_y = (float) height / (float) bitmap_info.bmHeight;

		float factor = factor_x > factor_y ? factor_y : factor_x;

		int new_width = bitmap_info.bmWidth * factor;
		int new_height = bitmap_info.bmHeight * factor;

		int x_indent = width / 2 - new_width / 2;
		int y_indent = height / 2 - new_height / 2;

		StretchBlt(dc, x_indent, y_indent, new_width, new_height, memory_context, 0, 0,
			bitmap_info.bmWidth, bitmap_info.bmHeight, SRCCOPY);

		SelectObject(dc, old_bitmap);
	}
		
	DeleteDC(memory_context);
}

// memory_context prevents flickering it uses double-buffering
void app::tile_bitmap(HDC dc)
{
	HDC memory_context = CreateCompatibleDC(dc); // acts as an offscreen buffer
	HDC memory_context_output = CreateCompatibleDC(dc);

	HBITMAP memory_bitmap = CreateCompatibleBitmap(dc, window_width, window_height);
	SelectObject(memory_context_output, memory_bitmap);

	HBITMAP old_bitmap = nullptr;
	BITMAP bitmap_info;
	RECT main_rc;

	if (background_bitmap != nullptr) {
		old_bitmap = (HBITMAP)SelectObject(memory_context, background_bitmap);
		GetObject(background_bitmap, sizeof(BITMAP), &bitmap_info);

		GetClientRect(main_window, &main_rc);
		int width = main_rc.right - main_rc.left;
		int height = main_rc.bottom - main_rc.top;

		for (int x = 0; x < width; x += bitmap_info.bmWidth)
			for (int y = 0; y < height; y += bitmap_info.bmHeight)
				BitBlt(memory_context_output, x, y, bitmap_info.bmWidth, bitmap_info.bmHeight,
					memory_context, 0, 0, SRCCOPY);

		BitBlt(dc, 0, 0, window_width, window_height, memory_context_output, 0, 0, SRCCOPY);
		SelectObject(dc, old_bitmap);
	}

	DeleteObject(memory_bitmap);
	DeleteDC(memory_context_output);
	DeleteDC(memory_context);
}

void app::redraw_background()
{
	HDC dc = GetDC(main_window);
	RECT main_rc;
	GetClientRect(main_window,&main_rc);
	FillRect(dc, &main_rc, background_brush);
	//is_redraw_background = false;
	ReleaseDC(main_window, dc);
}

void app::create_dialog_box()
{
	//DialogBoxW();
}


void app::render_plane_sprites(HDC dc)
{
	static int current_frame = 0;
	static const int no_of_frames = 3;

	HDC memory_context = CreateCompatibleDC(dc);

	SelectObject(memory_context, plane_bitmap);

	int x = (current_frame % no_of_frames) * player_width;
	int y = 0;

	current_frame = (current_frame + 1) % no_of_frames;

	//COLORREF color_to_exclude = GetPixel(memory_context, 0, 0); // extracts the color ofthe pixel

	//BitBlt(dc, 0, 0, player_width, player_height, memory_context, x, y, SRCCOPY);
	
	TransparentBlt(dc, 0, 0, player_width, player_height, memory_context, x, y, 50, 50, color_to_exclude);
	//BitBlt(dc, 400, 400, player_width, player_height, memory_context, x, y, SRCCOPY);


	DeleteDC(memory_context);
}

void app::render_enemy_sprites(HDC dc)
{
	static int current_frame = 0;
	static const int no_of_frames = 4;

	//HDC client_area = GetDC(main_window); //extracts the dra context for the client area
	HDC memory_context = CreateCompatibleDC(dc);

	SelectObject(memory_context, enemy_bitmap);

	int x = (current_frame % no_of_frames) * enemy_width;
	int y = 0;

	current_frame = (current_frame + 1) % no_of_frames;


	//BLENDFUNCTION blendFunc;
	//blendFunc.BlendOp = AC_SRC_OVER;
	//blendFunc.BlendFlags = 0;
	//blendFunc.SourceConstantAlpha = 0; // Full opacity
	//blendFunc.AlphaFormat = AC_SRC_ALPHA; // Use per-pixel alpha

	//AlphaBlend(dc, 0, 0, enemy_width, enemy_height, memory_context, x, y, 50, 40, blendFunc);

	TransparentBlt(dc, 0, 0, enemy_width, enemy_height, memory_context, x, y, 50, 40, color_to_exclude);
	//BitBlt(dc,0,0, enemy_width, enemy_height, memory_context, x, y, SRCCOPY);

	DeleteDC(memory_context);
}

void app::reset_game()
{
	for (auto& bullet : bullets)
	{
		DestroyWindow(bullet.bullet_window);
	}
	bullets.clear();

	MoveWindow(main_window, screen_size.x / 2 - window_width / 2, screen_size.y / 2 - window_height / 2, window_width, window_height, true);
	MoveWindow(player_window, window_width / 2 - player_width / 2, window_height - window_height / 4 - player_height, player_width, player_height, true);
	MoveWindow(enemy_window, window_width / 2 - enemy_width / 2, window_height / 6, enemy_width, enemy_height, true);

	//is_redraw_background = true;

	InvalidateRect(enemy_window, nullptr, true);
	InvalidateRect(player_window, nullptr, true);
	InvalidateRect(main_window, nullptr, true);
	enemy_window_direction = 1;
	score = 0;
}


void app::resize_window(float factor)
{
	// this ensured nothing is redrawed if the size was not changed
	if (window_width == default_window_width * factor && window_height == default_window_height * factor)
		return; 

	int prev_window_width = window_width;
	int prev_window_height = window_height;

	// WE probably need to scale the values accordingly 
	window_width = default_window_width * factor;
	window_height = default_window_height * factor;

	reset_game();
}


app::~app()
{
	DeleteObject(player_brush);
	DeleteObject(enemy_brush);
	DeleteObject(background_brush);
	DeleteObject(bullet_brush);
	KillTimer(main_window, 1);
	if (background_bitmap != nullptr)
		DeleteObject(background_bitmap);
}


//PAINTSTRUCT.rcPaint: The PAINTSTRUCT structure, passed to the BeginPaint function, contains a rectangle(rcPaint) that represents the area that needs to be repainted
// .You can use this rectangle to restrict your drawing to the invalidated area.If you don’t restrict the painting to this area, the entire window will be redrawn, which is not necessary.

//int x = screen_size.x / 2 - window_width / 2;
	//int y = screen_size.y / 2 - window_height / 2;

	//RECT player_rc, enemy_rc;
	//GetWindowRect(player_window, &player_rc);
	//GetWindowRect(player_window, &enemy_rc);

	//POINT en_client_coordinates{ enemy_rc.left, enemy_rc.top };
	//POINT pl_client_coordinates{ player_rc.left, player_rc.top };

	//ScreenToClient(main_window, &pl_client_coordinates);
	//ScreenToClient(main_window, &en_client_coordinates);

	//float x_scalling_factor = (float)window_width / (float)prev_window_width;
	//float y_scalling_factor = (float)window_height / (float)prev_window_height;

	////int pl_new_x = (pl_client_coordinates.x * x_scalling_factor);
	//int en_new_x = (en_client_coordinates.x * x_scalling_factor);

	//int pl_new_x = pl_client_coordinates.x * x_scalling_factor;
	//int pl_new_y = pl_client_coordinates.y * y_scalling_factor;

	//MoveWindow(main_window, x, y, window_width, window_height, true);
	//MoveWindow(player_window, pl_new_x , pl_new_y, player_width, player_height, true); // rounding errors based on scalling factor
	//MoveWindow(enemy_window, en_new_x, window_height / 6, enemy_width, enemy_height, true); // rescales the x position based on the factor
	//
	////For moving bullets properly
	//GetWindowRect(player_window, &player_rc);
	//POINT client_coordinates{ player_rc.left, player_rc.top };
	//ScreenToClient(main_window, &client_coordinates);

	//int bullet_x = client_coordinates.x + player_width / 2 - bullet_width / 2;
	//for (auto& bullet : bullets)
	//{
	//	bullet.x = bullet_x;
	//	bullet.y = bullet.y * y_scalling_factor;
	//	MoveWindow(bullet.bullet_window, bullet.x, bullet.y, bullet_width, bullet_height, true);
	//}

/*HWND hwndStatic;
		hwndStatic = reinterpret_cast<HWND>(lparam);
		if (hwndStatic == enemy_window)
			return reinterpret_cast<INT_PTR>(enemy_brush);
		else if (hwndStatic == player_window)
			return reinterpret_cast<INT_PTR>(player_brush);
		else*/


		//BitBlt(dc, 0, 0, window_width, window_height ,output_buffer, 0, 0, SRCCOPY);

				//SelectObject(output_buffer, old_bitmap);
				//DeleteObject(bitmap);
				//DeleteDC(memory_context);


// Sets background transparent
		/*memory_context = CreateCompatibleDC(dc);
		bitmap = CreateCompatibleBitmap(dc, window_width, window_height);
		old_bitmap = (HBITMAP)SelectObject(memory_context, bitmap);*/

		//SetBkMode(output_buffer, TRANSPARENT);