#include "Colours.h"

/** @file Colours.cpp
 *  @brief Cài đặt hàm tiện ích chuyển đổi giữa `SmartColor`, `COLORREF` và `Gdiplus::Color`.
 */

/**
 * @brief Chuyển `SmartColor` sang `COLORREF` (BGR cho GDI).
 */
COLORREF ToCOLORREF(const SmartColor &color)
{
    return RGB(color.r, color.g, color.b);
}

/**
 * @brief Chuyển `SmartColor` sang `Gdiplus::Color` (bao gồm alpha).
 */
Gdiplus::Color ToGdiColor(const SmartColor &color)
{
    return Gdiplus::Color(color.a, color.r, color.g, color.b);
}

/**
 * @brief Tạo `SmartColor` mới với kênh alpha được gán giá trị `newAlpha`.
 */
SmartColor WithAlpha(const SmartColor &color, BYTE newAlpha)
{
    return {newAlpha, color.r, color.g, color.b};
}
