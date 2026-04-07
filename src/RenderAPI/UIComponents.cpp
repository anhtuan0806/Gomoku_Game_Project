#include "UIComponents.h"
#include "Colours.h"
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

float g_GlobalAnimTime = 0.0f;



// -------------------------------------------------------------
// HỆ THỐNG LOAD PIXEL MODEL TỪ FILE TXT
// -------------------------------------------------------------
PixelModel LoadPixelModel(const std::string& filePath) {
    PixelModel model;
    std::ifstream file(filePath);
    if (!file.is_open()) return model;

    std::string line;
    // Đọc kích thước W H ở dòng đầu
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> model.width >> model.height;
    }

    model.data.resize(model.height, std::vector<int>(model.width, 0));
    for (int r = 0; r < model.height; ++r) {
        if (!std::getline(file, line)) break;
        std::stringstream ss(line);
        for (int c = 0; c < model.width; ++c) {
            int val = 0;
            if (ss >> val) {
                model.data[r][c] = val;
            }
        }
    }
    model.isLoaded = true;
    return model;
}

void DrawPixelModel(Gdiplus::Graphics& g, const PixelModel& model, int cx, int cy, int pSize, const std::map<int, Gdiplus::Color>& palette) {
    if (!model.isLoaded) return;

    int totalW = model.width * pSize;
    int totalH = model.height * pSize;
    int startX = cx - totalW / 2;
    int startY = cy - totalH / 2;

    for (int r = 0; r < model.height; ++r) {
        for (int c = 0; c < model.width; ++c) {
            int val = model.data[r][c];
            if (val != 0) {
                auto it = palette.find(val);
                if (it != palette.end()) {
                    Gdiplus::SolidBrush b(it->second);
                    g.FillRectangle(&b, startX + c * pSize, startY + r * pSize, pSize, pSize);
                }
            }
        }
    }
}
// -------------------------------------------------------------

// --- DỮ LIỆU MA TRẬN AVATAR PIXEL ART 8x8 ---
const int AVATAR_SIZE = 8;

Gdiplus::Color GetPaletteColor(int type, int code) {
    if (code == 0) return GdipColour::_TRANSPARENT;
    if (code == 1) return GdipColour::AVA_OUTLINE;
    if (code == 5) return GdipColour::AVA_EYE;

    if (type == 0) { // Do Trang
        if (code == 2) return GdipColour::AVA_P1_SKIN;
        if (code == 3) return GdipColour::AVA_P1_SHIRT;
        if (code == 4) return GdipColour::AVA_P1_ACCENT;
    } else if (type == 1) { // Lam Vang
        if (code == 2) return GdipColour::AVA_P2_SKIN;
        if (code == 3) return GdipColour::AVA_P2_SHIRT;
        if (code == 4) return GdipColour::AVA_P2_ACCENT;
    } else if (type == 2) { // Bot Easy
        if (code == 2) return GdipColour::AVA_BOT_EASY_BODY;
        if (code == 3) return GdipColour::AVA_BOT_EASY_ACCENT;
        if (code == 4) return GdipColour::AVA_BOT_EASY_ACCENT;
    } else if (type == 3) { // Bot Medium
        if (code == 2) return GdipColour::AVA_BOT_MED_BODY;
        if (code == 3) return GdipColour::AVA_BOT_MED_DARK;
        if (code == 4) return GdipColour::AVA_BOT_MED_DARKEST;
    } else { // Bot Hard
        if (code == 2) return GdipColour::AVA_BOT_HARD_BODY;
        if (code == 3) return GdipColour::AVA_BOT_HARD_SHIRT;
        if (code == 4) return GdipColour::AVA_BOT_HARD_DARK;
    }
    return GdipColour::DEBUG_MAGENTA;
}

