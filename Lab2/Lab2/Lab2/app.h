#pragma once
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <vector>

class bullet
{
public:
	HWND bullet_window;
	int x_position;
	int y_position;

	bullet();
};

class app
{
private:
	static std::wstring const s_class_name;
	std::vector<bullet> bullets;
	HWND main_window, enemy_window, player_window;
	HBRUSH enemy_brush, player_brush;
	POINT screen_size;
	HINSTANCE app_instance;

	bool register_class();
	HWND create_window(DWORD style, DWORD ex_style, HWND parent = nullptr);

	HWND create_child_window(int width, int height, int y);

	static LRESULT CALLBACK static_window_proc(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	LRESULT window_proc(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	void move_player_window(int direction);
	void move_enemy_window();
	void create_bullets();

	static constexpr int window_width = 800;
	static constexpr int window_height = 600;


public:
	app(HINSTANCE instance);
	int run(int show_window);
	~app();
};