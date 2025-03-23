#include "app.h"
#include <stdexcept>
#include "resource.h"
#include <commdlg.h> 

//By default, Alt + F4 works on your window because the Windows operating system sends a WM_CLOSE message when the user presses Alt + F4.Your window automatically processes this message unless you override it.

std::wstring const app::s_class_name = L"Space Invaders Window Class";

int app::enemy_window_direction = 1;

app::app(HINSTANCE instance) : app_instance{ instance }, main_window{},
screen_size{GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)},
enemy_brush{CreateSolidBrush(RGB(70,70,255))},
player_brush{ CreateSolidBrush(RGB(255, 0, 0)) },
background_brush{CreateSolidBrush(RGB(255, 255, 255))},
plane_bitmap{LoadBitmapW(app_instance, MAKEINTRESOURCEW(ID_PLANE_BITMAP))},
enemy_bitmap{LoadBitmapW(app_instance, MAKEINTRESOURCEW(ID_ENEMIES_BITMAP))}
{
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_BORDER | WS_CLIPCHILDREN ;
	DWORD ex_style = WS_EX_LAYERED | WS_EX_TOPMOST;

	register_class();
	main_window = create_window(style, ex_style);
	enemy_window = create_child_window(enemy_width, enemy_height, window_width/2 - enemy_width/2, window_height/6);
	player_window = create_child_window(player_width, player_height, window_width/2 - player_width/2 , window_height - window_height / 4 - player_height);

	SetLayeredWindowAttributes(main_window, 0, 255, LWA_ALPHA);
	SetLayeredWindowAttributes(player_window, RGB(0, 0, 0), 0, LWA_ALPHA);
	SetLayeredWindowAttributes(enemy_window, RGB(0, 0, 0), 0, LWA_ALPHA);
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

int app::run(int show_window)
{
	SetTimer(main_window,1, 50, nullptr);
	ShowWindow(main_window, show_window);

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
}

void app::create_bullets()
{
	RECT player_rc;
	GetWindowRect(player_window, &player_rc);

	POINT client_coordinates{ player_rc.left, player_rc.top };
	ScreenToClient(main_window, &client_coordinates);

	int x = client_coordinates.x + player_width / 2 - bullet_width / 2;
	int y = client_coordinates.y;
	HWND bullet_window = create_child_window(bullet_width, bullet_height, x, y);
	bullets.push_back(bullet(bullet_window, x, y));
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

			InvalidateRect(main_window, nullptr, false);
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
	//UINT mask = GetMenuState(menu, id, MF_BYCOMMAND); // to find by uId by identifier
	//if ((mask & MF_CHECKED) != 0)
	//	CheckMenuItem(menu, id, MF_BYCOMMAND | MF_UNCHECKED);
	//else {
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
			//todo
			break;
		case ID_FILL:
			CheckMenuItem(menu, ID_CENTER, MF_UNCHECKED);
			CheckMenuItem(menu, ID_TILE, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FIT, MF_UNCHECKED);
			//todo
			break;
		case ID_TILE:
			CheckMenuItem(menu, ID_CENTER, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FILL, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FIT, MF_UNCHECKED);
			break;
		case ID_FIT:
			CheckMenuItem(menu, ID_CENTER, MF_UNCHECKED);
			CheckMenuItem(menu, ID_TILE, MF_UNCHECKED);
			CheckMenuItem(menu, ID_FILL, MF_UNCHECKED);
			break;
		}
		CheckMenuItem(menu, id, MF_CHECKED);
	DrawMenuBar(main_window);
}

