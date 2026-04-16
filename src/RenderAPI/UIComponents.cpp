#include "UIComponents.h"
#include "UIScaler.h"
#include "Colours.h"
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>

#include <unordered_map>
float g_GlobalAnimTime = 0.0f;



// -------------------------------------------------------------
// HỆ THỐNG LOAD PIXEL MODEL TỪ FILE TXT
// -------------------------------------------------------------
PixelModel LoadPixelModel(const std::string& filePath) {
    PixelModel model;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        return model;
    }

    std::string line;
    // Đọc kích thước W H ở dòng đầu
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> model.width >> model.height;
    }

    model.data.resize(model.height, std::vector<int>(model.width, 0));
    for (int r = 0; r < model.height; ++r) {
        if (!std::getline(file, line)) {
            break;
        }
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

void DrawPixelModel(Gdiplus::Graphics& g, const PixelModel& model, int cx, int cy, int totalSize, const std::map<int, Gdiplus::Color>& palette) {
    if (!model.isLoaded || model.width == 0 || model.height == 0) {
        return;
    }

    // Tự động tính toán kích thước mỗi điểm ảnh dựa trên diện tích mục tiêu
    int pSize = totalSize / max(model.width, model.height);
    if (pSize < 1) pSize = 1;

    // --- BITMAP CACHING (Zero Lag Rendering) ---
    static std::unordered_map<std::string, Gdiplus::Bitmap*> modelCache;
    
    // Tạo cache key duy nhất dựa trên model (địa chỉ), kích thước và bảng màu
    std::string key = std::to_string((uintptr_t)&model) + "_" + std::to_string(totalSize);
    for (auto const& [id, col] : palette) {
        key += "_" + std::to_string(col.GetValue());
    }

    if (modelCache.find(key) == modelCache.end()) {
        int totalW = model.width * pSize;
        int totalH = model.height * pSize;

        // Tạo bitmap đệm với khoảng trống nhỏ để tránh răng cưa viền
        Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(totalW + 2, totalH + 2, PixelFormat32bppARGB);
        Gdiplus::Graphics gBmp(bmp);
        gBmp.SetSmoothingMode(Gdiplus::SmoothingModeNone); // Giữ pixel sắc nét

        for (int r = 0; r < model.height; ++r) {
            for (int c = 0; c < model.width; ++c) {
                int val = model.data[r][c];
                if (val != 0) {
                    auto it = palette.find(val);
                    if (it != palette.end()) {
                        Gdiplus::SolidBrush b(it->second);
                        gBmp.FillRectangle(&b, c * pSize, r * pSize, pSize, pSize);
                        
                        // Vẽ lưới pixel mờ để giữ phong cách Retro
                        Gdiplus::Pen p(Gdiplus::Color(60, 0, 0, 0), 1.0f);
                        gBmp.DrawRectangle(&p, c * pSize, r * pSize, pSize, pSize);
                    }
                }
            }
        }
        modelCache[key] = bmp;
    }

    Gdiplus::Bitmap* cachedBmp = modelCache[key];
    if (cachedBmp) {
        // Vẽ bitmap đã cache ra tâm cx, cy
        g.DrawImage(cachedBmp, cx - (int)cachedBmp->GetWidth() / 2, cy - (int)cachedBmp->GetHeight() / 2);
    }
}
// -------------------------------------------------------------

// --- DỮ LIỆU MA TRẬN AVATAR PIXEL ART 8x8 ---
const int AVATAR_SIZE = 8;

// --- HỆ THỐNG MÀU SẮC NHÂN VẬT (UNIFIED PALETTE SYSTEM) ---
// Hàm nội bộ tra cứu màu sắc của từng cầu thủ/máy trả về SmartColor (Functional Style)
// --- HỆ THỐNG MÀU SẮC NHÂN VẬT (UNIFIED PALETTE SYSTEM) ---
// Hàm nội bộ tra cứu màu sắc của từng cầu thủ/máy trả về SmartColor (Functional Style)
static SmartColor LookupAvatarColor(int type, int code) {
    // 1. Các mã màu "Toàn cầu" thực sự (Không đổi theo nhân vật)
    if (code == 0) return Palette::Transparent;
    if (code == 1) return Theme::AvaOutline;
    if (code == 5) return Theme::AvaEye;
    if (code == 7) return Palette::White;
    if (code == 9) return Palette::WhiteSoft;

    // 2. Tra cứu theo từng Loại nhân vật (Type)
    switch (type) {
    case 1: // Messi (P_MES)
        switch (code) {
        case 2: return Theme::MES_SkinL;
        case 3: return Theme::MES_SkinM;
        case 4: return Theme::MES_SkinD;
        case 6: return Theme::MES_HairD; // Tóc tối nhất
        case 8: return Theme::MES_HairN; // Tóc trung bình
        case 10: return Palette::GrayNormal; // Bóng xám
        case 12: return Palette::RedDarkest; // Môi
        case 13: return Palette::YellowNormal; // Highlight tóc
        case 14: return Palette::GrayDark; // Bóng sâu
        case 15: return Theme::MES_ShirtL; // Jersey Blue
        case 16: return Theme::MES_ShirtD; // Jersey Blue Shadow
        case 17: return Theme::MES_Shorts; // Shorts
        }
        break;

    case 2: // Neymar (P_NEY)
        switch (code) {
        case 2: return Theme::NEY_SkinL;
        case 3: return Theme::NEY_SkinM;
        case 4: return Theme::NEY_SkinD;
        case 6: return Theme::NEY_HairD; // Tóc tối nhất
        case 8: return Theme::NEY_HairN; // Tóc trung bình
        case 10: return Palette::GrayNormal; // Bóng xám
        case 12: return Palette::RedDarkest; // Môi
        case 13: return Theme::NEY_Shirt;   // Jersey Yellow
        case 14: return Palette::GrayDark; // Bóng sâu
        case 15: return Theme::NEY_Shorts;  // Shorts Blue
        case 18: return Theme::NEY_ShirtAcc;// Green Accent (Number 10)
        }
        break;

    case 3: // Bot Easy
        if (code == 3) return Theme::BotEasy_Body;
        if (code == 4) return Theme::BotEasy_Accent;
        break;

    case 4: // Bot Medium
        if (code == 3) return Theme::BotMed_Body;
        if (code == 4) return Theme::BotMed_Dark;
        if (code == 12) return Theme::BotMed_Darkest;
        break;

    case 5: // Bot Hard
        if (code == 2) return Theme::BotHard_Body;
        if (code == 3) return Theme::BotHard_Shirt;
        if (code == 4) return Theme::BotHard_Dark;
        break;

    case 0: // Ronaldo (Mặc định)
    default:
        switch (code) {
        case 2: return Theme::CR7_SkinL;
        case 3: return Theme::CR7_SkinM;
        case 4: return Theme::CR7_SkinD;
        case 6: return Theme::CR7_HairD; // Tóc tối nhất
        case 8: return Theme::CR7_HairN; // Tóc trung bình
        case 9: return Palette::White;         // Số áo/Mắt (Trắng chuẩn)
        case 10: return Palette::GrayNormal; // Bóng xám
        case 12: return Palette::RedDarkest; // Môi
        case 13: return Theme::TitleFill;    // Áo vàng Al Nassr (Dùng TitleFill cho rực rỡ)
        case 14: return Palette::YellowDarkest; // Bóng áo 
        case 15: return Palette::BlueNormal;   // Quần/Vớ xanh
        }
        break;
    }

    // 3. Các mã màu fallback hoặc "Toàn cầu" cấp thấp
    if (code == 11) return Palette::BrownNormal; // Da hốc mắt chung

    // Mặc định trả về màu da Ronaldo nếu không khớp
    return Theme::CR7_SkinM; 
}

// Cầu nối công khai trả về kiểu GDI+ cho Renderer
Gdiplus::Color GetPaletteColor(int type, int code) {
    return ToGdiColor(LookupAvatarColor(type, code));
}


void DrawPixelAvatar(Gdiplus::Graphics& g, int x, int y, int size, int avatarType) {
    if (avatarType < 0 || avatarType > 5) {
        avatarType = 0;
    }

    static std::unordered_map<std::string, Gdiplus::Bitmap*> avatarCache;
    std::string cacheKey = std::to_string(avatarType) + "_" + std::to_string(size);

    if (avatarCache.find(cacheKey) == avatarCache.end()) {
        // Đảm bảo thư mục là Asset/models/avt_09/avt.txt
        std::string filename = "Asset/models/avt_0" + std::to_string(avatarType) + "/avt.txt";
        PixelModel model = LoadPixelModel(filename);

        // Fallback: Nếu không load được avatar yêu cầu, thử load avatar mặc định (type 0)
        if (!model.isLoaded && avatarType != 0) {
            filename = "Asset/models/avt_00/avt.txt";
            model = LoadPixelModel(filename);
        }

        if (!model.isLoaded || model.width == 0) {
            avatarCache[cacheKey] = nullptr;
        }
        else {
            int pixelSize = size / model.width;
            int shadowOffset = pixelSize / 3;
            int bw = size + shadowOffset + 4;
            int bh = size + shadowOffset + 4;

            Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(bw, bh, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);

            // Vẽ bóng (Shadow)
            Gdiplus::SolidBrush shadowBrush(ToGdiColor(Theme::ShadowMed));
            for (int r = 0; r < model.height; r++) {
                for (int c = 0; c < model.width; c++) {
                    if (model.data[r][c] != 0) {
                        gBmp.FillRectangle(&shadowBrush, c * pixelSize + shadowOffset, r * pixelSize + shadowOffset, pixelSize, pixelSize);
                    }
                }
            }

            // Vẽ nhân vật chính
            for (int r = 0; r < model.height; r++) {
                for (int c = 0; c < model.width; c++) {
                    int code = model.data[r][c];
                    if (code != 0) {
                        Gdiplus::Color color = GetPaletteColor(avatarType, code);
                        Gdiplus::SolidBrush brush(color);
                        gBmp.FillRectangle(&brush, c * pixelSize, r * pixelSize, pixelSize, pixelSize);

                        // 2. Tối ưu hóa: Chỉ vẽ viền lưới cho các nhân vật ít chi tiết (Type < 9)
                        if (avatarType < 9) {
                            Gdiplus::Pen pixelPen(ToGdiColor(Theme::ShadowLight), 1.0f);
                            gBmp.DrawRectangle(&pixelPen, c * pixelSize, r * pixelSize, pixelSize, pixelSize);
                        }
                    }
                }
            }
            avatarCache[cacheKey] = bmp;
        }
    }

    Gdiplus::Bitmap* cachedBmp = avatarCache[cacheKey];
    if (cachedBmp) {
        g.DrawImage(cachedBmp, (float)x, (float)y);
    }
}


void DrawPixelFootball(Gdiplus::Graphics& g, int cx, int cy, int size) {
    if (size <= 0) return;
    static std::unordered_map<int, Gdiplus::Bitmap*> footballCache;
    if (footballCache.find(size) == footballCache.end()) {
        PixelModel model = LoadPixelModel("Asset/models/bg/football.txt");
        if (!model.isLoaded || model.width == 0) { footballCache[size] = nullptr; }
        else {
            int pSize = size / model.width; if (pSize < 1) pSize = 1;
            Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            for (int r = 0; r < model.height; r++) {
                for (int c = 0; c < model.width; c++) {
                    int val = model.data[r][c]; if (val == 0) continue;
                    Gdiplus::Color color = (val == 1) ? ToGdiColor(Theme::FootballDark) : ToGdiColor(Theme::FootballLight);
                    Gdiplus::SolidBrush b(color);
                    gBmp.FillRectangle(&b, c * pSize, r * pSize, pSize, pSize);
                }
            }
            footballCache[size] = bmp;
        }
    }
    Gdiplus::Bitmap* cachedBmp = footballCache[size];
    if (cachedBmp) {
        float pulse = 1.0f + sin(g_GlobalAnimTime * 15.0f) * 0.05f;
        int ds = (int)(size * pulse);
        g.DrawImage(cachedBmp, cx - ds / 2, cy - ds / 2, ds, ds);
    }
}

void DrawPixelTrophy(Gdiplus::Graphics& g, int cx, int cy, int size) {
    if (size <= 0) return;
    static std::unordered_map<int, Gdiplus::Bitmap*> trophyCache;
    if (trophyCache.find(size) == trophyCache.end()) {
        PixelModel model = LoadPixelModel("Asset/models/bg/trophy.txt");
        if (!model.isLoaded || model.width == 0) { trophyCache[size] = nullptr; }
        else {
            int pScale = size / model.width; if (pScale < 1) pScale = 1;
            Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            for (int r = 0; r < model.height; r++) {
                for (int c = 0; c < model.width; c++) {
                    int val = model.data[r][c]; if (val == 0) continue;
                    Gdiplus::Color color = (val == 1) ? ToGdiColor(Theme::TrophyRim) : (val == 2 ? ToGdiColor(Theme::TrophyBody) : ToGdiColor(Theme::TrophyShine));
                    Gdiplus::SolidBrush b(color);
                    gBmp.FillRectangle(&b, c * pScale, r * pScale, pScale, pScale);
                    Gdiplus::Pen pen(ToGdiColor(Theme::ShadowLight), 1.0f);
                    gBmp.DrawRectangle(&pen, c * pScale, r * pScale, pScale, pScale);
                }
            }
            trophyCache[size] = bmp;
        }
    }
    Gdiplus::Bitmap* cachedBmp = trophyCache[size];
    if (cachedBmp) g.DrawImage(cachedBmp, cx - size / 2, cy - size / 2);
}

void DrawPixelClock(Gdiplus::Graphics& g, int cx, int cy, int size, Gdiplus::Color color) {
    if (size <= 0) return;
    static std::unordered_map<std::string, Gdiplus::Bitmap*> clockCache;
    std::string key = std::to_string(size) + "_" + std::to_string(color.GetValue());
    if (clockCache.find(key) == clockCache.end()) {
        PixelModel model = LoadPixelModel("Asset/models/bg/clock.txt");
        if (!model.isLoaded || model.width == 0) { clockCache[key] = nullptr; }
        else {
            int pSize = size / model.width; if (pSize < 1) pSize = 1;
            Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            Gdiplus::SolidBrush darkBrush(Gdiplus::Color(180, 20, 20, 30));
            Gdiplus::SolidBrush shineBrush(Gdiplus::Color(255, 255, 255, 255));
            Gdiplus::SolidBrush mainBrush(color);
            for (int r = 0; r < model.height; r++) {
                for (int c = 0; c < model.width; c++) {
                    int val = model.data[r][c]; if (val == 0) continue;
                    Gdiplus::SolidBrush* b = &mainBrush;
                    if (val == 1) b = &darkBrush; if (val == 3) b = &shineBrush;
                    gBmp.FillRectangle(b, c * pSize, r * pSize, pSize, pSize);
                    Gdiplus::Pen p(Gdiplus::Color(60, 0, 0, 0), 1.0f);
                    gBmp.DrawRectangle(&p, c * pSize, r * pSize, pSize, pSize);
                }
            }
            clockCache[key] = bmp;
        }
    }
    Gdiplus::Bitmap* cachedBmp = clockCache[key];
    if (cachedBmp) {
        float pulse = 0.8f + sin(g_GlobalAnimTime * 8.0f) * 0.2f;
        int ds = (int)(size * (0.95f + pulse * 0.05f));
        g.DrawImage(cachedBmp, cx - ds / 2, cy - ds / 2, ds, ds);
    }
}

// =============================================================
// DrawPixelBanner: Tiêu đề procedural thay chữ thuần
// Nền gradient tối + viền sáng pulse + 2 icon bóng 2 bên + chữ GDI
// =============================================================
void DrawPixelBanner(Gdiplus::Graphics& g, HDC hdc, const std::wstring& text,
    int cx, int cy, int panelW, COLORREF textColor, COLORREF glowColor)
{
    DrawPixelBanner(g, hdc, text, cx, cy, panelW, textColor, glowColor, "");
}

void DrawPixelBanner(Gdiplus::Graphics& g, HDC hdc, const std::wstring& text,
    int cx, int cy, int panelW, COLORREF textColor, COLORREF glowColor, const std::string& iconModelPath)
{
    int bannerW = panelW - UIScaler::SX(24);
    int bannerH = UIScaler::SY(50);
    int bannerX = cx - bannerW / 2;
    int bannerY = cy - bannerH / 2;

    // 1. Nền gradient tối
    Gdiplus::LinearGradientBrush gradBrush(
        Gdiplus::Point(bannerX, bannerY),
        Gdiplus::Point(bannerX + bannerW, bannerY),
        Gdiplus::Color(230, 10, 15, 25),
        Gdiplus::Color(230, 30, 60, 90));
    g.FillRectangle(&gradBrush, bannerX, bannerY, bannerW, bannerH);

    // 2. Đường viền sáng pulse
    float pulse = 0.6f + sin(g_GlobalAnimTime * 6.0f) * 0.4f;
    BYTE lineA = (BYTE)(180 + pulse * 75);
    BYTE gr = GetRValue(glowColor), gg = GetGValue(glowColor), gb = GetBValue(glowColor);
    Gdiplus::Pen topLine(Gdiplus::Color(lineA, gr, gg, gb), 2.5f);
    Gdiplus::Pen botLine(Gdiplus::Color((BYTE)(lineA * 0.6f), gr, gg, gb), 1.5f);
    g.DrawLine(&topLine, bannerX, bannerY, bannerX + bannerW, bannerY);
    g.DrawLine(&botLine, bannerX, bannerY + bannerH, bannerX + bannerW, bannerY + bannerH);

    Gdiplus::SolidBrush cornerBrush(Gdiplus::Color(80, gr, gg, gb));
    g.FillRectangle(&cornerBrush, bannerX, bannerY, 4, bannerH);
    g.FillRectangle(&cornerBrush, bannerX + bannerW - 4, bannerY, 4, bannerH);

    // 3. Icon trang trí (Football mặc định hoặc Tùy chỉnh)
    int iconY = cy;
    int iconSize = UIScaler::S(32);
    
    if (iconModelPath.empty()) {
        DrawPixelFootball(g, bannerX + 25, iconY, iconSize);
        DrawPixelFootball(g, bannerX + bannerW - 25, iconY, iconSize);
    }
    else {
        static std::map<std::string, PixelModel> bannerModelCache;
        if (bannerModelCache.find(iconModelPath) == bannerModelCache.end()) {
            bannerModelCache[iconModelPath] = LoadPixelModel(iconModelPath);
        }
        
        // Khởi tạo Palette động dựa trên màu nhấn của màn hình để vẽ icon chuẩn xác
        std::map<int, Gdiplus::Color> palette;
        palette[1] = Gdiplus::Color(200, 10, 20, 30);      // Viền tối mờ
        palette[2] = Gdiplus::Color(255, gr, gg, gb);      // Màu chủ đạo (Accent)
        palette[3] = Gdiplus::Color(255, 255, 255, 255);  // Màu trắng (Shine)
        palette[4] = Gdiplus::Color(160, gr, gg, gb);      // Màu phụ (Sub-accent)

        DrawPixelModel(g, bannerModelCache[iconModelPath], bannerX + 25, iconY, iconSize, palette);
        DrawPixelModel(g, bannerModelCache[iconModelPath], bannerX + bannerW - 25, iconY, iconSize, palette);
    }

    // 4. Chữ tiêu đề (Căn giữa và giới hạn vùng bao quanh)
    SetTextColor(hdc, textColor);
    HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Title);
    SetBkMode(hdc, TRANSPARENT);
    RECT r = { bannerX + 45, bannerY, bannerX + bannerW - 45, bannerY + bannerH };
    DrawTextW(hdc, text.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, oldF);
}

