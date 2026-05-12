#ifndef _ENGINE_STATS_H_
#define _ENGINE_STATS_H_

#include <windows.h>
#include <chrono>

/** @file EngineStats.h
 *  @brief Quản lý thời gian khung hình, FPS và hiệu năng.
 */

namespace EngineStats
{
    /** @brief Khởi tạo các thông số engine. */
    void Initialize(double targetFPS);

    /** @brief Thay đổi FPS mục tiêu tại runtime (gọi khi người dùng đổi cài đặt). */
    void SetTargetFPS(double targetFPS);

    /** @brief Bắt đầu một khung hình mới, trả về Delta Time (giây). */
    double BeginFrame();

    /** @brief Kết thúc khung hình, thực hiện throttling để duy trì FPS mục tiêu. */
    void EndFrame();

    /** @brief Cập nhật FPS và cờ dirty (phục vụ title bar tuỳ biến).
     *  @param hWnd Handle của cửa sổ.
     *  @param dt Delta time của khung hình hiện tại.
     */
    void UpdateTitleStats(HWND hWnd, double dt);

    /** @brief Lấy FPS gần nhất đã được cập nhật. */
    double GetLastFps();

    /** @brief Trả về true nếu FPS vừa được cập nhật, đồng thời reset cờ dirty. */
    bool ConsumeFpsDirty();
}

#endif // _ENGINE_STATS_H_
