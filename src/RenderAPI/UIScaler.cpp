#include "UIScaler.h"

/** @file UIScaler.cpp
 *  @brief Cài đặt helper tỉ lệ giao diện.
 */

namespace UIScaler
{
    const float BASE_WIDTH = 1920.0f;
    const float BASE_HEIGHT = 1080.0f;

    float scaleX = 1.0f;
    float scaleY = 1.0f;
    float scaleMin = 1.0f;

    /**
     * @brief Cập nhật `scaleX`, `scaleY` và `scaleMin` theo kích thước mới.
     * @note Bảo vệ chống chia cho 0 bằng cách ép `width`/`height` >= 1.
     */
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

    /** @brief Scale theo `scaleMin` (giữ tỉ lệ đồng nhất giữa X và Y). */
    int S(int val)
    {
        return (int)(val * scaleMin);
    }

    /** @brief Scale theo trục X (chiều rộng). */
    int SX(int val)
    {
        return (int)(val * scaleX);
    }

    /** @brief Scale theo trục Y (chiều cao). */
    int SY(int val)
    {
        return (int)(val * scaleY);
    }
}
