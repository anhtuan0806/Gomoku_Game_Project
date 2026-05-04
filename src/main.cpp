#include <windows.h>
#include <chrono>
#include <sstream>
#include <string>
#include <timeapi.h>

// --- Khai báo các Header hệ thống ---
#include "ApplicationTypes/GameState.h"
#include "ApplicationTypes/PlayState.h"
#include "ApplicationTypes/GameConfig.h"
#include "SystemModules/ConfigLoader.h"
#include "SystemModules/AudioSystem.h"
#include "SystemModules/TimeSystem.h"
#include "SystemModules/Localization.h"
#include "RenderAPI/Colours.h"
#include "RenderAPI/UIScaler.h"
#include "RenderAPI/Renderer.h"
#include "RenderAPI/UIComponents.h"
#include "GameLogic/GameEngine.h"

// --- Khai báo các Header màn hình ---
#include "ScreenModules/MenuScreen.h"
#include "ScreenModules/PlayScreen.h"
#include "ScreenModules/SettingScreen.h"
#include "ScreenModules/LoadGameScreen.h"
#include "ScreenModules/MatchConfigScreen.h"
#include "ScreenModules/GuildScreen.h"
#include "ScreenModules/AboutScreen.h"

// --- Trạng thái toàn cục ---
ScreenState g_CurrentScreen = SCREEN_MENU;
GameConfig g_Config;
PlayState g_PlayState;

// Lựa chọn hiện tại trong các menu
int g_ConfigSelected = 0;
int g_MenuSelected = 0;
int g_LoadSelected = 0;
int g_SettingSelected = 0;
std::wstring g_LoadStatus = L"";
int g_GuildPage = 0;

// Tài nguyên đồ họa
ULONG_PTR g_GdiplusToken;
DoubleBuffer g_BackBuffer = {0};
double g_LastRenderMs = 0.0;
double g_LastUpdateMs = 0.0;
double g_LastBlitMs = 0.0;
double g_LastSleepMs = 0.0;
bool g_NeedsRedraw = true;
HANDLE g_FrameTimer = NULL;

static bool ShouldAnimateScreen(ScreenState screen)
{
    switch (screen)
    {
    case SCREEN_MENU:
    case SCREEN_PLAY:
    case SCREEN_SETTING:
    case SCREEN_MATCH_CONFIG:
    case SCREEN_LOAD_GAME:
    case SCREEN_GUIDE:
    case SCREEN_ABOUT:
        return true;
    default:
        return false;
    }
}

