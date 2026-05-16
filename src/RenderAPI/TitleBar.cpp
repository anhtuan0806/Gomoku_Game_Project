/** @file TitleBar.cpp
 *  @brief Triển khai thanh tiêu đề tùy chỉnh (Custom Title Bar).
 */
#include "TitleBar.h"
#include "Colours.h"
#include "UIScaler.h"
#include "Renderer.h"
#include "../ApplicationTypes/GameConstants.h"
#include <gdiplus.h>
#include <algorithm>
#include <cmath>

namespace
{
    struct TitleBarLayout
    {
        RECT bar = {};
        RECT icon = {};
        RECT minBtn = {};
        RECT maxBtn = {};
        RECT closeBtn = {};
        int textLeft = 0;
        int textRight = 0;
    };

    TitleBarLayout BuildLayout(int screenWidth)
    {
        TitleBarLayout layout;
        int barHeight = UIScaler::SY(TITLE_BAR_BASE_HEIGHT);
        int paddingX = UIScaler::SX(TITLE_BAR_PADDING_X);
        int iconSize = UIScaler::S(TITLE_BAR_ICON_SIZE);
        int buttonSize = UIScaler::S(TITLE_BAR_BUTTON_SIZE);
        int buttonGap = UIScaler::SX(TITLE_BAR_BUTTON_GAP);
        int textGap = UIScaler::SX(TITLE_BAR_TEXT_GAP);

        int buttonTop = (barHeight - buttonSize) / 2;
        int right = screenWidth - paddingX;

        layout.closeBtn = {right - buttonSize, buttonTop, right, buttonTop + buttonSize};
        right -= buttonSize + buttonGap;
        layout.maxBtn = {right - buttonSize, buttonTop, right, buttonTop + buttonSize};
        right -= buttonSize + buttonGap;
        layout.minBtn = {right - buttonSize, buttonTop, right, buttonTop + buttonSize};

        int iconTop = (barHeight - iconSize) / 2;
        layout.icon = {paddingX, iconTop, paddingX + iconSize, iconTop + iconSize};
        layout.bar = {0, 0, screenWidth, barHeight};
        layout.textLeft = layout.icon.right + textGap;
        layout.textRight = layout.minBtn.left - textGap;

        return layout;
    }

    bool IsInside(const RECT &rc, POINT pt)
    {
        return PtInRect(&rc, pt) != 0;
    }

    void DrawButton(Gdiplus::Graphics &g, const RECT &rc, TitleBar::Button btn, bool isHovered, bool isPressed)
    {
        Gdiplus::Color bg = Gdiplus::Color(0, 0, 0, 0);
        if (btn == TitleBar::Button::Close)
        {
            if (isPressed)
                bg = ToGdiColor(Theme::TitleBarClosePressed);
            else if (isHovered)
                bg = ToGdiColor(Theme::TitleBarCloseHover);
        }
        else
        {
            if (isPressed)
                bg = ToGdiColor(Theme::TitleBarBtnPressed);
            else if (isHovered)
                bg = ToGdiColor(Theme::TitleBarBtnHover);
        }

        if (bg.GetA() > 0)
        {
            Gdiplus::SolidBrush bgBrush(bg);
            Gdiplus::Rect fillRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
            g.FillRectangle(&bgBrush, fillRect);
        }

        int pad = UIScaler::S(TITLE_BAR_GLYPH_PADDING);
        int left = rc.left + pad;
        int right = rc.right - pad;
        int top = rc.top + pad;
        int bottom = rc.bottom - pad;
        int centerY = (top + bottom) / 2;
        float stroke = (float)(std::max)(1, UIScaler::S(TITLE_BAR_GLYPH_THICKNESS));

        Gdiplus::Pen pen(ToGdiColor(Theme::TitleBarText), stroke);
        pen.SetLineCap(Gdiplus::LineCapRound, Gdiplus::LineCapRound, Gdiplus::DashCapRound);

        if (btn == TitleBar::Button::Minimize)
        {
            g.DrawLine(&pen, (Gdiplus::REAL)left, (Gdiplus::REAL)centerY, (Gdiplus::REAL)right, (Gdiplus::REAL)centerY);
        }
        else if (btn == TitleBar::Button::Maximize)
        {
            Gdiplus::Rect rect(left, top, right - left, bottom - top);
            g.DrawRectangle(&pen, rect);
        }
        else if (btn == TitleBar::Button::Close)
        {
            g.DrawLine(&pen, (Gdiplus::REAL)left, (Gdiplus::REAL)top, (Gdiplus::REAL)right, (Gdiplus::REAL)bottom);
            g.DrawLine(&pen, (Gdiplus::REAL)left, (Gdiplus::REAL)bottom, (Gdiplus::REAL)right, (Gdiplus::REAL)top);
        }
    }
}

