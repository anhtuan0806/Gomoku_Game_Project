#ifndef PIXEL_LAYOUT_H
#define PIXEL_LAYOUT_H

#include <algorithm>
#include <cmath>
#include <windows.h>
#include <gdiplus.h>

/** @file PixelLayout.h
 *  @brief Helpers for crisp pixel-art blits and layout snapping on GDI+.
 */

namespace PixelLayout
{

/** Nearest-neighbor scaling, no edge anti-alias on bitmap blits (sprites, procedural pixel art). */
inline void ApplyPixelArtBlit(Gdiplus::Graphics &g)
{
    g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
}

/** Soft photos / thumbnails only — use sparingly so sprites stay sharp. */
inline void ApplySmoothPhotoBlit(Gdiplus::Graphics &g)
{
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
}

/** Round panel rects to whole pixels to avoid fuzzy rectangle edges after UIScaler math. */
inline void AlignRectToPixelGrid(int &x, int &y, int &w, int &h)
{
    x = static_cast<int>((double)x + 0.5);
    y = static_cast<int>((double)y + 0.5);
    w = (std::max)(1, static_cast<int>((double)w + 0.5));
    h = (std::max)(1, static_cast<int>((double)h + 0.5));
}

/** Pixel fonts (e.g. VT323): prefer even pixel heights after UIScaler::S. */
inline int QuantizedFontPixelHeight(int scaledPoints)
{
    int h = (std::max)(8, scaledPoints);
    if ((h & 1) != 0)
        ++h;
    return h;
}

/** smoothstep(0,1, 0.5+0.5*sin(t*w)) — smoother menu highlight than raw sin. */
inline float SinSmoothed01(float time, float w)
{
    float u = 0.5f + 0.5f * sinf(time * w);
    return u * u * (3.f - 2.f * u);
}

/** Same oscillation as sin(t*w) in [-1,1] but with smoothstep easing on the phase. */
inline float SinSmoothedSigned(float time, float w)
{
    return SinSmoothed01(time, w) * 2.f - 1.f;
}

} // namespace PixelLayout

#endif