// --- Khai báo hàm Win32 ---
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // 1. Khởi tạo Đồ họa & Font
    timeBeginPeriod(1); // Tăng độ chính xác của bộ hẹn giờ hệ thống lên 1ms
    if (!InitGraphics(g_GdiplusToken))
    {
        return 0;
    }

    g_FrameTimer = CreateWaitableTimer(NULL, TRUE, NULL);

    // Thiết lập tỷ lệ màn hình ngay từ ban đầu
    UIScaler::Update(850, 750);
    GlobalFont::Initialize();

    // 2. Tải Cấu hình & Ngôn ngữ & Âm thanh
    LoadConfig(&g_Config, "Asset/config.ini");
    LoadLanguageFile(g_Config.currentLang); // Khởi tạo ngôn ngữ trước
    initAudioSystem();

    if (g_Config.isBgmEnabled)
    {
        playBgm("Asset/audio/c1.mp3");
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

    // 4. Tạo cửa sổ
    HWND hWnd = CreateWindowExW(
        0, CLASS_NAME, L"CARO: Champions League",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
        NULL, NULL, hInstance, NULL);

    if (hWnd == NULL)
    {
        return 0;
    }

    // Thiết lập Icon cho cửa sổ từ file ảnh
    {
        HICON hIcon = NULL;
        Gdiplus::Bitmap *bmp = Gdiplus::Bitmap::FromFile(L"Asset/icon.png");
        if (bmp && bmp->GetLastStatus() == Gdiplus::Ok)
        {
            bmp->GetHICON(&hIcon);
            SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
            SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
            delete bmp;
        }
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG msg = {};
    const double targetFrameSeconds = 1.0 / 60.0;
    auto lastTime = std::chrono::high_resolution_clock::now();
    double fpsTimer = 0.0;
    int fpsFrames = 0;
    double lastFps = 0.0;

    bool bRunning = true;
    while (bRunning)
    {
        // 1. Xử lý TOÀN BỘ tin nhắn đang chờ trong hàng đợi
        // Việc này ngăn hiện tượng "Starving" khi người dùng ấn giữ phím (input spam)
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                bRunning = false;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!bRunning)
            break;

        // 2. Cập nhật Logic & Render (LUÔN CHẠY sau khi xử lý tin nhắn)
        auto frameStart = std::chrono::high_resolution_clock::now();

        // Tính toán Delta Time (dt)
        auto currentTime = frameStart;
        std::chrono::duration<double> elapsed = currentTime - lastTime;
        lastTime = currentTime;
        double dt = elapsed.count();

        // Giới hạn dt tối đa để tránh "nhảy vọt" (v.d khi di chuyển cửa sổ)
        if (dt > 0.1)
            dt = 0.1;

        g_GlobalAnimTime += (float)dt;

        // Cập nhật Logic
        auto updateStart = std::chrono::high_resolution_clock::now();
        bool needsLogicRedraw = false;
        if (g_CurrentScreen == SCREEN_PLAY)
        {
            needsLogicRedraw = UpdatePlayLogic(&g_PlayState, dt);
        }
        else if (g_CurrentScreen == SCREEN_EXIT)
        {
            ShowWindow(hWnd, SW_HIDE); // Hide window immediately to feel instant
            bRunning = false;
        }
        auto updateEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> updateElapsed = updateEnd - updateStart;
        g_LastUpdateMs = updateElapsed.count();

        // Cập nhật FPS và tiêu đề cửa sổ (mỗi 0.5s)
        fpsTimer += dt;
        fpsFrames++;
        if (fpsTimer >= 0.5)
        {
            lastFps = fpsFrames / fpsTimer;
            fpsFrames = 0;
            fpsTimer = 0.0;

            std::wstringstream title;
            title.setf(std::ios::fixed);
            title.precision(1);
            title << L"CARO: Champions League";
            title << L" | FPS: " << (int)lastFps;
            title << L" | Upd: " << g_LastUpdateMs << L" ms";
            title << L" | Ren: " << g_LastRenderMs << L" ms";
            title << L" | Blt: " << g_LastBlitMs << L" ms";
            title << L" | Slp: " << g_LastSleepMs << L" ms";
            SetWindowTextW(hWnd, title.str().c_str());
        }

        // Ép vẽ lại toàn cục mỗi khung hình
        if (!IsIconic(hWnd))
        {
            if (ShouldAnimateScreen(g_CurrentScreen) || needsLogicRedraw)
            {
                g_NeedsRedraw = true;
            }

            if (g_NeedsRedraw)
            {
                InvalidateRect(hWnd, NULL, FALSE);
                UpdateWindow(hWnd);
            }
        }

        // 3. Điều khiển tốc độ khung hình (Throttling)
        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> frameElapsed = frameEnd - frameStart;

        if (frameElapsed.count() < targetFrameSeconds)
        {
            double sleepTime = targetFrameSeconds - frameElapsed.count();
            auto sleepStart = std::chrono::high_resolution_clock::now();
            if (sleepTime > 0.0005)
            {
                if (g_FrameTimer)
                {
                    LARGE_INTEGER dueTime;
                    dueTime.QuadPart = -(LONGLONG)(sleepTime * 10000000.0);
                    if (SetWaitableTimer(g_FrameTimer, &dueTime, 0, NULL, NULL, FALSE))
                    {
                        WaitForSingleObject(g_FrameTimer, INFINITE);
                    }
                    else
                    {
                        Sleep((DWORD)(sleepTime * 1000.0));
                    }
                }
                else
                {
                    Sleep((DWORD)(sleepTime * 1000.0));
                }
            }
            auto sleepEnd = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double, std::milli> sleepElapsed = sleepEnd - sleepStart;
            g_LastSleepMs = sleepElapsed.count();
        }
        else
        {
            g_LastSleepMs = 0.0;
        }
    }

    // 7. Giải phóng tài nguyên
    ShowWindow(hWnd, SW_HIDE); // Safety: ensure window is hidden before slow cleanup

    GlobalFont::Cleanup();
    ClearUICaches();
    DeleteBuffer(g_BackBuffer);

    if (g_FrameTimer)
    {
        CloseHandle(g_FrameTimer);
        g_FrameTimer = NULL;
    }

    // Gỡ font khỏi bộ nhớ
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Regular.ttf", FR_PRIVATE, 0);
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Bold.ttf", FR_PRIVATE, 0);
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Black.ttf", FR_PRIVATE, 0);
    RemoveFontResourceExW(L"Asset/font/Be_Vietnam_Pro/BeVietnamPro-Italic.ttf", FR_PRIVATE, 0);

    shutdownAudioSystem();
    ShutdownGraphics(g_GdiplusToken);

    timeEndPeriod(1); // Trả lại độ chính xác mặc định cho hệ thống

    return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    {
        int w = LOWORD(lParam);
        int h = HIWORD(lParam);
        UIScaler::Update(w, h);
        GlobalFont::RebuildFonts();

        // Cập nhật lại Back Buffer khi kích thước cửa sổ thay đổi
        HDC hdc = GetDC(hWnd);
        DeleteBuffer(g_BackBuffer);
        CreateBuffer(hWnd, hdc, g_BackBuffer);
        ReleaseDC(hWnd, hdc);

        InvalidateRect(hWnd, NULL, FALSE);
        g_NeedsRedraw = true;
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    case WM_KEYDOWN:
    {
        bool isRepeat = (lParam & (1 << 30)) != 0;
        WPARAM extendedWParam = wParam | (isRepeat ? 0x20000 : 0);

        switch (g_CurrentScreen)
        {
        case SCREEN_MENU:
            UpdateMenuScreen(g_CurrentScreen, g_MenuSelected, extendedWParam);
            if (g_CurrentScreen == SCREEN_PLAY)
            {
                g_CurrentScreen = SCREEN_MATCH_CONFIG;
                g_ConfigSelected = 0;
                g_PlayState.gameMode = MODE_CARO;
                g_PlayState.matchType = MATCH_PVP;
                g_PlayState.difficulty = 2;
                g_PlayState.countdownTime = 30;
                g_PlayState.targetScore = 1;
            }
            break;

        case SCREEN_MATCH_CONFIG:
            UpdateMatchConfigScreen(g_CurrentScreen, &g_PlayState, g_ConfigSelected, extendedWParam);
            if (wParam == VK_ESCAPE)
            {
                g_CurrentScreen = SCREEN_MENU;
            }
            break;
        case SCREEN_PLAY:
            UpdatePlayScreen(&g_PlayState, g_CurrentScreen, extendedWParam, &g_Config);
            break;
        case SCREEN_SETTING:
            UpdateSettingScreen(g_CurrentScreen, &g_Config, g_SettingSelected, extendedWParam);
            break;
        case SCREEN_LOAD_GAME:
            UpdateLoadGameScreen(g_CurrentScreen, &g_PlayState, g_LoadSelected, g_LoadStatus, extendedWParam);
            break;
        case SCREEN_GUIDE:
            UpdateGuildScreen(g_CurrentScreen, g_GuildPage, extendedWParam);
            break;

        case SCREEN_ABOUT:
            UpdateAboutScreen(g_CurrentScreen, extendedWParam);
            break;
        case SCREEN_EXIT:
            PostQuitMessage(0);
            break;
        }
        g_NeedsRedraw = true;
        break;
    }

    case WM_CHAR:
    {
        switch (g_CurrentScreen)
        {
        case SCREEN_MATCH_CONFIG:
            UpdateMatchConfigScreen(g_CurrentScreen, &g_PlayState, g_ConfigSelected, wParam | 0x10000);
            break;
        case SCREEN_PLAY:
            UpdatePlayScreen(&g_PlayState, g_CurrentScreen, wParam | 0x10000, &g_Config);
            break;
        case SCREEN_LOAD_GAME:
            UpdateLoadGameScreen(g_CurrentScreen, &g_PlayState, g_LoadSelected, g_LoadStatus, wParam | 0x10000);
            break;
        }
        g_NeedsRedraw = true;
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        if (IsIconic(hWnd))
        {
            EndPaint(hWnd, &ps);
            break;
        }

        RECT clientRect;
        GetClientRect(hWnd, &clientRect);
        int w = clientRect.right - clientRect.left;
        int h = clientRect.bottom - clientRect.top;

        if (w <= 0 || h <= 0)
        {
            EndPaint(hWnd, &ps);
            break;
        }

        if (g_BackBuffer.hdcMem == nullptr)
        {
            CreateBuffer(hWnd, hdc, g_BackBuffer);
        }

        auto renderStart = std::chrono::high_resolution_clock::now();

        switch (g_CurrentScreen)
        {
        case SCREEN_MENU:
            RenderMenuScreen(g_BackBuffer.hdcMem, (int)g_MenuSelected, w, h);
            break;
        case SCREEN_PLAY:
            RenderPlayScreen(g_BackBuffer.hdcMem, &g_PlayState, w, h, &g_Config);
            break;
        case SCREEN_SETTING:
            RenderSettingScreen(g_BackBuffer.hdcMem, &g_Config, g_SettingSelected, w, h);
            break;
        case SCREEN_LOAD_GAME:
            RenderLoadGameScreen(g_BackBuffer.hdcMem, g_LoadSelected, g_LoadStatus, w, h);
            break;
        case SCREEN_MATCH_CONFIG:
            RenderMatchConfigScreen(g_BackBuffer.hdcMem, g_ConfigSelected, &g_PlayState, w, h);
            break;
        case SCREEN_GUIDE:
            RenderGuildScreen(g_BackBuffer.hdcMem, w, h, g_GuildPage);
            break;

        case SCREEN_ABOUT:
            RenderAboutScreen(g_BackBuffer.hdcMem, w, h);
            break;
        }

        auto blitStart = std::chrono::high_resolution_clock::now();
        BitBlt(hdc, 0, 0, w, h, g_BackBuffer.hdcMem, 0, 0, SRCCOPY);
        auto blitEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> blitElapsed = blitEnd - blitStart;
        g_LastBlitMs = blitElapsed.count();

        auto renderEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> renderElapsed = renderEnd - renderStart;
        g_LastRenderMs = renderElapsed.count();

        g_NeedsRedraw = false;

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}