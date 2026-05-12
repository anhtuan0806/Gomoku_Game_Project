#include "Renderer.h"
#include "UIScaler.h"
#include "Colours.h"
#include "../ApplicationTypes/GameConstants.h"
#include <atomic>
#include <string>
#include <vector>
#include <fstream>

/** @file Renderer.cpp
 *  @brief Cài đặt cho quản lý font toàn cục và lifecycle của GDI+/double buffer.
 *
 *  Font VT323 được nạp vào bộ nhớ bằng AddFontMemResourceEx thay vì
 *  AddFontResourceExW để tránh lỗi path trên WSL filesystem
 *  (Win32 GDI font API không hỗ trợ UNC path \\wsl.localhost\...).
 */

namespace
{
    /** @brief Handle trả về từ AddFontMemResourceEx, dùng để gỡ font khi cleanup. */
    HANDLE g_FontMemHandle = nullptr;

    /**
     * @brief Đọc toàn bộ nội dung file nhị phân vào vector<BYTE>.
     * @param path Đường dẫn file (relative hoặc absolute).
     * @return Vector chứa dữ liệu file. Rỗng nếu đọc thất bại.
     */
    std::vector<BYTE> ReadFileToMemory(const std::wstring &path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
            return {};

        std::streamsize fileSize = file.tellg();
        if (fileSize <= 0)
            return {};

        file.seekg(0, std::ios::beg);
        std::vector<BYTE> buffer(static_cast<size_t>(fileSize));
        if (!file.read(reinterpret_cast<char *>(buffer.data()), fileSize))
            return {};

        return buffer;
    }

    /**
     * @brief Thử tìm file font VT323 từ nhiều đường dẫn khác nhau.
     *
     * Chiến lược (theo thứ tự ưu tiên):
     * 1. Relative từ CWD: "Asset/font/VT323/VT323-Regular.ttf"
     * 2. Từ thư mục executable đi ngược lên 0-4 cấp
     *
     * @return Vector chứa dữ liệu font. Rỗng nếu không tìm thấy.
     */
    std::vector<BYTE> LoadFontData()
    {
        // Danh sách đường dẫn ứng viên (relative từ CWD)
        const wchar_t *relativeCandidates[] = {
            L"Asset/font/VT323/VT323-Regular.ttf",
            L"Asset\\font\\VT323\\VT323-Regular.ttf",
            L"src/Asset/font/VT323/VT323-Regular.ttf",
            L"src\\Asset\\font\\VT323\\VT323-Regular.ttf",
        };

        for (const auto *candidate : relativeCandidates)
        {
            auto data = ReadFileToMemory(candidate);
            if (!data.empty())
                return data;
        }

        // Từ thư mục chứa executable đi ngược lên
        wchar_t modulePath[MAX_PATH] = {};
        GetModuleFileNameW(NULL, modulePath, MAX_PATH);

        wchar_t *lastSlash = wcsrchr(modulePath, L'\\');
        if (!lastSlash)
            lastSlash = wcsrchr(modulePath, L'/');
        if (lastSlash)
            *lastSlash = L'\0';

        std::wstring baseDir = modulePath;
        for (int up = 0; up <= 4; ++up)
        {
            std::wstring candidate = baseDir;
            for (int idx = 0; idx < up; ++idx)
                candidate += L"\\..";
            candidate += L"\\Asset\\font\\VT323\\VT323-Regular.ttf";

            auto data = ReadFileToMemory(candidate);
            if (!data.empty())
                return data;
        }

        return {};
    }
}

namespace GlobalFont
{
    HFONT Default = nullptr;
    HFONT Bold = nullptr;
    HFONT Title = nullptr;
    HFONT Note = nullptr;
}

// --- Globals ---
DoubleBuffer g_BackBuffer = {0};
ULONG_PTR g_GdiplusToken = 0;
std::atomic_bool g_NeedsRedraw = true;

