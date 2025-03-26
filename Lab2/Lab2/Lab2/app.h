#pragma once
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <vector>
#include <commdlg.h> 
#include <chrono>

class bullet
{
public:
	HWND bullet_window;
	int x;
	int y;

	inline bullet(HWND bullet_window, int x, int y) : bullet_window{ bullet_window }, x{ x }, y{ y } 
	{}
};

class app
{
private:
	static std::wstring const s_class_name;
	static std::wstring const s_enemy_class_name;
	static std::wstring const s_player_class_name;
	std::vector<bullet> bullets;
	HWND main_window, enemy_window, player_window;
	HBRUSH enemy_brush, player_brush;
	HBRUSH background_brush;
	HBRUSH bullet_brush;
	HBITMAP background_bitmap;
	POINT screen_size;
	HINSTANCE app_instance;

	bool is_bitmap_set;
	bool is_filled;
	bool is_centered;
	bool is_tiled;
	bool is_fit;
	//bool is_score_updated;
	//bool is_redraw_background;

	HBITMAP plane_bitmap;
	HBITMAP enemy_bitmap;

	bool register_class();
	bool register_class_player();
	bool register_class_enemy();

	HWND create_window(DWORD style, DWORD ex_style, HWND parent = nullptr);
	HWND create_window_other(DWORD style, DWORD ex_style, int x, int y, int width, int height, LPCWSTR class_name, HWND parent);


	HWND create_child_window(int width, int height,int x, int y);

	static LRESULT CALLBACK static_window_proc(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	static LRESULT CALLBACK static_window_proc_player(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	static LRESULT CALLBACK static_window_proc_enemy(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	LRESULT window_proc(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	LRESULT window_proc_enemy(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	LRESULT window_proc_player(HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);

	void create_dialog_box();
	void redraw_background();
	void move_player_window(int direction);
	void move_enemy_window();
	void create_bullets();
	void move_bullets();
	void on_command(WORD id);
	void reset_game();
	void resize_window(float factor); // small -> half of the dimension size; medium -> default size; large -> twice the default_size

	void render_plane_sprites(HDC dc);
	void render_enemy_sprites(HDC dc);
	void checkbox_handle(WORD id);
	void choose_background_color_dialog();
	void choose_background_color_file();
	void set_background_color(COLORREF color);
	void set_background(LPWSTR file_name);
	void fill_bitmap(HDC dc);
	void center_bitmap(HDC dc);
	void fit_bitmap(HDC dc);
	void tile_bitmap(HDC dc);
	void render_bitmap(HDC dc);
	void update_score();

	static int enemy_window_direction; // 1 signs - right; 0 - signs left

	static constexpr int default_window_width = 800;
	static constexpr int default_window_height = 600;
	static constexpr int bullet_width = 5;
	static constexpr int bullet_height = 15;
	static constexpr int enemy_width = 50;
	static constexpr int enemy_height = 40;
	static constexpr int player_width = 50;
	static constexpr int player_height = 50;

	int window_width = default_window_width;
	int window_height = default_window_height;

	std::chrono::time_point<std::chrono::system_clock> last_bullet_time = std::chrono::system_clock::now();

	int score = 0;
	bool score_changed = false;

public:
	app(HINSTANCE instance);
	int run(int show_window);
	~app();
};