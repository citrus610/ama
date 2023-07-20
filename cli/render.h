#pragma once
#pragma comment(lib, "winmm.lib")

#ifndef UNICODE
#error Enable UNICODE for the compiler!
#endif

#include <windows.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <cassert>

enum Color
{
    COLOR_FG_BLACK = 0x0000,
    COLOR_FG_DARK_BLUE = 0x0001,    
    COLOR_FG_DARK_GREEN = 0x0002,
    COLOR_FG_DARK_CYAN = 0x0003,
    COLOR_FG_DARK_RED = 0x0004,
    COLOR_FG_DARK_MAGENTA = 0x0005,
    COLOR_FG_DARK_YELLOW = 0x0006,
    COLOR_FG_GREY = 0x0007,
    COLOR_FG_DARK_GREY = 0x0008,
    COLOR_FG_BLUE = 0x0009,
    COLOR_FG_GREEN = 0x000A,
    COLOR_FG_CYAN = 0x000B,
    COLOR_FG_RED = 0x000C,
    COLOR_FG_MAGENTA = 0x000D,
    COLOR_FG_YELLOW = 0x000E,
    COLOR_FG_WHITE = 0x000F,
    COLOR_BG_BLACK = 0x0000,
    COLOR_BG_DARK_BLUE = 0x0010,
    COLOR_BG_DARK_GREEN = 0x0020,
    COLOR_BG_DARK_CYAN = 0x0030,
    COLOR_BG_DARK_RED = 0x0040,
    COLOR_BG_DARK_MAGENTA = 0x0050,
    COLOR_BG_DARK_YELLOW = 0x0060,
    COLOR_BG_GREY = 0x0070,
    COLOR_BG_DARK_GREY = 0x0080,
    COLOR_BG_BLUE = 0x0090,
    COLOR_BG_GREEN = 0x00A0,
    COLOR_BG_CYAN = 0x00B0,
    COLOR_BG_RED = 0x00C0,
    COLOR_BG_MAGENTA = 0x00D0,
    COLOR_BG_YELLOW = 0x00E0,
    COLOR_BG_WHITE = 0x00F0,
};

enum PixelType
{
    PIXEL_SOLID = 0x2588,
    PIXEL_THREEQUARTERS = 0x2593,
    PIXEL_HALF = 0x2592,
    PIXEL_QUARTER = 0x2591,
    PIXEL_CIRCLE = '0'
};

static int width;
static int height;
static CHAR_INFO *screen;
static std::wstring title;
static HANDLE original_console_handle;
static CONSOLE_SCREEN_BUFFER_INFO original_console_info;
static HANDLE console_handle;
static HANDLE console_input_handle;
static SMALL_RECT window_rectangle;

static BOOL close_handler(DWORD event)
{
    if (event == CTRL_CLOSE_EVENT)
    {
        
    }
    return TRUE;
};

static void create_window(int w, int h, int pixel_size)
{
    width = w;
    height = h;
    screen = new CHAR_INFO[width * height];
    memset(screen, 0, sizeof(CHAR_INFO) * width * height);

    console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
    console_input_handle = GetStdHandle(STD_INPUT_HANDLE);

    title = L"Puyo AI";

    if (console_handle == INVALID_HANDLE_VALUE) {
        assert(false);
        return;
    }

    window_rectangle = { 0, 0, 1, 1 };
    SetConsoleWindowInfo(console_handle, TRUE, &window_rectangle);

    COORD coord = { (short)width, (short)height };
    if (!SetConsoleScreenBufferSize(console_handle, coord)) {
        assert(false);
    }
    if (!SetConsoleActiveScreenBuffer(console_handle)) {
        assert(false);
    }
    
    CONSOLE_FONT_INFOEX console_font_info;
    console_font_info.cbSize = sizeof(console_font_info);
    console_font_info.nFont = 0;
    console_font_info.dwFontSize.X = pixel_size;
    console_font_info.dwFontSize.Y = pixel_size;
    console_font_info.FontFamily = FF_DONTCARE;
    console_font_info.FontWeight = FW_NORMAL;
    wcscpy_s(console_font_info.FaceName, L"Consolas");
    if (!SetCurrentConsoleFontEx(console_handle, false, &console_font_info)) {
        assert(false);
    }

    CONSOLE_SCREEN_BUFFER_INFO console_screen_buffer_infor;
    if (!GetConsoleScreenBufferInfo(console_handle, &console_screen_buffer_infor)) {
        assert(false);
    }
    if (height > console_screen_buffer_infor.dwMaximumWindowSize.Y) {
        assert(false);
    }
    if (width > console_screen_buffer_infor.dwMaximumWindowSize.X) {
        assert(false);
    }

    window_rectangle = { 0, 0, (short)(width - 1), (short)(height - 1) };
    if (!SetConsoleWindowInfo(console_handle, TRUE, &window_rectangle)) {
        assert(false);
    }        
    if (!SetConsoleMode(console_handle, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)) {
        assert(false);
    }

    HWND console_window = GetConsoleWindow();
    long console_window_style = GetWindowLong(console_window, GWL_STYLE);
    console_window_style ^= WS_SIZEBOX;
    console_window_style ^= WS_MAXIMIZEBOX;
    SetWindowLong(console_window, GWL_STYLE, console_window_style);

    SetConsoleCtrlHandler((PHANDLER_ROUTINE)close_handler, TRUE);
};

static void clear()
{
    memset(screen, 0, sizeof(CHAR_INFO) * width * height);
};

static void draw(int x, int y, PixelType pixel, Color color)
{
    if (x >= 0 && x < width && y >= 0 && y < height)
    {
        screen[y * width + x].Char.UnicodeChar = pixel;
        screen[y * width + x].Attributes = color;
    }
};

static void draw_rectangle(int x, int y, int w, int h, PixelType pixel, Color color)
{
    x = std::min(x, width);
    x = std::max(x, 0);

    y = std::min(y, height);
    y = std::max(y, 0);

    w = std::min(w, width - x);
    w = std::max(w, 0);

    h = std::min(h, height - y);
    h = std::max(h, 0);

    for (int draw_x = x; draw_x < x + w; ++draw_x) {
        for (int draw_y = y; draw_y < y + h; ++draw_y) {
            draw(draw_x, draw_y, pixel, color);
        }
    }
};

static void draw_text(int x, int y, std::wstring text, Color color)
{
    for (size_t i = 0; i < text.size(); ++i)
    {
        if (y * width + x + i < 0 || y * width + x + i >= width * height) continue;
        screen[y * width + x + i].Char.UnicodeChar = text[i];
        screen[y * width + x + i].Attributes = color;
    }
};

static void render()
{
    wchar_t s[256];
    swprintf_s(s, 256, L"%s", title.c_str());
    SetConsoleTitle(s);
    WriteConsoleOutput(console_handle, screen, { (short)width, (short)height }, { 0, 0 }, &window_rectangle);
};