void DrawPixelAvatar(Gdiplus::Graphics& g, int x, int y, int size, int avatarType) {
    if (avatarType < 0 || avatarType > 4) avatarType = 0;
    
    // Nạp Data Mô hình Cầu thủ động (Load từ máy nếu chưa có)
    static PixelModel avatarModels[5];
    if (!avatarModels[avatarType].isLoaded) {
        std::string filename = "Asset/models/avatar_" + std::to_string(avatarType) + ".txt";
        avatarModels[avatarType] = LoadPixelModel(filename);
    }
    
    const PixelModel& model = avatarModels[avatarType];
    if (!model.isLoaded || model.width == 0) return;

    int pixelSize = size / model.width;
    int shadowOffset = pixelSize / 3;
    
    // Ve shadow truoc
    Gdiplus::SolidBrush shadowBrush(GdipColour::SHADOW_MED);
    for (int row = 0; row < model.height; row++) {
        for (int col = 0; col < model.width; col++) {
            int code = model.data[row][col];
            if (code != 0) {
                int px = x + col * pixelSize;
                int py = y + row * pixelSize;
                g.FillRectangle(&shadowBrush, px + shadowOffset, py + shadowOffset, pixelSize, pixelSize);
            }
        }
    }

    // Ve main colored pixels
    for (int row = 0; row < model.height; row++) {
        for (int col = 0; col < model.width; col++) {
            int code = model.data[row][col];
            if (code != 0) {
                Gdiplus::Color c = GetPaletteColor(avatarType, code);
                Gdiplus::SolidBrush brush(c);
                int px = x + col * pixelSize;
                int py = y + row * pixelSize;
                g.FillRectangle(&brush, px, py, pixelSize, pixelSize);

                // Vien tach nut Pixel cho chat Retro
                Gdiplus::Pen pixelPen(GdipColour::SHADOW_LIGHT, 1.0f);
                g.DrawRectangle(&pixelPen, px, py, pixelSize, pixelSize);
            }
        }
    }
}

void DrawPixelFootball(Gdiplus::Graphics& g, int cx, int cy, int size) {
    static PixelModel footModel;
    if (!footModel.isLoaded) {
        footModel = LoadPixelModel("Asset/models/football.txt");
    }
    if (!footModel.isLoaded || footModel.width == 0) return;

    int pSize = size / footModel.width;
    int startX = cx - (size / 2);
    int startY = cy - (size / 2);
    
    // Hiệu ứng tâng bóng (Pulse)
    float pulse = 1.0f + sin(g_GlobalAnimTime * 15.0f) * 0.05f;
    int pSizeMod = (int)(pSize * pulse);
    int offset = (size - footModel.width * pSizeMod) / 2;
    startX += offset;
    startY += offset;

    for (int r = 0; r < footModel.height; r++) {
        for (int c = 0; c < footModel.width; c++) {
            int val = footModel.data[r][c];
            if (val != 0) {
                Gdiplus::Color color = (val == 1) ? GdipColour::FOOTBALL_DARK : GdipColour::FOOTBALL_LIGHT;
                Gdiplus::SolidBrush b(color);
                g.FillRectangle(&b, startX + c * pSizeMod, startY + r * pSizeMod, pSizeMod, pSizeMod);
            }
        }
    }
}

void DrawPixelTrophy(Gdiplus::Graphics& g, int cx, int cy, int size) {
    static PixelModel trophyModel;
    if (!trophyModel.isLoaded) {
        trophyModel = LoadPixelModel("Asset/models/trophy.txt");
    }
    if (!trophyModel.isLoaded || trophyModel.width == 0) return;

    int pScale = size / trophyModel.width;
    int startX = cx - (size / 2);
    int startY = cy - (size / 2);

    for (int r = 0; r < trophyModel.height; r++) {
        for (int c = 0; c < trophyModel.width; c++) {
            int val = trophyModel.data[r][c];
            if (val != 0) {
                Gdiplus::Color color;
                if (val == 1) color = GdipColour::TROPHY_RIM;
                else if (val == 2) color = GdipColour::TROPHY_BODY;
                else color = GdipColour::TROPHY_SHINE;

                Gdiplus::SolidBrush b(color);
                g.FillRectangle(&b, startX + c * pScale, startY + r * pScale, pScale, pScale);

                Gdiplus::Pen pen(GdipColour::SHADOW_LIGHT, 1.0f);
                g.DrawRectangle(&pen, startX + c * pScale, startY + r * pScale, pScale, pScale);
            }
        }
    }
}

