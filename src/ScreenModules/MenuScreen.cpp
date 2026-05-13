#include "MenuScreen.h"
#include "../SystemModules/AudioSystem.h"
#include "../RenderAPI/UIComponents.h"
#include "../SystemModules/Localization.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../RenderAPI/PixelLayout.h"
#include <cmath>
#include <map>

/** @file MenuScreen.cpp
 *  @brief Màn menu chính: xử lý điều hướng menu và vẽ giao diện menu.
 *
 *  Chức năng chính:
 *  - `ProcessMenuInput`: xử lý phím người dùng (di chuyển, chọn mục).
 *  - `RenderMenuScreen`: vẽ danh sách mục menu và hiệu ứng tiêu đề.
 */

const int TOTAL_MENU_ITEMS = 6;

/** @brief Wrapper xử lý sự kiện cho menu (gọi `ProcessMenuInput`).
 *  @param currentState Tham chiếu trạng thái màn hình (có thể đổi màn hình từ hàm này).
 *  @param selectedOption Tham chiếu mục đang chọn.
 *  @param wParam Mã phím/flags từ WM_KEY/WM_CHAR.
 */
bool UpdateMenuScreen(ScreenState &currentState, int &selectedOption, WPARAM wParam)
{
    if (wParam == 0)
        return false;
    return ProcessMenuInput(wParam, currentState, selectedOption);
}

/** @brief Xử lý phím người dùng trên màn menu.
 *  @param wParam Mã phím/flags (WM_KEY/WM_CHAR encoded).
 *  @param currentState Tham chiếu trạng thái màn hình (có thể chuyển màn hình nếu chọn mục).
 *  @param selectedOption Tham chiếu mục được chọn (cập nhật khi di chuyển).
 *  @return `true` nếu trạng thái menu thay đổi và cần cập nhật UI.
 */
bool ProcessMenuInput(WPARAM wParam, ScreenState &currentState, int &selectedOption)
{
    bool hasChanged = false;

    // Phát hiện nút giữ (Autorepeat) từ bit 0x20000
    bool isRepeat = (wParam & 0x20000) != 0;
    WPARAM rawKey = wParam & 0xFFFF;

    // Throttling: Giới hạn 80ms cho phím nhấn tay, 150ms cho phím giữ (Repeat)
    static ULONGLONG lastMoveTime = 0;
    ULONGLONG now = GetTickCount64();
    bool canMove = (now - lastMoveTime > (ULONGLONG)(isRepeat ? 150 : 80));

    if (rawKey == 'W' || rawKey == 'w' || rawKey == VK_UP)
    {
        if (!canMove)
            return false;
        selectedOption--;
        if (selectedOption < 0)
        {
            selectedOption = TOTAL_MENU_ITEMS - 1;
        }
        if (!isRepeat)
            playSfx("sfx_move");
        lastMoveTime = now;
        hasChanged = true;
    }
    else if (rawKey == 'S' || rawKey == 's' || rawKey == VK_DOWN)
    {
        if (!canMove)
            return false;
        selectedOption++;
        if (selectedOption >= TOTAL_MENU_ITEMS)
        {
            selectedOption = 0;
        }
        if (!isRepeat)
            playSfx("sfx_move");
        lastMoveTime = now;
        hasChanged = true;
    }
    else if (wParam == VK_RETURN || wParam == VK_SPACE)
    {
        playSfx("sfx_select");
        switch (selectedOption)
        {
        case 0:
            currentState = SCREEN_PLAY;
            break;
        case 1:
            currentState = SCREEN_LOAD_GAME;
            break;
        case 2:
            currentState = SCREEN_SETTING;
            break;
        case 3:
            currentState = SCREEN_GUIDE;
            break;
        case 4:
            currentState = SCREEN_ABOUT;
            break;
        case 5:
            currentState = SCREEN_EXIT;
            break;
        }
        hasChanged = true;
    }
    return hasChanged;
}

