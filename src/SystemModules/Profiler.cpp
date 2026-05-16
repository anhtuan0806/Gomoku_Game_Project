/** @file Profiler.cpp
 *  @brief Triển khai công cụ đo đạc hiệu năng.
 */
#include "Profiler.h"
#include <fstream>
#include <chrono>
#include <mutex>
#include <filesystem>

namespace Profiler
{
    static std::ofstream s_ofs;
    static std::mutex s_mtx;
    static bool s_enabled = false;
    static std::string s_path;
    static const double s_minDirtyArea = 64.0; // frames with dirty area below this are filtered unless there's a spike

    void InitIfRequested(const std::string &workspacePath)
    {
        try
        {
            std::filesystem::path flag = std::filesystem::path(workspacePath) / "profiling_on.txt";
            if (!std::filesystem::exists(flag))
                return;
            std::filesystem::path out = std::filesystem::path(workspacePath) / "profiling_blit.csv";
            s_ofs.open(out.string(), std::ios::out | std::ios::app);
            if (s_ofs.is_open())
            {
                s_enabled = true;
                s_path = out.string();
                // CSV columns: timestamp_ms,lastBlitMs,rectCount,dirtyArea,spike
                s_ofs << "timestamp_ms,lastBlitMs,rectCount,dirtyArea,spike\n";
                s_ofs.flush();
            }
        }
        catch (...) { s_enabled = false; }
    }

    void LogFrame(double lastBlitMs, int rectCount, double dirtyArea)
    {
        if (!s_enabled) return;
        // Filter: ignore empty frames or very small dirty area to reduce noise, but always log visible spikes
        if (rectCount == 0) return;
        if (rectCount > 0 && dirtyArea < s_minDirtyArea && lastBlitMs <= 16.0) return;

        std::lock_guard<std::mutex> lk(s_mtx);
        using namespace std::chrono;
        auto now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        int spike = (lastBlitMs > 16.0) ? 1 : 0; // >16ms is a visible frame drop
        // Use invariant locale formatting (replace comma decimal)
        s_ofs << now << "," << lastBlitMs << "," << rectCount << "," << dirtyArea << "," << spike << "\n";
        // flush occasionally
        static int cnt = 0; if ((++cnt & 0x3) == 0) s_ofs.flush();
    }

    void Shutdown()
    {
        if (!s_enabled) return;
        std::lock_guard<std::mutex> lk(s_mtx);
        s_ofs.flush();
        s_ofs.close();
        s_enabled = false;
    }
}
