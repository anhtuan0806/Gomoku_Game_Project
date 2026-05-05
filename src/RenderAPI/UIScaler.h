#ifndef _UISCALER_H_
#define _UISCALER_H_
#include <windows.h>
#include <algorithm>

/** @file UIScaler.h
 *  @brief Hỗ trợ tính toán tỉ lệ giao diện (UI scaling) dựa trên kích thước màn hình.
 *
 *  Cung cấp các helper để nhân hệ số tỉ lệ `scaleX`, `scaleY` và `scaleMin`.
 */
namespace UIScaler
{
    /**
     * @brief Kích thước cơ sở (baseline) dùng để tính tỉ lệ UI.
     */
    extern const float BASE_WIDTH;
    extern const float BASE_HEIGHT;

    /**
     * @brief Tỉ lệ theo chiều ngang và chiều dọc hiện tại (so với baseline).
     */
    extern float scaleX;
    extern float scaleY;
    extern float scaleMin;

    /**
     * @brief Cập nhật các tỉ lệ dựa trên kích thước vùng vẽ `width` x `height`.
     * @param width Chiều rộng vùng vẽ (px).
     * @param height Chiều cao vùng vẽ (px).
     */
    void Update(int width, int height);

    // Scaling helpers
    /** @brief Scale đều theo `scaleMin` (dùng cho kích thước font/điểm ảnh chung). */
    int S(int val);
    /** @brief Scale theo trục X (chiều rộng). */
    int SX(int val);
    /** @brief Scale theo trục Y (chiều cao). */
    int SY(int val);
}

#endif // _UISCALER_H_
