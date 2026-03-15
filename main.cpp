#include <windows.h>
#include <string>

// --- Ép Linker sử dụng int main() làm điểm bắt đầu nhưng chạy dưới Subsystem Windows ---
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

// --- Include Kiểu dữ liệu ---
#include "ApplicationTypes/GameState.h"
#include "ApplicationTypes/GameConfig.h"
#include "ApplicationTypes/PlayState.h"

// --- Include Hệ thống (System) ---
#include "SystemModules/ConfigLoader.h"
#include "SystemModules/AudioSystem.h"
#include "SystemModules/Localization.h"
#include "SystemModules/TimeSystem.h"
#include "SystemModules/SaveLoadSystem.h"
#include "GameLogic/StateUpdater.h"

// --- Include Đồ họa (Render API) ---
#include "RenderAPI/Renderer.h"
#include "RenderAPI/UIComponents.h"
#include "RenderAPI/Colours.h"

// --- Include Các Màn hình (Screen Modules) ---
#include "ScreenModules/MenuScreen.h"
#include "ScreenModules/PlayScreen.h"
#include "ScreenModules/SettingScreen.h"
#include "ScreenModules/LoadGameScreen.h"

// --- BIẾN TOÀN CỤC ---
ScreenState currentState = SCREEN_MENU;
GameConfig config;
PlayState playState;

int menuSelection = 0;
int settingSelection = 0;
int loadSelection = 0;
std::wstring loadStatusMsg = L"";

ULONG_PTR gdiplusToken;
Sprite spriteX;
Sprite spriteO;

// --- HÀM XỬ LÝ THÔNG ĐIỆP WINDOWS ---
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        // 1. Khởi tạo cấu hình và hệ thống
        LoadConfig(&config, "Asset/config.ini");
        InitAudio(&config);
        LoadLanguageFile(config.currentLang);
        InitTimeSystem();

        // 2. Thiết lập ván đấu mặc định
        InitNewMatch(&playState, MODE_CARO, MATCH_PVP, 12, 30);
        PlayBGM("Asset/audio/bgm_menu.wav");

        // 3. Nạp tài nguyên hình ảnh
        spriteX = LoadPNG(L"Asset/images/x.png");
        spriteO = LoadPNG(L"Asset/images/o.png");

        ScaleSprite(spriteX, CELL_SIZE, CELL_SIZE);
        ScaleSprite(spriteO, CELL_SIZE, CELL_SIZE);

        break;

    case WM_KEYDOWN:
        // Định tuyến xử lý phím nhấn dựa trên màn hình hiện tại
        switch (currentState) {
        case SCREEN_MENU:
            UpdateMenuScreen(currentState, menuSelection, wParam);
            break;
        case SCREEN_PLAY:
            UpdatePlayScreen(&playState, currentState, wParam);
            break;
        case SCREEN_SETTING:
            UpdateSettingScreen(currentState, &config, settingSelection, wParam);
            break;
        case SCREEN_LOAD_GAME:
            UpdateLoadGameScreen(currentState, &playState, loadSelection, loadStatusMsg, wParam);
            break;
        case SCREEN_ABOUT:
        case SCREEN_GUIDE:
            if (wParam == VK_ESCAPE) currentState = SCREEN_MENU;
            break;
        default:
            break;
        }
        // Yêu cầu vẽ lại màn hình ngay sau khi cập nhật trạng thái logic
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // 1. Tạo bộ đệm kép (Double Buffer) trên RAM
        DoubleBuffer buffer;
        CreateBuffer(hwnd, hdc, buffer);

        // 2. Làm sạch nền bộ đệm bằng màu trắng
        RECT clientRect = { 0, 0, buffer.width, buffer.height };
        HBRUSH hBgBrush = CreateSolidBrush(Colour::WHITE);
        FillRect(buffer.hdcMem, &clientRect, hBgBrush);
        DeleteObject(hBgBrush);

        // 3. Vẽ giao diện (Render) tùy theo trạng thái
        switch (currentState) {
        case SCREEN_MENU:
            RenderMenuScreen(buffer.hdcMem, menuSelection, buffer.width, buffer.height);
            break;
        case SCREEN_PLAY:
            RenderPlayScreen(buffer.hdcMem, &playState, buffer.width, buffer.height, spriteX, spriteO);
            break;
        case SCREEN_LOAD_GAME:
            RenderLoadGameScreen(buffer.hdcMem, loadSelection, loadStatusMsg, buffer.width, buffer.height);
            break;
        case SCREEN_SETTING:
            RenderSettingScreen(buffer.hdcMem, &config, settingSelection, buffer.width, buffer.height);
            break;
        case SCREEN_ABOUT:
            DrawTextCentered(buffer.hdcMem, L"ĐỒ ÁN CỜ CARO - CS161", buffer.height / 2, buffer.width, Colour::GRAY_DARKEST, GlobalFont::Title);
            DrawTextCentered(buffer.hdcMem, L"Nhấn ESC để quay lại", buffer.height / 2 + 50, buffer.width, Colour::GRAY_NORMAL, GlobalFont::Default);
            break;
        case SCREEN_GUIDE:
            DrawTextCentered(buffer.hdcMem, L"W,A,S,D: DI CHUYỂN | ENTER: ĐÁNH CỜ", buffer.height / 2, buffer.width, Colour::BLUE_DARKEST, GlobalFont::Bold);
            DrawTextCentered(buffer.hdcMem, L"Nhấn ESC để quay lại", buffer.height / 2 + 50, buffer.width, Colour::GRAY_NORMAL, GlobalFont::Default);
            break;
        default:
            break;
        }

        // 4. Đổ bộ đệm (BitBlt) ra màn hình vật lý (Chống nháy/Tearing)
        BitBlt(hdc, 0, 0, buffer.width, buffer.height, buffer.hdcMem, 0, 0, SRCCOPY);

        // 5. Giải phóng tài nguyên GDI cục bộ
        DeleteBuffer(buffer);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_DESTROY:
        // Dọn dẹp tài nguyên trước khi tắt tiến trình
        StopBGM();
        SaveConfig(&config, "Asset/config.ini");

        FreeSprite(spriteX);
        FreeSprite(spriteO);

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// --- HÀM MAIN TIÊU CHUẨN ---
int main() {
    // Lấy Handle của tiến trình hiện tại
    HINSTANCE hInstance = GetModuleHandle(NULL);

    // Khởi tạo hệ thống GDI+ và Font toàn cục
    if (!InitGraphics(gdiplusToken)) return -1;
    GlobalFont::Initialize();

    // Đăng ký lớp cửa sổ Windows
    const wchar_t CLASS_NAME[] = L"CaroGameApp";
    WNDCLASSW wc = { };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClassW(&wc);

    // Căn chỉnh kích thước (Đảm bảo vùng Client Area thực tế là 1000x700)
    RECT wr = { 0, 0, 1000, 700 };
    AdjustWindowRect(&wr, WS_OVERLAPPEDWINDOW, FALSE);

    // Tạo cửa sổ (Đã khóa các nút thu phóng và kéo thả viền)
    HWND hwnd = CreateWindowExW(
        0, CLASS_NAME, L"ĐỒ ÁN CARO & TIC-TAC-TOE",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;
    ShowWindow(hwnd, SW_SHOW);

    // Vòng lặp thông điệp (Blocking Message Loop)
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Xóa bộ nhớ toàn cục khi nhận lệnh Quit
    GlobalFont::Cleanup();
    ShutdownGraphics(gdiplusToken);

    return (int)msg.wParam;
}