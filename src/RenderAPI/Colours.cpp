#include "Colours.h"

COLORREF ToCOLORREF(const SmartColor &color)
{
    return RGB(color.r, color.g, color.b);
}

Gdiplus::Color ToGdiColor(const SmartColor &color)
{
    return Gdiplus::Color(color.a, color.r, color.g, color.b);
}

SmartColor WithAlpha(const SmartColor &color, BYTE newAlpha)
{
    return {newAlpha, color.r, color.g, color.b};
}
