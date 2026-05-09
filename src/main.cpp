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
#include "SystemModules/Profiler.h"
#include "SystemModules/Localization.h"
#include "RenderAPI/Colours.h"
#include "RenderAPI/UIScaler.h"
#include "RenderAPI/Renderer.h"
#include "RenderAPI/DirtyRect.h"
#include "RenderAPI/UIComponents.h"
#include "GameLogic/GameEngine.h"
#include "SystemModules/EngineStats.h"

// --- Khai báo các Header màn hình ---
#include "ScreenModules/MenuScreen.h"
#include "ScreenModules/PlayScreen.h"
#include "ScreenModules/SettingScreen.h"
#include "ScreenModules/LoadGameScreen.h"
#include "ScreenModules/MatchConfigScreen.h"
#include "ScreenModules/GuildScreen.h"
#include "ScreenModules/AboutScreen.h"

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
    UIScaler::Update((int)UIScaler::BASE_WIDTH, (int)UIScaler::BASE_HEIGHT);
    GlobalFont::Initialize();

    // Initialize profiler if profiling_on.txt is present in workspace root
    Profiler::InitIfRequested("D:\\Code\\Gomoku_Game_Project");

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

    // FPS target lấy từ config (30 hoặc 60)
    EngineStats::Initialize(static_cast<double>(g_Config.fpsLimit));
    MSG msg = {};
    bool bRunning = true;

    while (bRunning)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) { bRunning = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!bRunning) break;

        // Bắt đầu khung hình và lấy Delta Time
        double dt = EngineStats::BeginFrame();
        g_GlobalAnimTime += (float)dt;

        // profiler helpers for this frame
        int profiler_rectCount = 0;
        double profiler_dirtyArea = 0.0;

        // Cập nhật Logic
        auto updateStart = std::chrono::high_resolution_clock::now();
        bool needsLogicRedraw = false;
        if (g_CurrentScreen == SCREEN_PLAY)
        {
            needsLogicRedraw = UpdatePlayLogic(&g_PlayState, dt);
        }
        else if (g_CurrentScreen == SCREEN_EXIT)
        {
            ShowWindow(hWnd, SW_HIDE);
            bRunning = false;
        }
        g_LastUpdateMs = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - updateStart).count();

        // Cập nhật thông số hiệu năng lên tiêu đề cửa sổ
        EngineStats::UpdateTitleStats(hWnd, dt);

        if (!IsIconic(hWnd))
        {
            if (ShouldAnimateScreen(g_CurrentScreen) || needsLogicRedraw)
                g_NeedsRedraw = true;

            if (g_NeedsRedraw)
            {
                // Prefer partial invalidation: resolve board cells and invalidate only dirty rects
                RECT clientRect;
                GetClientRect(hWnd, &clientRect);
                int w = clientRect.right - clientRect.left;
                int h = clientRect.bottom - clientRect.top;

                if (g_CurrentScreen == SCREEN_PLAY)
                {
                    // Compute board layout same as PlayScreen so we can resolve logical cells
                    int availableHeight = h - UIScaler::SY(120);
                    int availableWidth = w - UIScaler::SX(300);
                    int maxBoardSize = availableWidth < availableHeight ? availableWidth : availableHeight;
                    int minBoardSize = UIScaler::S(200);
                    if (maxBoardSize < minBoardSize)
                        maxBoardSize = minBoardSize;
                    int dynamicCellSize = maxBoardSize / g_PlayState.boardSize;
                    int boardPixelSize = g_PlayState.boardSize * dynamicCellSize;
                    int startX = (w - boardPixelSize) / 2;
                    int startY = (h - boardPixelSize) / 2 + UIScaler::SY(40);

                    DirtyRect::ResolveBoardCells(dynamicCellSize, startX, startY);
                    auto rects = DirtyRect::StealAndClear();
                    DirtyRect::MergeAndClip(rects, clientRect);

                    // Compute telemetry for profiler: rect count and total dirty area
                    if (rects.empty())
                    {
                        InvalidateRect(hWnd, NULL, FALSE);
                        profiler_rectCount = -1; // indicates full invalidate
                        profiler_dirtyArea = (double)(w) * (double)(h);
                    }
                    else
                    {
                        double totalArea = 0.0;
                        for (auto &r : rects)
                        {
                            InvalidateRect(hWnd, &r, FALSE);
                            double rw = (double)max(0, r.right - r.left);
                            double rh = (double)max(0, r.bottom - r.top);
                            totalArea += rw * rh;
                        }
                        profiler_rectCount = (int)rects.size();
                        profiler_dirtyArea = totalArea;
                    }
                }
                else
                {
                    InvalidateRect(hWnd, NULL, FALSE);
                }

                UpdateWindow(hWnd);
            }
        }

        // Kết thúc khung hình (Throttling)
        EngineStats::EndFrame();

        // Profiling: log last blit duration and dirty-rect stats if enabled
        Profiler::LogFrame(g_LastBlitMs, profiler_rectCount, profiler_dirtyArea);
    }

    // 7. Giải phóng tài nguyên
    Profiler::Shutdown();
    ShowWindow(hWnd, SW_HIDE); // Safety: ensure window is hidden before slow cleanup

    GlobalFont::Cleanup();
    ClearUICaches();
    DeleteBuffer(g_BackBuffer);

    if (g_FrameTimer)
    {
        CloseHandle(g_FrameTimer);
        g_FrameTimer = NULL;
    }

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

        if (g_BackBuffer.hdcMemory == nullptr)
        {
            CreateBuffer(hWnd, hdc, g_BackBuffer);
        }

        auto renderStart = std::chrono::high_resolution_clock::now();

        // Use the update region(s) provided by Windows and render/blit only those rects.
        HRGN hUpdateRgn = CreateRectRgn(0, 0, 0, 0);
        int rgnType = GetUpdateRgn(hWnd, hUpdateRgn, FALSE);

        double totalBlitMs = 0.0;

        if (rgnType == NULLREGION)
        {
            // No explicit update region — fallback to full render+blit to avoid leaving backbuffer stale
            switch (g_CurrentScreen)
            {
            case SCREEN_MENU:
                RenderMenuScreen(g_BackBuffer.hdcMemory, (int)g_MenuSelected, w, h);
                break;
            case SCREEN_PLAY:
                RenderPlayScreen(g_BackBuffer.hdcMemory, &g_PlayState, w, h, &g_Config);
                break;
            case SCREEN_SETTING:
                RenderSettingScreen(g_BackBuffer.hdcMemory, &g_Config, g_SettingSelected, w, h);
                break;
            case SCREEN_LOAD_GAME:
                RenderLoadGameScreen(g_BackBuffer.hdcMemory, g_LoadSelected, g_LoadStatus, w, h);
                break;
            case SCREEN_MATCH_CONFIG:
                RenderMatchConfigScreen(g_BackBuffer.hdcMemory, g_ConfigSelected, &g_PlayState, w, h);
                break;
            case SCREEN_GUIDE:
                RenderGuildScreen(g_BackBuffer.hdcMemory, w, h, g_GuildPage);
                break;
            case SCREEN_ABOUT:
                RenderAboutScreen(g_BackBuffer.hdcMemory, w, h);
                break;
            }

            auto blitStartFull = std::chrono::high_resolution_clock::now();
            BitBlt(hdc, 0, 0, w, h, g_BackBuffer.hdcMemory, 0, 0, SRCCOPY);
            auto blitEndFull = std::chrono::high_resolution_clock::now();
            totalBlitMs = std::chrono::duration<double, std::milli>(blitEndFull - blitStartFull).count();
        }
        else if (rgnType == ERROR)
        {
            // Fallback to full render+blit on error
            switch (g_CurrentScreen)
            {
            case SCREEN_MENU:
                RenderMenuScreen(g_BackBuffer.hdcMemory, (int)g_MenuSelected, w, h);
                break;
            case SCREEN_PLAY:
                RenderPlayScreen(g_BackBuffer.hdcMemory, &g_PlayState, w, h, &g_Config);
                break;
            case SCREEN_SETTING:
                RenderSettingScreen(g_BackBuffer.hdcMemory, &g_Config, g_SettingSelected, w, h);
                break;
            case SCREEN_LOAD_GAME:
                RenderLoadGameScreen(g_BackBuffer.hdcMemory, g_LoadSelected, g_LoadStatus, w, h);
                break;
            case SCREEN_MATCH_CONFIG:
                RenderMatchConfigScreen(g_BackBuffer.hdcMemory, g_ConfigSelected, &g_PlayState, w, h);
                break;
            case SCREEN_GUIDE:
                RenderGuildScreen(g_BackBuffer.hdcMemory, w, h, g_GuildPage);
                break;
            case SCREEN_ABOUT:
                RenderAboutScreen(g_BackBuffer.hdcMemory, w, h);
                break;
            }

            auto blitStartFull = std::chrono::high_resolution_clock::now();
            BitBlt(hdc, 0, 0, w, h, g_BackBuffer.hdcMemory, 0, 0, SRCCOPY);
            auto blitEndFull = std::chrono::high_resolution_clock::now();
            totalBlitMs = std::chrono::duration<double, std::milli>(blitEndFull - blitStartFull).count();
        }
        else
        {
            // Get region data (may contain multiple rects)
            DWORD needed = GetRegionData(hUpdateRgn, 0, NULL);
            if (needed == 0)
                needed = sizeof(RGNDATA) + 16 * sizeof(RECT);

            std::vector<BYTE> buf(needed);
            RGNDATA* prd = reinterpret_cast<RGNDATA*>(buf.data());
            DWORD got = GetRegionData(hUpdateRgn, needed, prd);
            int rectCount = (prd && prd->rdh.nCount > 0) ? prd->rdh.nCount : 0;
            RECT* pRects = (RECT*)(prd->Buffer);

            // For non-play screens render full backbuffer once, then blit per-rect
            bool renderedFullForNonPlay = false;
            if (g_CurrentScreen != SCREEN_PLAY)
            {
                switch (g_CurrentScreen)
                {
                case SCREEN_MENU:
                    RenderMenuScreen(g_BackBuffer.hdcMemory, (int)g_MenuSelected, w, h);
                    break;
                case SCREEN_SETTING:
                    RenderSettingScreen(g_BackBuffer.hdcMemory, &g_Config, g_SettingSelected, w, h);
                    break;
                case SCREEN_LOAD_GAME:
                    RenderLoadGameScreen(g_BackBuffer.hdcMemory, g_LoadSelected, g_LoadStatus, w, h);
                    break;
                case SCREEN_MATCH_CONFIG:
                    RenderMatchConfigScreen(g_BackBuffer.hdcMemory, g_ConfigSelected, &g_PlayState, w, h);
                    break;
                case SCREEN_GUIDE:
                    RenderGuildScreen(g_BackBuffer.hdcMemory, w, h, g_GuildPage);
                    break;
                case SCREEN_ABOUT:
                    RenderAboutScreen(g_BackBuffer.hdcMemory, w, h);
                    break;
                default:
                    break;
                }
                renderedFullForNonPlay = true;
            }

            for (int i = 0; i < rectCount; ++i)
            {
                RECT rc = pRects[i];
                // Intersect with client area
                RECT clipped = rc;
                IntersectRect(&clipped, &clipped, &clientRect);
                if (clipped.right <= clipped.left || clipped.bottom <= clipped.top)
                    continue;

                if (g_CurrentScreen == SCREEN_PLAY)
                {
                    // Render only the clip area into backbuffer
                    RenderPlayScreen(g_BackBuffer.hdcMemory, &g_PlayState, w, h, &g_Config, &clipped);
                }
                // Blit only the updated rect from backbuffer to window
                auto blitStart = std::chrono::high_resolution_clock::now();
                BitBlt(hdc, clipped.left, clipped.top, clipped.right - clipped.left, clipped.bottom - clipped.top,
                    g_BackBuffer.hdcMemory, clipped.left, clipped.top, SRCCOPY);
                auto blitEnd = std::chrono::high_resolution_clock::now();
                totalBlitMs += std::chrono::duration<double, std::milli>(blitEnd - blitStart).count();
            }

            // If there were no rects (shouldn't happen), fallback to full blit
            if (rectCount == 0 && !renderedFullForNonPlay)
            {
                RenderPlayScreen(g_BackBuffer.hdcMemory, &g_PlayState, w, h, &g_Config, &clientRect);
                auto blitStartFull = std::chrono::high_resolution_clock::now();
                BitBlt(hdc, 0, 0, w, h, g_BackBuffer.hdcMemory, 0, 0, SRCCOPY);
                auto blitEndFull = std::chrono::high_resolution_clock::now();
                totalBlitMs = std::chrono::duration<double, std::milli>(blitEndFull - blitStartFull).count();
            }
        }

        g_LastBlitMs = totalBlitMs;

        auto renderEnd = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> renderElapsed = renderEnd - renderStart;
        g_LastRenderMs = renderElapsed.count();

        g_NeedsRedraw = false;

        DeleteObject(hUpdateRgn);
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