namespace TitleBar
{
    static std::wstring g_TitleText = APP_TITLE;
    static double g_LastFps = 0.0;
    static Gdiplus::Bitmap *g_IconBitmap = nullptr;
    static Button g_HoveredButton = Button::None;
    static Button g_PressedButton = Button::None;
    static bool g_TrackingMouse = false;

    void Initialize(const std::wstring &iconPath)
    {
        if (g_IconBitmap)
        {
            delete g_IconBitmap;
            g_IconBitmap = nullptr;
        }
        g_IconBitmap = Gdiplus::Bitmap::FromFile(iconPath.c_str());
        if (!g_IconBitmap || g_IconBitmap->GetLastStatus() != Gdiplus::Ok)
        {
            delete g_IconBitmap;
            g_IconBitmap = nullptr;
        }
    }

    void Shutdown()
    {
        if (g_IconBitmap)
        {
            delete g_IconBitmap;
            g_IconBitmap = nullptr;
        }
    }

    void SetTitleText(const std::wstring &text)
    {
        g_TitleText = text;
    }

    void UpdateFps(double fps)
    {
        g_LastFps = fps;
    }

    int GetHeightPx()
    {
        return UIScaler::SY(TITLE_BAR_BASE_HEIGHT);
    }

    RECT GetBarRect(int screenWidth)
    {
        RECT r = {0, 0, screenWidth, GetHeightPx()};
        return r;
    }

