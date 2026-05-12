#include "EngineStats.h"
#include "TimeSystem.h"
#include "../ApplicationTypes/GameConfig.h"

/** @file EngineStats.cpp
 *  @brief Triển khai đo lường hiệu năng: quản lý mục tiêu FPS, delta time và cập nhật tiêu đề cửa sổ.
 */

namespace EngineStats
{
    static double s_TargetFrameSeconds = 1.0 / 60.0;
    static auto s_LastFrameTime = std::chrono::high_resolution_clock::now();
    static std::chrono::time_point<std::chrono::high_resolution_clock> s_FrameStartTime;

    static double s_FPSTimer = 0.0;
    static int s_FPSFrames = 0;
    static double s_LastFPS = 0.0;
    static bool s_FpsDirty = false;

    /** @brief Khởi tạo module EngineStats với mục tiêu FPS.
     *  @param targetFPS FPS mục tiêu (ví dụ 60.0)
     */
    void Initialize(double targetFPS)
    {
        s_TargetFrameSeconds = 1.0 / targetFPS;
        s_LastFrameTime = std::chrono::high_resolution_clock::now();
    }

    /** @brief Thiết lập FPS mục tiêu tại runtime. */
    void SetTargetFPS(double targetFPS)
    {
        s_TargetFrameSeconds = 1.0 / targetFPS;
    }

    /** @brief Bắt đầu một khung hình, trả về delta time (giây) kể từ khung trước. */
    double BeginFrame()
    {
        // Ưu tiên sử dụng giới hạn FPS từ cấu hình của người dùng
        double target = g_Config.fpsLimit > 0 ? (double)g_Config.fpsLimit : 60.0;
        
        // Kiểm tra nguồn điện để tự động giảm FPS nếu đang dùng pin VÀ người dùng đang bật 60FPS
        // Nhưng nếu người dùng chủ động ép 60FPS, ta vẫn nên ưu tiên?
        // Theo yêu cầu, nếu người dùng bật 60fps trong setting thì phải được 60fps.
        // Do đó, ta bỏ hoàn toàn việc ép 30fps bằng pin ở đây, hoặc chỉ áp dụng nếu fpsLimit == 0 (auto).
        // Gomoku Game Config hiện tại có 30 hoặc 60.
        s_TargetFrameSeconds = 1.0 / target;

        s_FrameStartTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = s_FrameStartTime - s_LastFrameTime;
        s_LastFrameTime = s_FrameStartTime;

        double dt = elapsed.count();
        if (dt > 0.1)
            dt = 0.1; // Cap dt to prevent jumps
        return dt;
    }

    /** @brief Kết thúc khung hình và thực hiện sleep/throttle để đạt FPS mục tiêu. */
    void EndFrame()
    {
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> frameElapsed = frameEndTime - s_FrameStartTime;

        if (frameElapsed.count() < s_TargetFrameSeconds)
        {
            double sleepSeconds = s_TargetFrameSeconds - frameElapsed.count();
            auto sleepStart = std::chrono::high_resolution_clock::now();

            if (sleepSeconds > 0.0005 && g_FrameTimer)
            {
                LARGE_INTEGER dueTime;
                dueTime.QuadPart = -(LONGLONG)(sleepSeconds * 10000000.0);
                if (SetWaitableTimer(g_FrameTimer, &dueTime, 0, NULL, NULL, FALSE))
                    WaitForSingleObject(g_FrameTimer, INFINITE);
                else
                    Sleep((DWORD)(sleepSeconds * 1000.0));
            }
            else if (sleepSeconds > 0)
            {
                Sleep((DWORD)(sleepSeconds * 1000.0));
            }
            g_LastSleepMs = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - sleepStart).count();
        }
        else
        {
            g_LastSleepMs = 0.0;
        }
    }

    /** @brief Cập nhật FPS và đánh dấu dirty để title bar tự vẽ.
     *  @param hWnd Handle cửa sổ.
     *  @param dt Delta time của khung hiện tại.
     */
    void UpdateTitleStats(HWND hWnd, double dt)
    {
        (void)hWnd;
        s_FPSTimer += dt;
        s_FPSFrames++;

        if (s_FPSTimer >= 0.5)
        {
            s_LastFPS = s_FPSFrames / s_FPSTimer;
            s_FPSFrames = 0;
            s_FPSTimer = 0.0;
            s_FpsDirty = true;
        }
    }

    double GetLastFps()
    {
        return s_LastFPS;
    }

    bool ConsumeFpsDirty()
    {
        bool wasDirty = s_FpsDirty;
        s_FpsDirty = false;
        return wasDirty;
    }
}