void DrawProceduralStadium(Gdiplus::Graphics& g, int screenWidth, int screenHeight) {
    // 1. Nền Cỏ sọc ngang
    int stripeHeight = 60;
    for (int y = 0; y < screenHeight; y += stripeHeight) {
        bool isDark = (y / stripeHeight) % 2 == 0;
        // Màu cỏ tối/sáng xen kẽ tạo chất bóng đá
        Gdiplus::SolidBrush stripeBrush(isDark ? GdipColour::PITCH_DARK : GdipColour::PITCH_LIGHT);
        g.FillRectangle(&stripeBrush, 0, y, screenWidth, stripeHeight);
    }

    // 1.5. Vẽ hệ thống Line Sân Bóng (Pitch Markings)
    Gdiplus::Pen pitchPen(GdipColour::PITCH_LINE, 6.0f);
    // Vạch chia giữa sân (Vertical Halfway Line)
    int midX = screenWidth / 2;
    int midY = screenHeight / 2;
    g.DrawLine(&pitchPen, midX, 0, midX, screenHeight);
    
    // Vòng tròn trung tâm (Center Circle)
    int circleRadius = screenHeight / 3;
    g.DrawEllipse(&pitchPen, midX - circleRadius, midY - circleRadius, circleRadius * 2, circleRadius * 2);

    // Chấm giao bóng (Center Dot)
    Gdiplus::SolidBrush dotBrush(GdipColour::PITCH_DOT);
    g.FillEllipse(&dotBrush, midX - 8, midY - 8, 16, 16);

    // Box Penanty (Viền Khu cấm địa 2 bên mép)
    int boxW = 150;
    int boxH = screenHeight / 2;
    g.DrawRectangle(&pitchPen, -5, midY - boxH / 2, boxW, boxH); // Mép trái
    g.DrawRectangle(&pitchPen, screenWidth - boxW + 5, midY - boxH / 2, boxW, boxH); // Mép phải

    // Bán nguyệt Penanty (Arcs)
    int arcRadius = 80;
    // Bán nguyệt trái
    g.DrawArc(&pitchPen, (int)(boxW - arcRadius/2), (int)(midY - arcRadius), (int)(arcRadius*2), (int)(arcRadius*2), -90.0f, 180.0f);
    // Bán nguyệt phải
    g.DrawArc(&pitchPen, (int)(screenWidth - boxW - arcRadius*1.5f), (int)(midY - arcRadius), (int)(arcRadius*2), (int)(arcRadius*2), 90.0f, 180.0f);

    // 2. Hiệu ứng Camera Flash trên khán đài (Stands) - Giảm hào quang để hết giống bong bóng
    int flashCount = 60;
    for (int i = 0; i < flashCount; i++) {
        int fx = (i * 918273) % screenWidth;
        int fy = (i * 374621) % (screenHeight / 3); 
        float phase = (i * 137) % 314 / 50.0f;
        
        float rawPulse = sin(g_GlobalAnimTime * 15.0f + phase);
        if (rawPulse > 0.85f) { 
            int alpha = (int)((rawPulse - 0.85f) * 6.66f * 255);
            alpha = max(0, min(255, alpha));
            
            // Xóa hào quang tròn, thay bằng tia chớp chữ thập (Cross/Star flare)
            Gdiplus::SolidBrush flashBrush(GdipColour::WithAlpha(GdipColour::CAMERA_FLASH, (BYTE)alpha));
            g.FillRectangle(&flashBrush, fx - 1, fy - 6, 2, 12); // Dọc
            g.FillRectangle(&flashBrush, fx - 6, fy - 1, 12, 2); // Ngang
            g.FillRectangle(&flashBrush, fx - 2, fy - 2, 4, 4);  // Tâm
        }
    }
}

void DrawSprite(Gdiplus::Graphics& g, const Sprite& sprite, int x, int y, int width, int height) {
    if (sprite.image) {
        g.DrawImage(sprite.image, (Gdiplus::REAL)x, (Gdiplus::REAL)y, (Gdiplus::REAL)width, (Gdiplus::REAL)height);
    }
}

void PreScaleSprite(const Sprite& orig, Sprite& scaled, int width, int height) {
    if (!orig.image || width <= 0 || height <= 0) return;
    if (scaled.image) {
        delete scaled.image;
        scaled.image = nullptr;
    }
    
    scaled.image = new Gdiplus::Bitmap(width, height, orig.image->GetPixelFormat());
    Gdiplus::Graphics g(scaled.image);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.DrawImage(orig.image, 0, 0, width, height);
}

void DrawTextCentered(HDC hdc, const std::wstring& text, int y, int screenWidth, COLORREF color, HFONT hFont) {
    HFONT fontToUse = (hFont != nullptr) ? hFont : GlobalFont::Default;
    HFONT hOldFont = (HFONT)SelectObject(hdc, fontToUse);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    RECT rect = { 0, y, screenWidth, y + 100 };
    DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

    SelectObject(hdc, hOldFont);
}

