#ifndef _TITLE_BAR_H_
#define _TITLE_BAR_H_

#include <windows.h>
#include <string>

/**
 * @file TitleBar.h
 * @brief Title bar tùy biến: vẽ UI và xử lý nút điều khiển cửa sổ.
 */
namespace TitleBar
{
    enum class Button
    {
        None,
        Minimize,
        Maximize,
        Close
    };

    void Initialize(const std::wstring &iconPath);
    void Shutdown();

    void SetTitleText(const std::wstring &text);
    void UpdateFps(double fps);

    int GetHeightPx();
    RECT GetBarRect(int screenWidth);

    void Render(HDC hdc, int screenWidth, int screenHeight);

    Button HitTestButton(POINT pt, int screenWidth, int screenHeight);
    bool IsDragRegion(POINT pt, int screenWidth, int screenHeight);

    bool OnMouseMove(HWND hWnd, POINT pt, int screenWidth, int screenHeight);
    bool OnMouseDown(HWND hWnd, POINT pt, int screenWidth, int screenHeight);
    bool OnMouseUp(HWND hWnd, POINT pt, int screenWidth, int screenHeight);
    void OnMouseLeave(HWND hWnd);
}

#endif // _TITLE_BAR_H_
