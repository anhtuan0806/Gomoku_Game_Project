#ifndef _RENDERER_H_
#define _RENDERER_H_
#include <windows.h>
#include <gdiplus.h>
#include <string>
#pragma comment(lib, "gdiplus.lib")

/** @file Renderer.h
 *  @brief Quản lý GDI+/font và double buffering cho renderer.
 */

/**
 * @brief Cấu trúc lưu trạng thái double-buffer dùng để vẽ không nhấp nháy.
 *
 * @note Trường `hBitmap` và `hdcMemory` được cấp phát trong `CreateBuffer` và
 *       phải được giải phóng bằng `DeleteBuffer` để tránh rò rỉ tài nguyên GDI.
 */
struct DoubleBuffer
{
    HDC hdcMemory;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    int width;
    int height;
};

/**
 * @brief Fonts toàn cục cho UI.
 *
 * @note Các `HFONT` được tạo trong `GlobalFont::RebuildFonts()` và phải được
 *       giải phóng trong `GlobalFont::Cleanup()`.
 */
namespace GlobalFont
{
    extern HFONT Default;
    extern HFONT Bold;
    extern HFONT Title;
    extern HFONT Note;

    /**
     * @brief Nạp font resource (từ thư mục Asset) và khởi tạo các HFONT mặc định.
     *
     * Gọi khi khởi tạo subsystem đồ họa/UI.
     */
    void Initialize();

    /**
     * @brief Tái tạo các `HFONT` (ví dụ khi scale/UI sizing thay đổi).
     */
    void RebuildFonts();

    /**
     * @brief Giải phóng các `HFONT` đã tạo và gỡ font resource khỏi hệ thống.
     */
    void Cleanup();
}

/**
 * @brief Khởi tạo GDI+.
 * @param token Tham chiếu để trả về token GDI+ (sử dụng trong `ShutdownGraphics`).
 * @return `true` nếu khởi tạo thành công.
 */
bool InitGraphics(ULONG_PTR &token);

/**
 * @brief Tắt GDI+ theo token trả về từ `InitGraphics`.
 * @param token Token nhận được từ `InitGraphics`.
 */
void ShutdownGraphics(ULONG_PTR token);

/**
 * @brief Tạo double buffer tương thích cho `hwnd`.
 *
 * @param hwnd Handle của cửa sổ.
 * @param windowHdc HDC tương thích để tạo bitmap.
 * @param buffer Tham chiếu tới `DoubleBuffer` để nhận kết quả.
 * @note Hàm đảm bảo kích thước tối thiểu 1x1 để tránh `CreateCompatibleBitmap` trả về NULL.
 *       Caller phải gọi `DeleteBuffer` để giải phóng tài nguyên do hàm này tạo.
 */
void CreateBuffer(HWND hwnd, HDC windowHdc, DoubleBuffer &buffer);

/**
 * @brief Giải phóng tài nguyên trong `DoubleBuffer` và reset các trường.
 */
void DeleteBuffer(DoubleBuffer &buffer);

// --- Globals (Owned by Renderer) ---
extern DoubleBuffer g_BackBuffer;
extern ULONG_PTR g_GdiplusToken;
extern bool g_NeedsRedraw;

#endif // _RENDERER_H_