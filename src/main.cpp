#include <windows.h>
#include <chrono>
#include <string>

// --- Khai báo các Header hệ thống ---
#include "ApplicationTypes/GameState.h"
#include "ApplicationTypes/PlayState.h"
#include "ApplicationTypes/GameConfig.h"
#include "SystemModules/ConfigLoader.h"
#include "SystemModules/AudioSystem.h"
#include "SystemModules/TimeSystem.h"
#include "RenderAPI/Colours.h"
#include "RenderAPI/Renderer.h"
#include "RenderAPI/UIComponents.h"
#include "GameLogic/GameEngine.h"

// --- Khai báo các Header màn hình ---
#include "ScreenModules/MenuScreen.h"
#include "ScreenModules/PlayScreen.h"
#include "ScreenModules/SettingScreen.h"
#include "ScreenModules/LoadGameScreen.h"
#include "ScreenModules/MatchConfigScreen.h"

// --- Trạng thái toàn cục ---
ScreenState g_CurrentScreen = SCREEN_MENU;
GameConfig  g_Config;
PlayState   g_PlayState;

// Lựa chọn hiện tại trong các menu
int g_ConfigSelected = 0;
int g_MenuSelected = 0;
int g_LoadSelected = 0;
int g_SettingSelected = 0;
std::wstring g_LoadStatus = L"";

// Tài nguyên đồ họa
Sprite g_SpriteX, g_SpriteO;
ULONG_PTR g_GdiplusToken;

