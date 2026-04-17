#include "UIScaler.h"

namespace UIScaler
{
    const float BASE_WIDTH = 1920.0f;
    const float BASE_HEIGHT = 1080.0f;

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleMin = 1.0f;

    void Update(int width, int height)
    {
        if (width <= 0)
        {
            width = 1;
        }
        if (height <= 0)
        {
            height = 1;
        }

        scaleX = width / BASE_WIDTH;
        scaleY = height / BASE_HEIGHT;
        scaleMin = (std::min)(scaleX, scaleY);
    }

    int S(int val)
    {
        return (int)(val * scaleMin);
    }

    int SX(int val)
    {
        return (int)(val * scaleX);
    }

    int SY(int val)
    {
        return (int)(val * scaleY);
    }
}