void DrawProceduralStadium(Gdiplus::Graphics& g, int screenWidth, int screenHeight, bool showFlashes) {
    // --- CACHE TẦNG TĨNH (Pitch & Lines) ---
    static Gdiplus::Bitmap* pitchCache = nullptr;
    static int cachedW = 0, cachedH = 0;

    if (!pitchCache || cachedW != screenWidth || cachedH != screenHeight) {
        if (pitchCache) delete pitchCache;
        pitchCache = new Gdiplus::Bitmap(screenWidth, screenHeight, PixelFormat32bppARGB);
        Gdiplus::Graphics gP(pitchCache);
        gP.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        // 1. Nền Cỏ sọc ngang
        int stripeHeight = UIScaler::SY(60);
        for (int y = 0; y < screenHeight; y += stripeHeight) {
            bool isDark = (y / stripeHeight) % 2 == 0;
            Gdiplus::SolidBrush stripeBrush(isDark ? ToGdiColor(Theme::PitchDark) : ToGdiColor(Theme::PitchLight));
            gP.FillRectangle(&stripeBrush, 0, y, screenWidth, stripeHeight);
        }

        // 1.5. Hệ thống Line Sân Bóng (Pitch Markings)
        Gdiplus::Pen pitchPen(ToGdiColor(Theme::PitchLine), (Gdiplus::REAL)UIScaler::S(6));
        int midX = screenWidth / 2;
        int midY = screenHeight / 2;
        gP.DrawLine(&pitchPen, midX, 0, midX, screenHeight);

        int circleRadius = screenHeight / 3;
        gP.DrawEllipse(&pitchPen, midX - circleRadius, midY - circleRadius, circleRadius * 2, circleRadius * 2);

        Gdiplus::SolidBrush dotBrush(ToGdiColor(Theme::PitchDot));
        int dotR = UIScaler::S(8);
        gP.FillEllipse(&dotBrush, midX - dotR, midY - dotR, dotR * 2, dotR * 2);

        int boxW = UIScaler::SX(150);
        int boxH = screenHeight / 2;
        gP.DrawRectangle(&pitchPen, -5, midY - boxH / 2, boxW, boxH);
        gP.DrawRectangle(&pitchPen, screenWidth - boxW + 5, midY - boxH / 2, boxW, boxH);

        int arcRadius = UIScaler::S(80);
        gP.DrawArc(&pitchPen, (int)(boxW - arcRadius / 2), (int)(midY - arcRadius), (int)(arcRadius * 2), (int)(arcRadius * 2), -90.0f, 180.0f);
        gP.DrawArc(&pitchPen, (int)(screenWidth - boxW - arcRadius * 1.5f), (int)(midY - arcRadius), (int)(arcRadius * 2), (int)(arcRadius * 2), 90.0f, 180.0f);

        cachedW = screenWidth;
        cachedH = screenHeight;
    }

    // Vẽ nền tĩnh từ Cache
    g.DrawImage(pitchCache, 0, 0);

    // 2. Hiệu ứng Camera Flash (CHỈ vẽ ở MenuScreen khi showFlashes = true)
    if (showFlashes) {
        int flashCount = 40; // Giảm số lượng flash để tăng performance mà vẫn đẹp
        for (int i = 0; i < flashCount; i++) {
            int fx = (i * 918273) % screenWidth;
            int fy = (i * 374621) % (screenHeight / 3);
            float phase = (i * 137) % 314 / 50.0f;

            float rawPulse = sin(g_GlobalAnimTime * 15.0f + phase);
            if (rawPulse > 0.88f) {
                int alpha = (int)((rawPulse - 0.88f) * 8.3f * 255);
                alpha = max(0, min(255, alpha));

                Gdiplus::SolidBrush flashBrush(Gdiplus::Color((BYTE)alpha, 255, 255, 255));
                g.FillRectangle(&flashBrush, fx - 1, fy - 6, 2, 12);
                g.FillRectangle(&flashBrush, fx - 6, fy - 1, 12, 2);
                g.FillRectangle(&flashBrush, fx - 2, fy - 2, 4, 4);
            }
        }
    }

    // 3. Rạch gió (Wind Streaks) bay ngang sân cỏ (Tối ưu hóa Brush)
    {
        const int WIND_COUNT = 8; // Giảm số lượng rạch gió
        static const struct WindLine {
            float yFrac, speed; int len, alpha;
        } WIND_LINES[] = {
            { 0.18f, 55.0f, 160, 45 }, { 0.31f, 40.0f, 220, 50 },
            { 0.44f, 60.0f, 140, 40 }, { 0.60f, 50.0f, 180, 45 },
            { 0.72f, 45.0f, 200, 50 }, { 0.85f, 62.0f, 150, 42 },
            { 0.24f, 80.0f, 90, 30 }, { 0.52f, 75.0f, 110, 35 }
        };

        for (int i = 0; i < WIND_COUNT; i++) {
            const WindLine& wl = WIND_LINES[i];
            int wy = (int)(wl.yFrac * screenHeight);
            int wx = (int)fmod(wl.speed * g_GlobalAnimTime + i * (screenWidth / (float)WIND_COUNT), (float)(screenWidth + wl.len)) - wl.len;

            Gdiplus::SolidBrush windBrush(Gdiplus::Color((BYTE)wl.alpha, 255, 255, 255));
            g.FillRectangle(&windBrush, wx, wy, wl.len, UIScaler::SY(2));
        }
    }

    // 4. Đám mây trôi ngang
    {
        static PixelModel cloudModel;
        if (!cloudModel.isLoaded) cloudModel = LoadPixelModel("Asset/models/bg/cloud.txt");
        if (cloudModel.isLoaded) {
            static std::map<int, Gdiplus::Color> cloudPalette = {{1, Gdiplus::Color(170, 245, 248, 255)}};
            const float clouds[][3] = {{22.0f, 0.04f, 0.18f}, {14.0f, 0.10f, 0.12f}, {30.0f, 0.02f, 0.10f}};
            for (int i = 0; i < 3; i++) {
                int cSize = (int)(screenWidth * clouds[i][2]);
                int cx = (int)fmod(clouds[i][0] * g_GlobalAnimTime + i * (screenWidth / 3.0f), (float)(screenWidth + UIScaler::SX(200)));
                int cy = (int)(clouds[i][1] * screenHeight) + (int)(sin(g_GlobalAnimTime * 0.8f + i) * UIScaler::SY(4));
                DrawPixelModel(g, cloudModel, cx, cy, cSize, cloudPalette);
            }
        }
    }

    // 5. Bóng bay (Balloons)
    {
        static PixelModel balloonModel;
        if (!balloonModel.isLoaded) balloonModel = LoadPixelModel("Asset/models/bg/balloon.txt");
        if (balloonModel.isLoaded) {
            static const struct BalloonDef { float s, x; Gdiplus::Color c, sh; } bs[] = {
                { 28.0f, 0.10f, Gdiplus::Color(210, 230, 50,  50),  Gdiplus::Color(255, 255, 180, 180) },
                { 20.0f, 0.35f, Gdiplus::Color(210, 50,  120, 220), Gdiplus::Color(255, 160, 200, 255) },
                { 35.0f, 0.60f, Gdiplus::Color(210, 50,  200, 80),  Gdiplus::Color(255, 160, 255, 180) },
                { 24.0f, 0.82f, Gdiplus::Color(210, 220, 160, 30),  Gdiplus::Color(255, 255, 230, 140) }
            };
            for (int i = 0; i < 4; i++) {
                int bx = (int)(bs[i].x * screenWidth) + (int)(sin(g_GlobalAnimTime * 1.2f + i * 1.1f) * UIScaler::SX(18));
                int by = screenHeight - (int)fmod(bs[i].s * g_GlobalAnimTime + i * (screenHeight / 4.0f), (float)(screenHeight + UIScaler::SY(120)));
                
                std::map<int, Gdiplus::Color> bPalette = {{1, Gdiplus::Color(200,30,30,30)}, {2, bs[i].c}, {3, bs[i].sh}};
                DrawPixelModel(g, balloonModel, bx, by, UIScaler::S(48), bPalette);
            }
        }
    }
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

void DrawGameBoard(HDC hdc, const PlayState* state, int cellSize, int offsetX, int offsetY) {
    int size = state->boardSize;
    int boardLength = size * cellSize;

    // 1. Vẽ lưới (Grid) - Kẻ vạch vôi sân bóng màu Trắng
    HPEN hPen = CreatePen(PS_SOLID, max(1, UIScaler::S(2)), ToCOLORREF(Palette::White));
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
                Gdiplus::SolidBrush lastMoveBrush(ToGdiColor(WithAlpha(Theme::LastMoveHighlight, (BYTE)max(0, min(255, alpha)))));
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
                Gdiplus::SolidBrush winBrush(ToGdiColor(WithAlpha(Theme::WinCellFill, (BYTE)wAlpha)));
                g.FillRectangle(&winBrush, drawX + 1, drawY + 1, cellSize - 2, cellSize - 2);
                // Vien phat sang xanh la
                float pWidth = 1.5f + wPulse * 2.0f;
                Gdiplus::Pen winPen(ToGdiColor(Theme::WinCellBorder), pWidth);
                g.DrawRectangle(&winPen, drawX + 2, drawY + 2, cellSize - 4, cellSize - 4);
            }

            if (state->board[r][c] == CELL_PLAYER1) {
                SetTextColor(hdc, ToCOLORREF(Palette::OrangeNormal));
                DrawTextW(hdc, L"X", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            else if (state->board[r][c] == CELL_PLAYER2) {
                SetTextColor(hdc, ToCOLORREF(Palette::CyanNormal));
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

        HBRUSH highlightBrush = CreateSolidBrush(ToCOLORREF(Palette::RedNormal));
        // Tăng độ dày viền highlight lên 3px để dễ nhìn hơn
        HPEN highlightPen = CreatePen(PS_SOLID, max(1, UIScaler::S(3)), ToCOLORREF(Palette::RedNormal));
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


void DrawPixelAction(Gdiplus::Graphics& g, int cx, int cy, int size, PlayerState& state) {
    if (size <= 0) return;

    // 1. Cập nhật Frame dựa trên thời gian (Delta Time)
    DWORD now = GetTickCount();
    if (now - state.lastFrameTime > (DWORD)state.animationSpeed) {
        state.currentFrame++;
        state.lastFrameTime = now;
    }

    // 2. Tạo Cache Key (Bao gồm Type, Action, Frame và Flip)
    std::string cacheKey = "v5_" + std::to_string(state.avatarType) + "_" + 
                           state.currentAction + "_" + 
                           std::to_string(state.currentFrame) + "_" + 
                           (state.flipH ? "f" : "n") + "_" +
                           std::to_string(size);

    static std::unordered_map<std::string, Gdiplus::Bitmap*> actionCache;
    
    if (actionCache.find(cacheKey) == actionCache.end()) {
        std::string path = "Asset/models/avt_0" + std::to_string(state.avatarType) + 
                           "/" + state.currentAction + "/f_" + std::to_string(state.currentFrame) + ".txt";
        
        PixelModel model = LoadPixelModel(path);
        
        // Nếu không load được frame này (hết hành động), reset về frame 0
        if (!model.isLoaded && state.currentFrame > 0) {
            state.currentFrame = 0;
            path = "Asset/models/avt_0" + std::to_string(state.avatarType) + 
                   "/" + state.currentAction + "/f_0.txt";
            model = LoadPixelModel(path);
            
            // Cập nhật lại cacheKey cho f_0
            cacheKey = "v5_" + std::to_string(state.avatarType) + "_" + 
                       state.currentAction + "_0_" + 
                       (state.flipH ? "f" : "n") + "_" +
                       std::to_string(size);
            
            if (actionCache.find(cacheKey) != actionCache.end() && actionCache[cacheKey] != nullptr) {
                g.DrawImage(actionCache[cacheKey], cx - (int)actionCache[cacheKey]->GetWidth() / 2, cy - (int)actionCache[cacheKey]->GetHeight() / 2);
                return;
            }
        }

        if (!model.isLoaded || model.width == 0) {
            // Thử fallback sang avt_00 nếu load thất bại
            if (state.avatarType != 0) {
                std::string fallbackPath = "Asset/models/avt_00/" + state.currentAction + "/f_" + std::to_string(state.currentFrame) + ".txt";
                model = LoadPixelModel(fallbackPath);
            }
            
            if (!model.isLoaded) {
                actionCache[cacheKey] = nullptr;
            }
        }
        
        // Nếu load thành công (hoặc qua fallback)
        if (model.isLoaded && model.width > 0) {
            int pSize = size / model.width;
            if (pSize < 1) pSize = 1;
            int bw = model.width * pSize + 4;
            int bh = model.height * pSize + 4;

            Gdiplus::Bitmap* bmp = new Gdiplus::Bitmap(bw, bh, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            gBmp.SetSmoothingMode(Gdiplus::SmoothingModeNone);
            
            // Vẽ bóng
            Gdiplus::SolidBrush shadowBrush(ToGdiColor(Theme::ShadowMed));
            for (int r = 0; r < model.height; r++) {
                for (int c = 0; c < model.width; c++) {
                    if (model.data[r][c] == 0) continue;
                    int dc = state.flipH ? (model.width - 1 - c) : c;
                    gBmp.FillRectangle(&shadowBrush, dc * pSize + 2, r * pSize + 2, pSize, pSize);
                }
            }
            
            // Vẽ thân
            for (int r = 0; r < model.height; r++) {
                for (int c = 0; c < model.width; c++) {
                    int val = model.data[r][c];
                    if (val == 0) continue;
                    Gdiplus::Color color = (val == 7) ? ToGdiColor(Theme::AnimBoot) : (val == 6 ? ToGdiColor(Theme::AnimBall) : GetPaletteColor(state.avatarType, val));
                    Gdiplus::SolidBrush b(color);
                    int dc = state.flipH ? (model.width - 1 - c) : c;
                    gBmp.FillRectangle(&b, dc * pSize, r * pSize, pSize, pSize);
                    
                    Gdiplus::Pen pen(ToGdiColor(Theme::ShadowLight), 1.0f);
                    gBmp.DrawRectangle(&pen, dc * pSize, r * pSize, pSize, pSize);
                }
            }
            actionCache[cacheKey] = bmp;
        }
    }

    Gdiplus::Bitmap* cachedBmp = actionCache[cacheKey];
    if (cachedBmp) {
        g.DrawImage(cachedBmp, cx - (int)cachedBmp->GetWidth() / 2, cy - (int)cachedBmp->GetHeight() / 2);
    }
}