/** @brief Vẽ màn hình menu chính.
 *  @param hdc Device context để vẽ.
 *  @param selectedOption Mục đang được chọn (để highlight và hiệu ứng).
 *  @param screenWidth, screenHeight Kích thước vùng vẽ.
 */
void RenderMenuScreen(HDC hdc, int selectedOption, int screenWidth, int screenHeight)
{
    Gdiplus::Graphics g(hdc);
    PixelLayout::ApplyPixelArtBlit(g);

    // 0. Nền sân vận động (showFlashes=true: chỉ Menu mới có hiệu ứng tia chớp khán đài)
    DrawProceduralStadium(g, screenWidth, screenHeight, true);
    Gdiplus::SolidBrush lightGlassBrush(Gdiplus::Color(80, 255, 255, 255));
    g.FillRectangle(&lightGlassBrush, 0, 0, screenWidth, screenHeight);

    int cupYOffset = UIScaler::SY((int)(PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 2.5f) * 10.0f));
    DrawPixelTrophy(g, screenWidth / 2, screenHeight / 4 - UIScaler::SY(85) + cupYOffset, UIScaler::S(100));

    static PixelModel titleModel;
    static std::map<int, Gdiplus::Color> titlePalette;
    if (!titleModel.isLoaded)
    {
        titleModel = LoadPixelModel("Asset/models/bg/title_caro.txt");
        titlePalette[1] = ToGdiColor(Theme::TitleBorder);
        titlePalette[2] = ToGdiColor(Theme::TitleFill);
        titlePalette[3] = ToGdiColor(Theme::TitleShadow);
    }
    int titleYOffset = UIScaler::SY((int)(PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 2.0f) * 6.0f));

    DrawPixelModel(g, titleModel, screenWidth / 2, screenHeight / 4 + UIScaler::SY(20) + titleYOffset, UIScaler::S(500), titlePalette);

    DrawTextCenteredOutlined(hdc, L"CHAMPIONS LEAGUE", screenHeight / 4 + UIScaler::SY(75) + titleYOffset, screenWidth, ToCOLORREF(Palette::BlueDarkest), RGB(255, 255, 255), GlobalFont::Title);

    int startY = screenHeight / 2 + UIScaler::SY(10);
    int spacing = UIScaler::SY(55);

    std::wstring menuItems[TOTAL_MENU_ITEMS] = {
        GetText("menu_play"), GetText("menu_load"), GetText("menu_settings"),
        GetText("menu_guild"), GetText("menu_about"), GetText("menu_exit")};

    for (int i = 0; i < TOTAL_MENU_ITEMS; i++)
    {
        int currentY = startY + i * spacing;

        if (i == selectedOption)
        {
            std::wstring highlightedText = std::wstring(menuItems[i]);

            // Hiệu ứng màu nhấp nháy cho mục đang chọn (Xanh lam -> Cyan)
            int gCol = (int)(180 + PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 8.f) * 75);
            COLORREF dynColor = RGB(0, max(0, min(255, gCol)), 255);

            int wStrOffset = UIScaler::S((int)highlightedText.length() * 18 + 70);
            DrawPixelFootball(g, screenWidth / 2 - wStrOffset, currentY + UIScaler::SY(38), UIScaler::S(48));
            DrawPixelFootball(g, screenWidth / 2 + wStrOffset, currentY + UIScaler::SY(38), UIScaler::S(48));
            DrawTextCenteredOutlined(hdc, highlightedText, currentY, screenWidth, dynColor, RGB(255, 255, 255), GlobalFont::Title);
        }
        else
        {
            DrawTextCenteredOutlined(hdc, menuItems[i], currentY + UIScaler::SY(6), screenWidth, ToCOLORREF(Palette::GrayDarkest), RGB(255, 255, 255), GlobalFont::Bold);
        }
    }
    DrawTextCenteredOutlined(hdc, GetText("menu_instruct"), screenHeight - UIScaler::SY(50), screenWidth, ToCOLORREF(Palette::GrayDark), RGB(255, 255, 255), GlobalFont::Note);
}