namespace GlobalFont
{
    void Initialize()
    {
        // Đọc font file vào bộ nhớ để dùng AddFontMemResourceEx.
        std::vector<BYTE> fontData = LoadFontData();

        if (!fontData.empty())
        {
            DWORD numFontsInstalled = 0;
            g_FontMemHandle = AddFontMemResourceEx(
                fontData.data(),
                static_cast<DWORD>(fontData.size()),
                0,
                &numFontsInstalled);

#ifdef _DEBUG
            if (g_FontMemHandle == nullptr || numFontsInstalled == 0)
            {
                MessageBoxW(NULL,
                            L"AddFontMemResourceEx thất bại.\nFont VT323 sẽ không hiển thị đúng.",
                            L"Font Warning", MB_OK | MB_ICONWARNING);
            }
#endif
        }
        else
        {
#ifdef _DEBUG
            // Hiển thị CWD để debug
            wchar_t cwd[MAX_PATH] = {};
            GetCurrentDirectoryW(MAX_PATH, cwd);

            wchar_t exePath[MAX_PATH] = {};
            GetModuleFileNameW(NULL, exePath, MAX_PATH);

            std::wstring msg = L"Không tìm thấy file font VT323-Regular.ttf\n\n";
            msg += L"CWD: ";
            msg += cwd;
            msg += L"\nEXE: ";
            msg += exePath;
            MessageBoxW(NULL, msg.c_str(), L"Font Error", MB_OK | MB_ICONWARNING);
#endif
        }

        RebuildFonts();
    }

    void RebuildFonts()
    {
        // Xóa HFONT cũ (nếu có) trước khi tạo mới để tránh rò rỉ GDI.
        if (Default)
        {
            DeleteObject(Default);
            Default = nullptr;
        }
        if (Bold)
        {
            DeleteObject(Bold);
            Bold = nullptr;
        }
        if (Title)
        {
            DeleteObject(Title);
            Title = nullptr;
        }
        if (Note)
        {
            DeleteObject(Note);
            Note = nullptr;
        }

        // VT323 chỉ có 1 weight (Regular). Sử dụng FW_NORMAL cho tất cả
        // để Windows luôn ánh xạ đúng VT323 thay vì fallback sang font hệ thống.
        Default = CreateFontW(UIScaler::S(FONT_SIZE_DEFAULT), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                              DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_FAMILY_NAME);

        Bold = CreateFontW(UIScaler::S(FONT_SIZE_BOLD), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_FAMILY_NAME);

        Title = CreateFontW(UIScaler::S(FONT_SIZE_TITLE), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_FAMILY_NAME);

        Note = CreateFontW(UIScaler::S(FONT_SIZE_NOTE), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
                           CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, FONT_FAMILY_NAME);
    }

    void Cleanup()
    {
        if (Default)
        {
            DeleteObject(Default);
            Default = nullptr;
        }
        if (Bold)
        {
            DeleteObject(Bold);
            Bold = nullptr;
        }
        if (Title)
        {
            DeleteObject(Title);
            Title = nullptr;
        }
        if (Note)
        {
            DeleteObject(Note);
            Note = nullptr;
        }

        // Gỡ font đã thêm từ bộ nhớ
        if (g_FontMemHandle != nullptr)
        {
            RemoveFontMemResourceEx(g_FontMemHandle);
            g_FontMemHandle = nullptr;
        }
    }
}

bool InitGraphics(ULONG_PTR &gdiplusToken)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    return Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Ok;
}

void ShutdownGraphics(ULONG_PTR gdiplusToken)
{
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

void CreateBuffer(HWND hwnd, HDC windowHdc, DoubleBuffer &buffer)
{
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    buffer.width = max(1, clientRect.right - clientRect.left);
    buffer.height = max(1, clientRect.bottom - clientRect.top);

    buffer.hdcMemory = CreateCompatibleDC(windowHdc);
    buffer.hBitmap = CreateCompatibleBitmap(windowHdc, buffer.width, buffer.height);
    buffer.hOldBitmap = (HBITMAP)SelectObject(buffer.hdcMemory, buffer.hBitmap);

    // Khởi tạo nền trắng an toàn
    HBRUSH backgroundBrush = CreateSolidBrush(ToCOLORREF(Palette::White));
    RECT fillRect = {0, 0, buffer.width, buffer.height};
    FillRect(buffer.hdcMemory, &fillRect, backgroundBrush);
    DeleteObject(backgroundBrush);
}

void DeleteBuffer(DoubleBuffer &buffer)
{
    if (buffer.hdcMemory)
    {
        SelectObject(buffer.hdcMemory, buffer.hOldBitmap);
        DeleteDC(buffer.hdcMemory);
        buffer.hdcMemory = NULL;
    }

    if (buffer.hBitmap)
    {
        DeleteObject(buffer.hBitmap);
        buffer.hBitmap = NULL;
    }
}