// --- Khai báo hàm Win32 ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 1. Khởi tạo Đồ họa & Font
    if (!InitGraphics(g_GdiplusToken)) {
        return 0;
    }
    
    // Nạp Font Tiếng Việt vào bộ nhớ riêng của App
    AddFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Regular.ttf", FR_PRIVATE, 0);
    AddFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Bold.ttf", FR_PRIVATE, 0);
    AddFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Black.ttf", FR_PRIVATE, 0);
    AddFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Italic.ttf", FR_PRIVATE, 0);

    GlobalFont::Initialize();

    // 2. Tải Cấu hình & Nhạc nền
    LoadConfig(&g_Config, "Asset/config.ini");
    if (g_Config.isBgmEnabled) {
        //PlayBGM("Asset/audio/bgm_menu.wav");
    }

    // 3. Đăng ký lớp cửa sổ
    const wchar_t CLASS_NAME[] = L"GomokuGameClass";
    WNDCLASSW wc = {};
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    // 4. Tạo cửa sổ (Kích thước 850x750 để bàn cờ 15x15 nằm cân đối)
    HWND hWnd = CreateWindowExW(
        0, CLASS_NAME, L"Caro & Tic-Tac-Toe - Ultimate Edition",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 850, 750,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    g_SpriteX = LoadPNG(L"Asset/images/x.png");
    g_SpriteO = LoadPNG(L"Asset/images/o.png");

    MSG msg = {};
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            // Tối ưu hóa FPS (~60 FPS) bằng cách Sleep khoảng 16ms
            Sleep(16);
            
            // Tính toán Delta Time (dt)
            auto currentTime = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed = currentTime - lastTime;
            lastTime = currentTime;
            double dt = elapsed.count();

            g_GlobalAnimTime += (float)dt;

            // Cập nhật Logic (Đếm ngược, AI tự đánh)
            if (g_CurrentScreen == SCREEN_PLAY) {
                UpdatePlayLogic(&g_PlayState, dt);
            }
            else if (g_CurrentScreen == SCREEN_EXIT) {
                PostQuitMessage(0);
            }

            // Ép vẽ lại toàn cục mỗi khung hình để Animation mượt
            InvalidateRect(hWnd, NULL, FALSE);
        }
    }

    // 7. Giải phóng tài nguyên
    FreeSprite(g_SpriteX);
    FreeSprite(g_SpriteO);
    GlobalFont::Cleanup();
    
    // Gỡ font khỏi bộ nhớ
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Regular.ttf", FR_PRIVATE, 0);
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Bold.ttf", FR_PRIVATE, 0);
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Black.ttf", FR_PRIVATE, 0);
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Italic.ttf", FR_PRIVATE, 0);
    
    ShutdownGraphics(g_GdiplusToken);

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_SIZE:
        InvalidateRect(hWnd, NULL, FALSE);
        return DefWindowProc(hWnd, message, wParam, lParam);

    case WM_KEYDOWN: {
        bool changed = false;

        // Điều phối Input theo màn hình hiện tại
        switch (g_CurrentScreen) {
        case SCREEN_MENU:
            UpdateMenuScreen(g_CurrentScreen, g_MenuSelected, wParam);
            if (g_CurrentScreen == SCREEN_PLAY) {
                // Thay vì vào SCREEN_PLAY, ta chuyển sang SCREEN_MATCH_CONFIG
                g_CurrentScreen = SCREEN_MATCH_CONFIG;
                g_ConfigSelected = 0;
                // Khởi tạo thông số mặc định để người chơi chỉnh sửa
                g_PlayState.gameMode = MODE_CARO;
                g_PlayState.matchType = MATCH_PVP;
                g_PlayState.difficulty = 2;
                g_PlayState.countdownTime = 30;
                g_PlayState.targetScore = 1;
            }
            changed = true;
            break;

        case SCREEN_MATCH_CONFIG:
            UpdateMatchConfigScreen(g_CurrentScreen, &g_PlayState, g_ConfigSelected, wParam);
            if (wParam == VK_ESCAPE) {
                g_CurrentScreen = SCREEN_MENU;
            }
            changed = true;
            break;
        case SCREEN_PLAY:
            UpdatePlayScreen(&g_PlayState, g_CurrentScreen, wParam, &g_Config);
            changed = true;
            break;
        case SCREEN_SETTING:
            UpdateSettingScreen(g_CurrentScreen, &g_Config, g_SettingSelected, wParam);
            changed = true;
            break;
        case SCREEN_LOAD_GAME:
            UpdateLoadGameScreen(g_CurrentScreen, &g_PlayState, g_LoadSelected, g_LoadStatus, wParam);
            changed = true;
            break;
        case SCREEN_GUIDE:
            if (wParam == VK_ESCAPE) {
                g_CurrentScreen = SCREEN_MENU;
                changed = true;
            }
            break;

        case SCREEN_ABOUT:
            if (wParam == VK_ESCAPE) {
                g_CurrentScreen = SCREEN_MENU;
                changed = true;
            }
            break;
        case SCREEN_EXIT:
            PostQuitMessage(0);
			break;
        }

        if (changed) {
            InvalidateRect(hWnd, NULL, FALSE);
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // Lấy kích thước thực tế vùng vẽ
        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int w = clientRect.right - clientRect.left;
        int h = clientRect.bottom - clientRect.top;

        // Cơ chế Double Buffering chống chớp màn hình
        DoubleBuffer buffer;
        CreateBuffer(hWnd, hdc, buffer);

        // Vẽ giao diện theo trạng thái màn hình
        switch (g_CurrentScreen) {
        case SCREEN_MENU:
            RenderMenuScreen(buffer.hdcMem, g_MenuSelected, w, h);
            break;
        case SCREEN_PLAY:
            RenderPlayScreen(buffer.hdcMem, &g_PlayState, w, h, g_SpriteX, g_SpriteO, &g_Config);
            break;
        case SCREEN_SETTING:
            RenderSettingScreen(buffer.hdcMem, &g_Config, g_SettingSelected, w, h);
            break;
        case SCREEN_LOAD_GAME:
            RenderLoadGameScreen(buffer.hdcMem, g_LoadSelected, g_LoadStatus, w, h);
            break;
        case SCREEN_MATCH_CONFIG:
            RenderMatchConfigScreen(buffer.hdcMem, g_ConfigSelected, &g_PlayState, w, h);
            break;
        case SCREEN_GUIDE: {
            // Vẽ nền xám nhạt giống Menu
            RECT rect = { 0, 0, w, h };
            HBRUSH hBg = CreateSolidBrush(Colour::GRAY_LIGHTEST); // Sử dụng bảng màu Colour

            // Dòng thông báo thoát
            DrawTextCentered(buffer.hdcMem, L"Nhấn ESC để quay lại Menu", h - 100, w, Colour::ORANGE_NORMAL, GlobalFont::Bold);
            break;
        }

        case SCREEN_ABOUT: {
            RECT rect = { 0, 0, w, h };
            HBRUSH hBg = CreateSolidBrush(Colour::GRAY_LIGHTEST);

            // Dòng thông báo thoát
            DrawTextCentered(buffer.hdcMem, L"Nhấn ESC để quay lại Menu", h - 100, w, Colour::ORANGE_NORMAL, GlobalFont::Bold);
            break;
        }
        }

        // Chép từ Buffer ra màn hình chính
        BitBlt(hdc, 0, 0, w, h, buffer.hdcMem, 0, 0, SRCCOPY);

        DeleteBuffer(buffer);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1; // Ngăn Windows tự xóa nền gây nhấp nháy

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}