void DrawGameBoard(HDC hdc, const PlayState* state, int cellSize, int offsetX, int offsetY, const Sprite& spriteX, const Sprite& spriteO) {
    int size = state->boardSize;
    int boardLength = size * cellSize;

    // 1. Vẽ lưới (Grid) - Kẻ vạch vôi sân bóng màu Trắng
    HPEN hPen = CreatePen(PS_SOLID, 2, Colour::WHITE);
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    for (int i = 0; i <= size; ++i) {
        int currX = offsetX + i * cellSize;
        int currY = offsetY + i * cellSize;
        // Đường ngang
        MoveToEx(hdc, offsetX, currY, NULL);
        LineTo(hdc, offsetX + boardLength, currY);
        // Đường dọc
        MoveToEx(hdc, currX, offsetY, NULL);
        LineTo(hdc, currX, offsetY + boardLength);
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // 2. Khởi tạo đối tượng đồ họa 1 LẦN DUY NHẤT để vẽ tất cả quân cờ
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    // Tạo Font cho quân cờ động theo cellSize
    HFONT pieceFont = CreateFont(
        cellSize - 4, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, pieceFont);
    SetBkMode(hdc, TRANSPARENT);

    for (int r = 0; r < size; r++) {
        for (int c = 0; c < size; c++) {
            int drawX = offsetX + c * cellSize;
            int drawY = offsetY + r * cellSize;
            RECT cellRect = { drawX, drawY, drawX + cellSize, drawY + cellSize };

            // Highlight Last Move (Nháy chớp Alpha)
            if (r == state->lastMoveRow && c == state->lastMoveCol) {
                int alpha = (int)(150 + sin(g_GlobalAnimTime * 8.0f) * 100);
                Gdiplus::SolidBrush lastMoveBrush(GdipColour::WithAlpha(GdipColour::LAST_MOVE_HIGHLIGHT, (BYTE)max(0, min(255, alpha))));
                g.FillRectangle(&lastMoveBrush, drawX + 1, drawY + 1, cellSize - 1, cellSize - 1);
            }

            // Highlight Winning Line với hiệu ứng pulse + viền sao chép
            bool isWinCell = false;
            for (const auto& wCell : state->winningCells) {
                if (wCell.first == r && wCell.second == c) { isWinCell = true; break; }
            }
            if (isWinCell) {
                float wPulse = 0.5f + sin(g_GlobalAnimTime * 10.0f) * 0.5f;
                int wAlpha = (int)(100 + wPulse * 155);
                Gdiplus::SolidBrush winBrush(GdipColour::WithAlpha(GdipColour::WIN_CELL_FILL, (BYTE)wAlpha));
                g.FillRectangle(&winBrush, drawX + 1, drawY + 1, cellSize - 2, cellSize - 2);
                // Vien phat sang xanh la
                float pWidth = 1.5f + wPulse * 2.0f;
                Gdiplus::Pen winPen(GdipColour::WIN_CELL_BORDER, pWidth);
                g.DrawRectangle(&winPen, drawX + 2, drawY + 2, cellSize - 4, cellSize - 4);
            }

            if (state->board[r][c] == CELL_PLAYER1) {
                SetTextColor(hdc, Colour::ORANGE_NORMAL);
                DrawTextW(hdc, L"X", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            else if (state->board[r][c] == CELL_PLAYER2) {
                SetTextColor(hdc, Colour::CYAN_NORMAL);
                DrawTextW(hdc, L"O", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
    
    SelectObject(hdc, oldFont);
    DeleteObject(pieceFont);

    // 3. Vẽ Highlight Con trỏ (Cursor)
    if (state->status == MATCH_PLAYING) {
        RECT cursorRect = {
            offsetX + state->cursorCol * cellSize,
            offsetY + state->cursorRow * cellSize,
            offsetX + (state->cursorCol + 1) * cellSize,
            offsetY + (state->cursorRow + 1) * cellSize
        };

        HBRUSH highlightBrush = CreateSolidBrush(Colour::RED_NORMAL);
        // Tăng độ dày viền highlight lên 3px để dễ nhìn hơn
        HPEN highlightPen = CreatePen(PS_SOLID, 3, Colour::RED_NORMAL);
        HPEN oldHighlightPen = (HPEN)SelectObject(hdc, highlightPen);

        // Vẽ viền thay vì lấp đầy
        FrameRect(hdc, &cursorRect, highlightBrush);

        SelectObject(hdc, oldHighlightPen);
        DeleteObject(highlightPen);
        DeleteObject(highlightBrush);
    }
}

void SetTextColour(HDC hdc, COLORREF colour) {
    ::SetTextColor(hdc, colour); // Gọi hàm chuẩn của Windows GDI
    SetBkMode(hdc, TRANSPARENT); // Đảm bảo chữ không có nền màu bao quanh
}