    void Render(HDC hdc, int screenWidth, int /*screenHeight*/)
    {
        TitleBarLayout layout = BuildLayout(screenWidth);

        Gdiplus::Graphics g(hdc);
        g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        Gdiplus::SolidBrush bgBrush(ToGdiColor(Theme::TitleBarBg));
        Gdiplus::Rect fillRect(layout.bar.left, layout.bar.top, layout.bar.right - layout.bar.left, layout.bar.bottom - layout.bar.top);
        g.FillRectangle(&bgBrush, fillRect);

        Gdiplus::Pen borderPen(ToGdiColor(Theme::TitleBarBorder), 1.0f);
        g.DrawLine(&borderPen, (Gdiplus::REAL)layout.bar.left, (Gdiplus::REAL)(layout.bar.bottom - 1),
                   (Gdiplus::REAL)layout.bar.right, (Gdiplus::REAL)(layout.bar.bottom - 1));

        if (g_IconBitmap)
        {
            int iconW = layout.icon.right - layout.icon.left;
            int iconH = layout.icon.bottom - layout.icon.top;
            g.DrawImage(g_IconBitmap, layout.icon.left, layout.icon.top, iconW, iconH);
        }

        SetBkMode(hdc, TRANSPARENT);

        int textRightLimit = layout.textRight;
        int fpsGap = UIScaler::SX(TITLE_BAR_FPS_GAP);
        std::wstring fpsText = L"FPS: " + std::to_wstring((int)std::round(g_LastFps));

        SIZE fpsSize = {};
        HFONT oldFont = (HFONT)SelectObject(hdc, GlobalFont::Note);
        GetTextExtentPoint32W(hdc, fpsText.c_str(), (int)fpsText.size(), &fpsSize);
        SelectObject(hdc, oldFont);

        int fpsRight = textRightLimit;
        int fpsX = fpsRight - fpsSize.cx;
        bool canDrawFps = fpsX > layout.textLeft + fpsGap;

        RECT titleRect = {layout.textLeft, layout.bar.top, textRightLimit, layout.bar.bottom};
        if (canDrawFps)
            titleRect.right = fpsX - fpsGap;

        SetTextColor(hdc, ToCOLORREF(Theme::TitleBarText));
        oldFont = (HFONT)SelectObject(hdc, GlobalFont::Bold);
        DrawTextW(hdc, g_TitleText.c_str(), -1, &titleRect, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(hdc, oldFont);

        if (canDrawFps)
        {
            RECT fpsRect = {fpsX, layout.bar.top, fpsRight, layout.bar.bottom};
            SetTextColor(hdc, ToCOLORREF(Theme::TitleBarFpsText));
            oldFont = (HFONT)SelectObject(hdc, GlobalFont::Note);
            DrawTextW(hdc, fpsText.c_str(), -1, &fpsRect, DT_VCENTER | DT_SINGLELINE | DT_RIGHT);
            SelectObject(hdc, oldFont);
        }

        DrawButton(g, layout.minBtn, Button::Minimize, g_HoveredButton == Button::Minimize, g_PressedButton == Button::Minimize);
        DrawButton(g, layout.maxBtn, Button::Maximize, g_HoveredButton == Button::Maximize, g_PressedButton == Button::Maximize);
        DrawButton(g, layout.closeBtn, Button::Close, g_HoveredButton == Button::Close, g_PressedButton == Button::Close);
    }

    Button HitTestButton(POINT pt, int screenWidth, int /*screenHeight*/)
    {
        TitleBarLayout layout = BuildLayout(screenWidth);
        if (IsInside(layout.closeBtn, pt))
            return Button::Close;
        if (IsInside(layout.maxBtn, pt))
            return Button::Maximize;
        if (IsInside(layout.minBtn, pt))
            return Button::Minimize;
        return Button::None;
    }

    bool IsDragRegion(POINT pt, int screenWidth, int screenHeight)
    {
        (void)screenHeight;
        TitleBarLayout layout = BuildLayout(screenWidth);
        if (!IsInside(layout.bar, pt))
            return false;
        return HitTestButton(pt, screenWidth, screenHeight) == Button::None;
    }

    bool OnMouseMove(HWND hWnd, POINT pt, int screenWidth, int screenHeight)
    {
        Button newHover = HitTestButton(pt, screenWidth, screenHeight);
        if (newHover != g_HoveredButton)
        {
            g_HoveredButton = newHover;
            RECT barRect = GetBarRect(screenWidth);
            InvalidateRect(hWnd, &barRect, FALSE);
        }

        if (!g_TrackingMouse)
        {
            TRACKMOUSEEVENT tme = {};
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = hWnd;
            TrackMouseEvent(&tme);
            g_TrackingMouse = true;
        }
        return newHover != Button::None || IsDragRegion(pt, screenWidth, screenHeight);
    }

    bool OnMouseDown(HWND hWnd, POINT pt, int screenWidth, int screenHeight)
    {
        Button hit = HitTestButton(pt, screenWidth, screenHeight);
        if (hit == Button::None)
            return false;

        g_PressedButton = hit;
        SetCapture(hWnd);
        RECT barRect = GetBarRect(screenWidth);
        InvalidateRect(hWnd, &barRect, FALSE);
        return true;
    }

    bool OnMouseUp(HWND hWnd, POINT pt, int screenWidth, int screenHeight)
    {
        if (g_PressedButton == Button::None)
            return false;

        Button releasedOn = HitTestButton(pt, screenWidth, screenHeight);
        Button pressed = g_PressedButton;
        g_PressedButton = Button::None;
        ReleaseCapture();

        if (releasedOn == pressed)
        {
            if (pressed == Button::Close)
                SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, 0);
            else if (pressed == Button::Minimize)
                SendMessage(hWnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            else if (pressed == Button::Maximize)
                SendMessage(hWnd, WM_SYSCOMMAND, IsZoomed(hWnd) ? SC_RESTORE : SC_MAXIMIZE, 0);
        }

        RECT barRect = GetBarRect(screenWidth);
        InvalidateRect(hWnd, &barRect, FALSE);
        return true;
    }

    void OnMouseLeave(HWND hWnd)
    {
        if (g_HoveredButton != Button::None)
        {
            g_HoveredButton = Button::None;
            RECT client = {};
            GetClientRect(hWnd, &client);
            RECT barRect = GetBarRect(client.right - client.left);
            InvalidateRect(hWnd, &barRect, FALSE);
        }
        g_TrackingMouse = false;
    }
}
