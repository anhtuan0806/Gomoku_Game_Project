#ifndef _UISCALER_H_
#define _UISCALER_H_
#include <windows.h>
#include <algorithm>

namespace UIScaler
{
    // 1920x1080 as the baseline for optimized UI calculations
    extern const float BASE_WIDTH;
    extern const float BASE_HEIGHT;

    extern float scaleX;
    extern float scaleY;
    extern float scaleMin;

    void Update(int width, int height);

    // Scaling helpers
    int S(int val);  // Scale uniform (min of X/Y scale to maintain aspect ratio)
    int SX(int val); // Scale X (Width)
    int SY(int val); // Scale Y (Height)
}

#endif // _UISCALER_H_
