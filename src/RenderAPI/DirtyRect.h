#ifndef _DIRTY_RECT_H_
#define _DIRTY_RECT_H_
#include <windows.h>
#include <vector>
#include <utility>
#include <mutex>

/**
 * Simple Dirty Rect manager: supports adding pixel RECTs and logical board
 * cell coordinates (row,col). Thread-safe for producers (input/AI/timers)
 * and single-consumer (main/UI thread) resolution.
 */
namespace DirtyRect
{
    struct Manager
    {
        std::mutex mtx;
        std::vector<RECT> rects;                    // pixel rects
        std::vector<std::pair<int,int>> cells;      // logical board cells
    };

    // Global manager instance (module-owned)
    extern Manager g_mgr;

    // Add a raw pixel rect (thread-safe)
    void AddRect(const RECT &r);

    // Add a logical board cell to be resolved later (thread-safe)
    void AddCell(int row, int col);

    // Convert stored logical cells to pixel rects using cellSize/offset and clear cells list
    void ResolveBoardCells(int cellSize, int offsetX, int offsetY, int padding = 2);

    // Merge overlapping/adjacent rects and clip to client rect
    void MergeAndClip(std::vector<RECT> &inOut, const RECT &client, int margin = 2);

    // Steal current pixel rects (after resolution & merge) and clear manager
    std::vector<RECT> StealAndClear();

    // Clear everything
    void Clear();
}

#endif // _DIRTY_RECT_H_
