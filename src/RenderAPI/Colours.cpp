#include "Colours.h"

COLORREF ToCOLORREF(const SmartColor &sc)
{
    return RGB(sc.r, sc.g, sc.b);
}

Gdiplus::Color ToGdiColor(const SmartColor &sc)
{
    return Gdiplus::Color(sc.a, sc.r, sc.g, sc.b);
}

SmartColor WithAlpha(const SmartColor &sc, BYTE newAlpha)
{
    return {newAlpha, sc.r, sc.g, sc.b};
}
