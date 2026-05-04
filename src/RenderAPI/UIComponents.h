#ifndef _UI_COMPONENTS_H_
#define _UI_COMPONENTS_H_
#include "Renderer.h"
#include "../ApplicationTypes/PlayState.h"
#include <vector>
#include <string>
#include <map>

// Thời gian cho Animation toàn cục
extern float g_GlobalAnimTime;

// Struct quản lý trạng thái diễn hoạt
struct PlayerState
{
    int avatarType = 0;
    std::string currentAction = "idle";
    int currentFrame = 0;
    ULONGLONG lastFrameTime = 0;
    int animationSpeed = 100; // ms per frame
    bool flipH = false;
};

// Cấu trúc chứa dữ liệu Ma Trận Pixel
struct PixelModel
{
    int width = 0;
    int height = 0;
    std::vector<std::vector<int>> data;
    bool isLoaded = false;
};

// Hàm Load dữ liệu Pixel từ file txt bên ngoài (hỗ trợ đọc config)
PixelModel LoadPixelModel(const std::string &filePath);
// Vẽ mô hình Pixel với Palette động
void DrawPixelModel(Gdiplus::Graphics &g, const PixelModel &model, int centerX, int centerY, int totalSize, const std::map<int, Gdiplus::Color> &palette, size_t manualPaletteHash = 0);

// Vẽ văn bản căn giữa trong khoảng [leftX, rightX]
void DrawTextCentered(HDC hdc, const std::wstring &text, int posY, int rightX, COLORREF color, HFONT hFont = nullptr, int leftX = 0);

// Vẽ nền sân vận động bằng giải thuật
// showFlashes: chỉ bật true ở MenuScreen để hiệu ứng khán đài nhấp nháy
// animate: tắt để dùng nền tĩnh (tiết kiệm CPU)
void DrawProceduralStadium(Gdiplus::Graphics &g, int screenWidth, int screenHeight, bool shouldShowFlashes = false, bool shouldAnimate = true);

// Vẽ Avatar cầu thủ theo kiểu Pixel Art 8x8 từ ma trận số
void DrawPixelAvatar(Gdiplus::Graphics &g, int centerX, int centerY, int size, int avatarType);
Gdiplus::Color GetPaletteColor(int type, int code);

// Vẽ hoạt họa theo thư mục Hành động
void DrawPixelAction(Gdiplus::Graphics &g, int centerX, int centerY, int size, PlayerState &state);

// Vẽ hoạt họa Pixel chuyên dụng cho Khán Đài
void DrawPixelFootball(Gdiplus::Graphics &g, int centerX, int centerY, int size);
void DrawPixelTrophy(Gdiplus::Graphics &g, int centerX, int centerY, int size);
void DrawPixelClock(Gdiplus::Graphics &g, int centerX, int centerY, int size, Gdiplus::Color color);

// Vẽ Banner Tiêu đề Pixel Art cho màn hình Menu/Settings/Load
// text: chuỗi ký tự đơn giản (ASCII), centerX/centerY: tâm banner, accent: màu nổi bật
void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text, int centerX, int centerY, int panelWidth, COLORREF textColor, COLORREF glowColor);
void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text, int centerX, int centerY, int panelWidth, COLORREF textColor, COLORREF glowColor, const std::string &iconModelPath);

// Hàm này sẽ vẽ lưới, các quân cờ hiện có, và ô viền highlight đang chọn
void DrawGameBoard(Gdiplus::Graphics &g, HDC hdc, const PlayState *state, int cellSize, int offsetX, int offsetY);

// Cài đặt màu sắc cho văn bản và chế độ nền trong suốt
void SetTextColour(HDC hdc, COLORREF colour);

// Quản lý tài nguyên Brush tập trung để tối ưu hiệu suất
Gdiplus::SolidBrush *GetCachedBrush(const Gdiplus::Color &color);

// Giải phóng toàn bộ bộ nhớ đệm (Cache) để tránh rò rỉ RAM
void ClearUICaches();

#endif // _UI_COMPONENTS_H_