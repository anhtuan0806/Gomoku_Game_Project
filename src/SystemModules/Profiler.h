/** @file Profiler.h
 *  @brief Giao diện công cụ đo đạc hiệu năng.
 */
#ifndef _PROFILER_H_
#define _PROFILER_H_
#include <string>

namespace Profiler
{
    // Initialize profiler if profiling flag file exists. Safe to call multiple times.
    void InitIfRequested(const std::string &workspacePath);

    // Log a single frame metric (lastBlitMs) plus dirty-rect stats
    void LogFrame(double lastBlitMs, int rectCount = 0, double dirtyArea = 0.0);

    // Shutdown and flush
    void Shutdown();
}

#endif // _PROFILER_H_