LRESULT app::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{  
	std::wstring scoreText; // initialisation bypasses case label fix
	PAINTSTRUCT ps;
	HBRUSH brush;
	HDC dc;

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
		InvalidateRect(main_window, nullptr, TRUE); // Invalidates the whole client area
		//InvalidateRect(player_window, nullptr, TRUE);
		return 0;
	case WM_PAINT:
		// Implement more throughouly
		dc = BeginPaint(window, &ps);

		scoreText = L"Score: " + std::to_wstring(score);
		FillRect(dc, &ps.rcPaint, background_brush);
		TextOutW(dc, 10, 10, scoreText.c_str(), scoreText.length());

		render_enemy_sprites(dc);
		render_plane_sprites(dc);

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

void app::choose_background_color_file()
{

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

	InvalidateRect(main_window, nullptr, TRUE);  
	UpdateWindow(main_window); // immediate redraw  
}



void app::render_plane_sprites(HDC dc)
{
	static int current_frame = 0;
	static const int no_of_frames = 3;

	//HDC client_area = GetDC(main_window); //extracts the dra context for the client area
	HDC memory_context = CreateCompatibleDC(dc);

	SelectObject(memory_context, plane_bitmap);

	int x = (current_frame % no_of_frames) * player_width;
	int y = 0;

	current_frame = (current_frame + 1) % no_of_frames;

	RECT player_rc;

	GetWindowRect(player_window, &player_rc);
	POINT point{ player_rc.left, player_rc.top };

	ScreenToClient(main_window, &point);

	//BitBlt(dc, point.x, point.y, enemy_width, enemy_height, memory_context, x, y, SRCCOPY);
	BitBlt(dc, 400, 400, player_width, player_height, memory_context, x, y, SRCCOPY);


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

	RECT enemy_rc;

	GetWindowRect(enemy_window, &enemy_rc);
	POINT point{ enemy_rc.left, enemy_rc.top };

	ScreenToClient(main_window, &point);

	BitBlt(dc,point.x,point.y, enemy_width, enemy_height, memory_context, x, y, SRCCOPY);

	DeleteDC(memory_context);
}

void app::reset_game()
{
	for (auto& bullet : bullets)
	{
		DestroyWindow(bullet.bullet_window);
	}
	bullets.clear();

	MoveWindow(player_window, window_width / 2 - player_width / 2, window_height - window_height / 4 - player_height, player_width, player_height, true);
	MoveWindow(enemy_window, window_width / 2 - enemy_width / 2, window_height / 6, enemy_width, enemy_height, true);
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

	int x = screen_size.x / 2 - window_width / 2;
	int y = screen_size.y / 2 - window_height / 2;

	RECT player_rc, enemy_rc;
	GetWindowRect(player_window, &player_rc);
	GetWindowRect(player_window, &enemy_rc);

	POINT en_client_coordinates{ enemy_rc.left, enemy_rc.top };
	POINT pl_client_coordinates{ player_rc.left, player_rc.top };

	ScreenToClient(main_window, &pl_client_coordinates);
	ScreenToClient(main_window, &en_client_coordinates);

	float x_scalling_factor = (float)window_width / (float)prev_window_width;
	float y_scalling_factor = (float)window_height / (float)prev_window_height;

	//int pl_new_x = (pl_client_coordinates.x * x_scalling_factor);
	int en_new_x = (en_client_coordinates.x * x_scalling_factor);

	int pl_new_x = pl_client_coordinates.x * x_scalling_factor;
	int pl_new_y = pl_client_coordinates.y * y_scalling_factor;

	MoveWindow(main_window, x, y, window_width, window_height, true);
	MoveWindow(player_window, pl_new_x , pl_new_y, player_width, player_height, true); // rounding errors based on scalling factor
	MoveWindow(enemy_window, en_new_x, window_height / 6, enemy_width, enemy_height, true); // rescales the x position based on the factor
	
	//For moving bullets properly
	GetWindowRect(player_window, &player_rc);
	POINT client_coordinates{ player_rc.left, player_rc.top };
	ScreenToClient(main_window, &client_coordinates);

	int bullet_x = client_coordinates.x + player_width / 2 - bullet_width / 2;
	for (auto& bullet : bullets)
	{
		bullet.x = bullet_x;
		bullet.y = bullet.y * y_scalling_factor;
		MoveWindow(bullet.bullet_window, bullet.x, bullet.y, bullet_width, bullet_height, true);
	}
}



app::~app()
{
	DeleteObject(player_brush);
	DeleteObject(enemy_brush);
	DeleteObject(background_brush);
	KillTimer(main_window, 1);
}


//PAINTSTRUCT.rcPaint: The PAINTSTRUCT structure, passed to the BeginPaint function, contains a rectangle(rcPaint) that represents the area that needs to be repainted
// .You can use this rectangle to restrict your drawing to the invalidated area.If you don’t restrict the painting to this area, the entire window will be redrawn, which is not necessary.