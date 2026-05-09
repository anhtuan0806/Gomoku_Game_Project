#include "DirtyRect.h"
#include <algorithm>

namespace DirtyRect
{
    Manager g_mgr;

    static inline RECT NormalizeRect(const RECT &r)
    {
        RECT out = r;
        if (out.left > out.right) std::swap(out.left, out.right);
        if (out.top > out.bottom) std::swap(out.top, out.bottom);
        return out;
    }

    void AddRect(const RECT &r)
    {
        RECT n = NormalizeRect(r);
        if (n.right <= n.left || n.bottom <= n.top) return;
        std::lock_guard<std::mutex> lk(g_mgr.mtx);
        g_mgr.rects.push_back(n);
    }

    void AddCell(int row, int col)
    {
        std::lock_guard<std::mutex> lk(g_mgr.mtx);
        g_mgr.cells.emplace_back(row, col);
    }

    void ResolveBoardCells(int cellSize, int offsetX, int offsetY, int padding)
    {
        std::lock_guard<std::mutex> lk(g_mgr.mtx);
        for (auto &p : g_mgr.cells)
        {
            int row = p.first, col = p.second;
            RECT r;
            r.left = offsetX + col * cellSize - padding;
            r.top = offsetY + row * cellSize - padding;
            r.right = r.left + cellSize + padding * 2;
            r.bottom = r.top + cellSize + padding * 2;
            g_mgr.rects.push_back(NormalizeRect(r));
        }
        g_mgr.cells.clear();
    }

    // Simple rectangle union/merge based on bounding box expansion
    static bool TryMergeRects(RECT &a, const RECT &b, int margin)
    {
        // First check simple expansion overlap (margin)
        RECT expanded = {a.left - margin, a.top - margin, a.right + margin, a.bottom + margin};
        if ( !(b.right < expanded.left || b.left > expanded.right || b.bottom < expanded.top || b.top > expanded.bottom) )
        {
            a.left = (a.left < b.left) ? a.left : b.left;
            a.top = (a.top < b.top) ? a.top : b.top;
            a.right = (a.right > b.right) ? a.right : b.right;
            a.bottom = (a.bottom > b.bottom) ? a.bottom : b.bottom;
            return true;
        }

        // If not overlapping by margin, consider union-area heuristic: if the bounding union's area
        // is not much larger than the sum of areas, merging reduces overhead.
        int aW = max(0, a.right - a.left);
        int aH = max(0, a.bottom - a.top);
        int bW = max(0, b.right - b.left);
        int bH = max(0, b.bottom - b.top);
        int areaA = aW * aH;
        int areaB = bW * bH;
        RECT uni;
        uni.left = min(a.left, b.left);
        uni.top = min(a.top, b.top);
        uni.right = max(a.right, b.right);
        uni.bottom = max(a.bottom, b.bottom);
        int uW = max(0, uni.right - uni.left);
        int uH = max(0, uni.bottom - uni.top);
        int unionArea = uW * uH;

        // Allow merge if unionArea is <= 1.25 * (areaA + areaB) or if distance between centers < 8 px
        double areaThresh = 1.25;
        int ax = a.left + aW/2; int ay = a.top + aH/2;
        int bx = b.left + bW/2; int by = b.top + bH/2;
        int dx = ax - bx; int dy = ay - by;
        int centerDistSq = dx*dx + dy*dy;

        if (unionArea <= (int)(areaThresh * (areaA + areaB)) || centerDistSq <= (8*8))
        {
            a = uni;
            return true;
        }

        return false;
    }

    void MergeAndClip(std::vector<RECT> &inOut, const RECT &client, int margin)
    {
        if (inOut.empty()) return;
        // Clip first
        std::vector<RECT> clipped;
        for (auto &r : inOut)
        {
            RECT out;
            if (IntersectRect(&out, &r, &client))
                clipped.push_back(out);
        }
        if (clipped.empty()) { inOut.clear(); return; }

        // Sort by left then top
        std::sort(clipped.begin(), clipped.end(), [](const RECT &a, const RECT &b){ if (a.left!=b.left) return a.left<b.left; return a.top<b.top; });

        // Merge greedily
        std::vector<RECT> merged;
        merged.push_back(clipped[0]);
        for (size_t i=1;i<clipped.size();++i)
        {
            if (!TryMergeRects(merged.back(), clipped[i], margin))
                merged.push_back(clipped[i]);
        }
        inOut.swap(merged);
    }

    std::vector<RECT> StealAndClear()
    {
        std::lock_guard<std::mutex> lk(g_mgr.mtx);
        std::vector<RECT> out = std::move(g_mgr.rects);
        g_mgr.rects.clear();
        g_mgr.cells.clear();
        return out;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lk(g_mgr.mtx);
        g_mgr.rects.clear();
        g_mgr.cells.clear();
    }
}
