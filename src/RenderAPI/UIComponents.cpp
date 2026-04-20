#include "UIComponents.h"
#include "UIScaler.h"
#include "Colours.h"
#include <map>
#include <cmath>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <functional>

float g_GlobalAnimTime = 0.0f;

static std::unordered_map<int, Gdiplus::Bitmap *> g_FootballCache;
static std::unordered_map<int, Gdiplus::Bitmap *> g_TrophyCache;
static std::unordered_map<size_t, Gdiplus::Bitmap *> g_ClockCache;
static std::unordered_map<size_t, Gdiplus::Bitmap *> g_ActionCache;
static std::unordered_map<size_t, Gdiplus::Bitmap *> g_AvatarCache;
static std::unordered_map<size_t, Gdiplus::Bitmap *> g_ModelCache;
static std::unordered_map<std::string, PixelModel> g_RawModelCache;
static std::unordered_map<ULONG, Gdiplus::SolidBrush *> g_BrushCache;

// Helper: Kết hợp Hash để tạo Key nhanh
template <class T>
inline void hash_combine(size_t &seed, const T &v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

Gdiplus::SolidBrush *GetCachedBrush(const Gdiplus::Color &color)
{
    ULONG key = color.GetValue();
    auto it = g_BrushCache.find(key);
    if (it != g_BrushCache.end())
    {
        return it->second;
    }
    Gdiplus::SolidBrush *brush = new Gdiplus::SolidBrush(color);
    g_BrushCache[key] = brush;
    return brush;
}

// -------------------------------------------------------------
// HỆ THỐNG LOAD PIXEL MODEL TỪ FILE TXT
// -------------------------------------------------------------
PixelModel LoadPixelModel(const std::string &filePath)
{
    // Kiểm tra cache trước khi đọc file từ đĩa
    auto it = g_RawModelCache.find(filePath);
    if (it != g_RawModelCache.end())
        return it->second;

    PixelModel model;
    std::ifstream file(filePath);
    if (!file.is_open())
    {
        return model;
    }

    std::string line;
    if (std::getline(file, line))
    {
        std::stringstream ss(line);
        ss >> model.width >> model.height;
    }

    model.data.resize(model.height, std::vector<int>(model.width, 0));
    for (int r = 0; r < model.height; ++r)
    {
        if (!std::getline(file, line))
            break;
        std::stringstream ss(line);
        for (int c = 0; c < model.width; ++c)
        {
            int val = 0;
            if (ss >> val)
                model.data[r][c] = val;
        }
    }
    model.isLoaded = true;
    g_RawModelCache[filePath] = model; // Lưu vào cache
    return model;
}

void DrawPixelModel(Gdiplus::Graphics &g, const PixelModel &model, int cx, int cy, int totalSize, const std::map<int, Gdiplus::Color> &palette, size_t manualPaletteHash)
{
    if (!model.isLoaded || model.width == 0 || model.height == 0 || totalSize <= 0)
    {
        return;
    }

    // Tự động tính toán kích thước mỗi điểm ảnh dựa trên diện tích mục tiêu
    int pSize = totalSize / max(model.width, model.height);
    if (pSize < 1)
        pSize = 1;

    // --- Hashing Key Optimization (No string concatenation in hot loop) ---
    size_t key = (size_t)&model;
    hash_combine(key, totalSize);

    if (manualPaletteHash != 0)
    {
        hash_combine(key, manualPaletteHash);
    }
    else
    {
        for (auto const &[id, col] : palette)
        {
            hash_combine(key, id);
            hash_combine(key, col.GetValue());
        }
    }

    if (g_ModelCache.find(key) == g_ModelCache.end())
    {
        int totalW = model.width * pSize;
        int totalH = model.height * pSize;

        // Tạo bitmap đệm với khoảng trống nhỏ để tránh răng cưa viền
        Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(totalW + 2, totalH + 2, PixelFormat32bppARGB);
        Gdiplus::Graphics gBmp(bmp);
        gBmp.SetSmoothingMode(Gdiplus::SmoothingModeNone); // Giữ pixel sắc nét

        Gdiplus::Pen gridPen(Gdiplus::Color(60, 0, 0, 0), 1.0f);
        for (int r = 0; r < model.height; ++r)
        {
            for (int c = 0; c < model.width; ++c)
            {
                int val = model.data[r][c];
                if (val != 0)
                {
                    auto it = palette.find(val);
                    if (it != palette.end())
                    {
                        Gdiplus::SolidBrush *b = GetCachedBrush(it->second);
                        gBmp.FillRectangle(b, c * pSize, r * pSize, pSize, pSize);

                        // Vẽ lưới pixel mờ để giữ phong cách Retro
                        gBmp.DrawRectangle(&gridPen, c * pSize, r * pSize, pSize, pSize);
                    }
                }
            }
        }
        g_ModelCache[key] = bmp;
    }

    Gdiplus::Bitmap *cachedBmp = g_ModelCache[key];
    if (cachedBmp)
    {
        // --- High Performance Rendering Settings ---
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

        // Vẽ bitmap đã cache ra tâm cx, cy
        g.DrawImage(cachedBmp, cx - (int)cachedBmp->GetWidth() / 2, cy - (int)cachedBmp->GetHeight() / 2);
    }
}
// -------------------------------------------------------------

// --- DỮ LIỆU MA TRẬN AVATAR PIXEL ART 8x8 ---
const int AVATAR_SIZE = 8;

static SmartColor LookupAvatarColor(int type, int code)
{
    // 1. Các mã màu "Toàn cầu" thực sự (Không đổi theo nhân vật)
    if (code == 0)
    {
        return Palette::Transparent;
    }
    if (code == 1)
    {
        return Theme::AvaOutline;
    }
    if (code == 5)
    {
        return Theme::AvaEye;
    }
    if (code == 7)
    {
        return Palette::White;
    }
    if (code == 9)
    {
        return Palette::WhiteSoft;
    }

    // 2. Tra cứu theo từng Loại nhân vật (Type)
    switch (type)
    {
    case 1: // Messi (P_MES)
        switch (code)
        {
        case 2:
            return Theme::MES_SkinL;
        case 3:
            return Theme::MES_SkinM;
        case 4:
            return Theme::MES_SkinD;
        case 6:
            return Theme::MES_HairD; // Tóc tối nhất
        case 8:
            return Theme::MES_HairN; // Tóc trung bình
        case 10:
            return Palette::GrayNormal; // Bóng xám
        case 12:
            return Palette::RedDarkest; // Môi
        case 13:
            return Palette::YellowNormal; // Highlight tóc
        case 14:
            return Palette::GrayDark; // Bóng sâu
        case 15:
            return Theme::MES_ShirtL; // Jersey Blue
        case 16:
            return Theme::MES_ShirtD; // Jersey Blue Shadow
        case 17:
            return Theme::MES_Shorts; // Shorts
        }
        break;

    case 2: // Neymar (P_NEY)
        switch (code)
        {
        case 2:
            return Theme::NEY_SkinL;
        case 3:
            return Theme::NEY_SkinM;
        case 4:
            return Theme::NEY_SkinD;
        case 6:
            return Theme::NEY_HairD; // Tóc tối nhất
        case 8:
            return Theme::NEY_HairN; // Tóc trung bình
        case 10:
            return Palette::GrayNormal; // Bóng xám
        case 12:
            return Palette::RedDarkest; // Môi
        case 13:
            return Theme::NEY_Shirt; // Jersey Yellow
        case 14:
            return Palette::GrayDark; // Bóng sâu
        case 15:
            return Theme::NEY_Shorts; // Shorts Blue
        case 18:
            return Theme::NEY_ShirtAcc; // Green Accent (Number 10)
        }
        break;

    case 0: // Ronaldo (Mặc định)
    default:
        switch (code)
        {
        case 2:
            return Theme::CR7_SkinL;
        case 3:
            return Theme::CR7_SkinM;
        case 4:
            return Theme::CR7_SkinD;
        case 6:
            return Theme::CR7_HairD; // Tóc tối nhất
        case 8:
            return Theme::CR7_HairN; // Tóc trung bình
        case 9:
            return Palette::White; // Số áo/Mắt (Trắng chuẩn)
        case 10:
            return Palette::GrayNormal; // Bóng xám
        case 12:
            return Palette::RedDarkest; // Môi
        case 13:
            return Theme::TitleFill; // Áo vàng Al Nassr (Dùng TitleFill cho rực rỡ)
        case 14:
            return Palette::YellowDarkest; // Bóng áo
        case 15:
            return Palette::BlueNormal; // Quần/Vớ xanh
        }
        break;
    }

    // 3. Các mã màu fallback hoặc "Toàn cầu" cấp thấp
    if (code == 11)
    {
        return Palette::BrownNormal; // Da hốc mắt chung
    }

    // Mặc định trả về màu da Ronaldo nếu không khớp
    return Theme::CR7_SkinM;
}

// Cầu nối công khai trả về kiểu GDI+ cho Renderer
Gdiplus::Color GetPaletteColor(int type, int code)
{
    return ToGdiColor(LookupAvatarColor(type, code));
}

void DrawPixelAvatar(Gdiplus::Graphics &g, int x, int y, int size, int avatarType)
{
    if (avatarType < 0 || avatarType > 2)
    {
        avatarType = 0;
    }

    size_t cacheKey = avatarType;
    hash_combine(cacheKey, size);

    if (g_AvatarCache.find(cacheKey) == g_AvatarCache.end())
    {
        // Đảm bảo thư mục là Asset/models/avt_09/avt.txt
        std::string filename = "Asset/models/avt_0" + std::to_string(avatarType) + "/avt.txt";
        PixelModel model = LoadPixelModel(filename);

        // Fallback: Nếu không load được avatar yêu cầu, load avatar mặc định (type 0)
        if (!model.isLoaded && avatarType != 0)
        {
            filename = "Asset/models/avt_00/avt.txt";
            model = LoadPixelModel(filename);
        }

        if (!model.isLoaded || model.width == 0)
        {
            g_AvatarCache[cacheKey] = nullptr;
        }
        else
        {
            int pixelSize = size / model.width;
            int shadowOffset = pixelSize / 3;
            int bw = size + shadowOffset + 4;
            int bh = size + shadowOffset + 4;

            Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(bw, bh, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);

            // Vẽ bóng (Shadow)
            Gdiplus::SolidBrush *shadowBrush = GetCachedBrush(ToGdiColor(Theme::ShadowMed));
            for (int r = 0; r < model.height; r++)
            {
                for (int c = 0; c < model.width; c++)
                {
                    if (model.data[r][c] != 0)
                    {
                        gBmp.FillRectangle(shadowBrush, c * pixelSize + shadowOffset, r * pixelSize + shadowOffset, pixelSize, pixelSize);
                    }
                }
            }

            // Vẽ nhân vật chính
            Gdiplus::Pen pixelPen(ToGdiColor(Theme::ShadowLight), 1.0f);
            for (int r = 0; r < model.height; r++)
            {
                for (int c = 0; c < model.width; c++)
                {
                    int code = model.data[r][c];
                    if (code != 0)
                    {
                        Gdiplus::Color color = GetPaletteColor(avatarType, code);
                        Gdiplus::SolidBrush *brush = GetCachedBrush(color);
                        gBmp.FillRectangle(brush, c * pixelSize, r * pixelSize, pixelSize, pixelSize);

                        // 2. Chỉ vẽ viền lưới cho các nhân vật ít chi tiết
                        if (avatarType < 6)
                        {
                            gBmp.DrawRectangle(&pixelPen, c * pixelSize, r * pixelSize, pixelSize, pixelSize);
                        }
                    }
                }
            }
            g_AvatarCache[cacheKey] = bmp;
        }
    }

    Gdiplus::Bitmap *cachedBmp = g_AvatarCache[cacheKey];
    if (cachedBmp)
    {
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBmp, (float)x, (float)y);
    }
}

void DrawPixelFootball(Gdiplus::Graphics &g, int cx, int cy, int size)
{
    if (size <= 0)
    {
        return;
    }
    if (g_FootballCache.find(size) == g_FootballCache.end())
    {
        PixelModel model = LoadPixelModel("Asset/models/bg/football.txt");
        if (!model.isLoaded || model.width == 0)
        {
            g_FootballCache[size] = nullptr;
        }
        else
        {
            int pSize = size / model.width;
            if (pSize < 1)
                pSize = 1;
            Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            for (int r = 0; r < model.height; r++)
            {
                for (int c = 0; c < model.width; c++)
                {
                    int val = model.data[r][c];
                    if (val == 0)
                        continue;
                    Gdiplus::Color color = (val == 1) ? ToGdiColor(Theme::FootballDark) : ToGdiColor(Theme::FootballLight);
                    Gdiplus::SolidBrush b(color);
                    gBmp.FillRectangle(&b, c * pSize, r * pSize, pSize, pSize);
                }
            }
            g_FootballCache[size] = bmp;
        }
    }
    Gdiplus::Bitmap *cachedBmp = g_FootballCache[size];
    if (cachedBmp)
    {
        float pulse = 1.0f + sin(g_GlobalAnimTime * 15.0f) * 0.05f;
        int ds = (int)(size * pulse);
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBmp, cx - ds / 2, cy - ds / 2, ds, ds);
    }
}

void DrawPixelTrophy(Gdiplus::Graphics &g, int cx, int cy, int size)
{
    if (size <= 0)
    {
        return;
    }
    if (g_TrophyCache.find(size) == g_TrophyCache.end())
    {
        PixelModel model = LoadPixelModel("Asset/models/bg/trophy.txt");
        if (!model.isLoaded || model.width == 0)
        {
            g_TrophyCache[size] = nullptr;
        }
        else
        {
            int pScale = size / model.width;
            if (pScale < 1)
                pScale = 1;
            Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            for (int r = 0; r < model.height; r++)
            {
                for (int c = 0; c < model.width; c++)
                {
                    int val = model.data[r][c];
                    if (val == 0)
                        continue;
                    Gdiplus::Color color = (val == 1) ? ToGdiColor(Theme::TrophyRim) : (val == 2 ? ToGdiColor(Theme::TrophyBody) : ToGdiColor(Theme::TrophyShine));
                    Gdiplus::SolidBrush b(color);
                    gBmp.FillRectangle(&b, c * pScale, r * pScale, pScale, pScale);
                    Gdiplus::Pen pen(ToGdiColor(Theme::ShadowLight), 1.0f);
                    gBmp.DrawRectangle(&pen, c * pScale, r * pScale, pScale, pScale);
                }
            }
            g_TrophyCache[size] = bmp;
        }
    }
    Gdiplus::Bitmap *cachedBmp = g_TrophyCache[size];
    if (cachedBmp)
    {
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBmp, cx - size / 2, cy - size / 2, size, size);
    }
}

void DrawPixelClock(Gdiplus::Graphics &g, int cx, int cy, int size, Gdiplus::Color color)
{
    if (size <= 0)
    {
        return;
    }
    size_t key = size;
    hash_combine(key, color.GetValue());

    if (g_ClockCache.find(key) == g_ClockCache.end())
    {
        PixelModel model = LoadPixelModel("Asset/models/bg/clock.txt");
        if (!model.isLoaded || model.width == 0)
        {
            g_ClockCache[key] = nullptr;
        }
        else
        {
            int pSize = size / model.width;
            if (pSize < 1)
                pSize = 1;
            Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            Gdiplus::SolidBrush darkBrush(Gdiplus::Color(180, 20, 20, 30));
            Gdiplus::SolidBrush shineBrush(Gdiplus::Color(255, 255, 255, 255));
            Gdiplus::SolidBrush mainBrush(color);
            for (int r = 0; r < model.height; r++)
            {
                for (int c = 0; c < model.width; c++)
                {
                    int val = model.data[r][c];
                    if (val == 0)
                        continue;
                    Gdiplus::SolidBrush *b = &mainBrush;
                    if (val == 1)
                        b = &darkBrush;
                    if (val == 3)
                        b = &shineBrush;
                    gBmp.FillRectangle(b, c * pSize, r * pSize, pSize, pSize);
                    Gdiplus::Pen p(Gdiplus::Color(60, 0, 0, 0), 1.0f);
                    gBmp.DrawRectangle(&p, c * pSize, r * pSize, pSize, pSize);
                }
            }
            g_ClockCache[key] = bmp;
        }
    }
    Gdiplus::Bitmap *cachedBmp = g_ClockCache[key];
    if (cachedBmp)
    {
        float pulse = 0.8f + sin(g_GlobalAnimTime * 8.0f) * 0.2f;
        int ds = (int)(size * (0.95f + pulse * 0.05f));
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBmp, cx - ds / 2, cy - ds / 2, ds, ds);
    }
}

// =============================================================
// DrawPixelBanner: Tiêu đề procedural thay chữ thuần
// Nền gradient tối + viền sáng pulse + 2 icon bóng 2 bên + chữ GDI
// =============================================================
void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text,
                     int cx, int cy, int panelW, COLORREF textColor, COLORREF glowColor)
{
    DrawPixelBanner(g, hdc, text, cx, cy, panelW, textColor, glowColor, "");
}

void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text,
                     int cx, int cy, int panelW, COLORREF textColor, COLORREF glowColor, const std::string &iconModelPath)
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

    if (iconModelPath.empty())
    {
        DrawPixelFootball(g, bannerX + 25, iconY, iconSize);
        DrawPixelFootball(g, bannerX + bannerW - 25, iconY, iconSize);
    }
    else
    {
        // Sử dụng cache raw model đã có trong LoadPixelModel
        PixelModel model = LoadPixelModel(iconModelPath);

        // Khởi tạo Palette động dựa trên màu nhấn của màn hình để vẽ icon chuẩn xác
        std::map<int, Gdiplus::Color> palette;
        palette[1] = Gdiplus::Color(200, 10, 20, 30);    // Viền tối mờ
        palette[2] = Gdiplus::Color(255, gr, gg, gb);    // Màu chủ đạo (Accent)
        palette[3] = Gdiplus::Color(255, 255, 255, 255); // Màu trắng (Shine)
        palette[4] = Gdiplus::Color(160, gr, gg, gb);    // Màu phụ (Sub-accent)

        DrawPixelModel(g, model, bannerX + 25, iconY, iconSize, palette);
        DrawPixelModel(g, model, bannerX + bannerW - 25, iconY, iconSize, palette);
    }

    // 4. Chữ tiêu đề (Căn giữa và giới hạn vùng bao quanh)
    SetTextColor(hdc, textColor);
    HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Title);
    SetBkMode(hdc, TRANSPARENT);
    RECT r = {bannerX + 45, bannerY, bannerX + bannerW - 45, bannerY + bannerH};
    DrawTextW(hdc, text.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, oldF);
}

void DrawProceduralStadium(Gdiplus::Graphics &g, int screenWidth, int screenHeight, bool showFlashes, bool animate)
{
    // --- CACHE TẦNG TĨNH (Pitch & Lines) ---
    static Gdiplus::Bitmap *pitchCache = nullptr;
    static int cachedW = 0;
    static int cachedH = 0;

    if (!pitchCache || cachedW != screenWidth || cachedH != screenHeight)
    {
        if (pitchCache)
        {
            delete pitchCache;
            pitchCache = nullptr;
        }
        pitchCache = new Gdiplus::Bitmap(screenWidth, screenHeight, PixelFormat32bppARGB);
        Gdiplus::Graphics gP(pitchCache);
        gP.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

        // 1. Nền Cỏ sọc ngang
        int stripeHeight = UIScaler::SY(60);
        for (int y = 0; y < screenHeight; y += stripeHeight)
        {
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

    if (animate)
    {
        // 2. Hiệu ứng Camera Flash (CHỈ vẽ ở MenuScreen khi showFlashes = true)
        if (showFlashes)
        {
            const int flashCount = 15;
            for (int i = 0; i < flashCount; i++)
            {
                int fx = (i * 918273) % screenWidth;
                int fy = (i * 374621) % (screenHeight / 3);
                float phase = (i * 137) % 314 / 50.0f;

                float rawPulse = sin(g_GlobalAnimTime * 15.0f + phase);
                if (rawPulse > 0.92f)
                { // Ngưỡng cao hơn để chớp dứt khoát hơn
                    int alpha = (int)((rawPulse - 0.92f) * 12.5f * 255);
                    alpha = max(0, min(255, alpha));

                    Gdiplus::SolidBrush flashBrush(Gdiplus::Color((BYTE)alpha, 255, 255, 255));
                    // Vẽ một hình chữ thập đơn giản thay vì 3 hình chữ nhật
                    g.FillRectangle(&flashBrush, fx - 1, fy - 6, 2, 12);
                    g.FillRectangle(&flashBrush, fx - 6, fy - 1, 12, 2);
                }
            }
        }

        // 3. Rạch gió (Wind Streaks) bay ngang sân cỏ
        {
            const int WIND_COUNT = 8;
            static const struct WindLine
            {
                float yFrac, speed;
                int len, alpha;
            } WIND_LINES[] = {
                {0.18f, 55.0f, 160, 45}, {0.31f, 40.0f, 220, 50}, {0.44f, 60.0f, 140, 40}, {0.60f, 50.0f, 180, 45}, {0.72f, 45.0f, 200, 50}, {0.85f, 62.0f, 150, 42}, {0.24f, 80.0f, 90, 30}, {0.52f, 75.0f, 110, 35}};

            for (int i = 0; i < WIND_COUNT; i++)
            {
                const WindLine &wl = WIND_LINES[i];
                int wy = (int)(wl.yFrac * screenHeight);
                int wx = (int)fmod(wl.speed * g_GlobalAnimTime + i * (screenWidth / (float)WIND_COUNT), (float)(screenWidth + wl.len)) - wl.len;

                Gdiplus::SolidBrush windBrush(Gdiplus::Color((BYTE)wl.alpha, 255, 255, 255));
                g.FillRectangle(&windBrush, wx, wy, wl.len, UIScaler::SY(2));
            }
        }

        // 4. Đám mây trôi ngang
        {
            static PixelModel cloudModel;
            if (!cloudModel.isLoaded)
            {
                cloudModel = LoadPixelModel("Asset/models/bg/cloud.txt");
            }
            if (cloudModel.isLoaded)
            {
                static std::map<int, Gdiplus::Color> cloudPalette = {{1, Gdiplus::Color(170, 245, 248, 255)}};
                const float clouds[][3] = {{22.0f, 0.04f, 0.18f}, {14.0f, 0.10f, 0.12f}, {30.0f, 0.02f, 0.10f}};
                for (int i = 0; i < 3; i++)
                {
                    int cSize = (int)(screenWidth * clouds[i][2]);
                    int cx = (int)fmod(clouds[i][0] * g_GlobalAnimTime + i * (screenWidth / 3.0f), (float)(screenWidth + UIScaler::SX(200)));
                    int cy = (int)(clouds[i][1] * screenHeight) + (int)(sin(g_GlobalAnimTime * 0.8f + i) * UIScaler::SY(4));
                    DrawPixelModel(g, cloudModel, cx, cy, cSize, cloudPalette, 9991); // 9991 is clouds fixed palette hash
                }
            }
        }

        // 5. Bóng bay
        {
            static PixelModel balloonModel;
            if (!balloonModel.isLoaded)
            {
                balloonModel = LoadPixelModel("Asset/models/bg/balloon.txt");
            }
            if (balloonModel.isLoaded)
            {
                static const struct BalloonDef
                {
                    float s = 0.0f;
                    float x = 0.0f;
                    Gdiplus::Color c = Gdiplus::Color(0, 0, 0, 0);
                    Gdiplus::Color sh = Gdiplus::Color(0, 0, 0, 0);
                } bs[] = {
                    {28.0f, 0.10f, Gdiplus::Color(210, 230, 50, 50), Gdiplus::Color(255, 255, 180, 180)},
                    {20.0f, 0.35f, Gdiplus::Color(210, 50, 120, 220), Gdiplus::Color(255, 160, 200, 255)},
                    {35.0f, 0.60f, Gdiplus::Color(210, 50, 200, 80), Gdiplus::Color(255, 160, 255, 180)},
                    {24.0f, 0.82f, Gdiplus::Color(210, 220, 160, 30), Gdiplus::Color(255, 255, 230, 140)}};
                for (int i = 0; i < 4; i++)
                {
                    int bx = (int)(bs[i].x * screenWidth) + (int)(sin(g_GlobalAnimTime * 1.2f + i * 1.1f) * UIScaler::SX(18));
                    int by = screenHeight - (int)fmod(bs[i].s * g_GlobalAnimTime + i * (screenHeight / 4.0f), (float)(screenHeight + UIScaler::SY(120)));

                    std::map<int, Gdiplus::Color> bPalette = {{1, Gdiplus::Color(200, 30, 30, 30)}, {2, bs[i].c}, {3, bs[i].sh}};
                    // Balloons palette is unique per balloon but constant over time.
                    // Use a key based on balloon index i
                    DrawPixelModel(g, balloonModel, bx, by, UIScaler::S(48), bPalette, 8880 + i);
                }
            }
        }
    }
}

void DrawTextCentered(HDC hdc, const std::wstring &text, int y, int rightX, COLORREF color, HFONT hFont, int leftX)
{
    HFONT fontToUse = (hFont != nullptr) ? hFont : GlobalFont::Default;
    HFONT hOldFont = (HFONT)SelectObject(hdc, fontToUse);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);

    RECT rect = {leftX, y, rightX, y + 100};
    DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

    SelectObject(hdc, hOldFont);
}

void DrawGameBoard(Gdiplus::Graphics &g, HDC hdc, const PlayState *state, int cellSize, int offsetX, int offsetY)
{
    int size = state->boardSize;
    int boardLength = size * cellSize;

    // 1. Vẽ lưới (Grid) - GDI thuần
    HPEN hPen = CreatePen(PS_SOLID, max(1, UIScaler::S(2)), ToCOLORREF(Palette::White));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    for (int i = 0; i <= size; ++i)
    {
        int currX = offsetX + i * cellSize;
        int currY = offsetY + i * cellSize;
        MoveToEx(hdc, offsetX, currY, NULL);
        LineTo(hdc, offsetX + boardLength, currY);
        MoveToEx(hdc, currX, offsetY, NULL);
        LineTo(hdc, currX, offsetY + boardLength);
    }
    SelectObject(hdc, hOldPen);
    DeleteObject(hPen);

    // 2. GIAI ĐOẠN 1 (GDI+): Vẽ các hiệu ứng Highlight & Animation
    // Không tạo Graphics mới ở đây, dùng đối tượng g truyền từ ngoài vào để tránh overhead

    // Highlight ô thắng (Draw Win Brushes first)
    if (!state->winningCells.empty())
    {
        float wPulse = 0.5f + sin(g_GlobalAnimTime * 10.0f) * 0.5f;
        int wAlpha = (int)(100 + wPulse * 155);
        Gdiplus::SolidBrush *winBrush = GetCachedBrush(ToGdiColor(WithAlpha(Theme::WinCellFill, (BYTE)wAlpha)));
        float pWidth = 1.5f + wPulse * 2.0f;
        Gdiplus::Pen winPen(ToGdiColor(Theme::WinCellBorder), pWidth);

        for (const auto &wCell : state->winningCells)
        {
            int drawX = offsetX + wCell.second * cellSize;
            int drawY = offsetX + wCell.first * cellSize;
            // Kiểm tra lại logic tọa độ: state->board[r][c] -> r là Row (Y), c là Col (X)
            drawX = offsetX + wCell.second * cellSize;
            drawY = offsetY + wCell.first * cellSize;
            g.FillRectangle(winBrush, drawX + 1, drawY + 1, cellSize - 2, cellSize - 2);
            g.DrawRectangle(&winPen, drawX + 2, drawY + 2, cellSize - 4, cellSize - 4);
        }
    }

    // Highlight nước đi cuối & Con trỏ
    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            int drawX = offsetX + c * cellSize;
            int drawY = offsetY + r * cellSize;

            if (r == state->lastMoveRow && c == state->lastMoveCol)
            {
                int alpha = (int)(150 + sin(g_GlobalAnimTime * 8.0f) * 100);
                Gdiplus::SolidBrush *lastMoveBrush = GetCachedBrush(ToGdiColor(WithAlpha(Theme::LastMoveHighlight, (BYTE)max(0, min(255, alpha)))));
                g.FillRectangle(lastMoveBrush, drawX + 1, drawY + 1, cellSize - 1, cellSize - 1);
            }
        }
    }

    // Vẽ Con trỏ nếu đang chơi
    if (state->status == MATCH_PLAYING)
    {
        int cursorX = offsetX + state->cursorCol * cellSize;
        int cursorY = offsetY + state->cursorRow * cellSize;
        float pulse = 0.5f + sin(g_GlobalAnimTime * 8.0f) * 0.5f;
        Gdiplus::Color cursorColor = state->isP1Turn ? ToGdiColor(Palette::OrangeNormal) : ToGdiColor(Palette::CyanNormal);

        int glowAlpha = (int)(40 + pulse * 60);
        Gdiplus::SolidBrush *glowBrush = GetCachedBrush(Gdiplus::Color((BYTE)glowAlpha, cursorColor.GetR(), cursorColor.GetG(), cursorColor.GetB()));
        g.FillRectangle(glowBrush, cursorX + 1, cursorY + 1, cellSize - 1, cellSize - 1);

        Gdiplus::Pen cornerPen(cursorColor, (Gdiplus::REAL)UIScaler::S(3));
        int cornerLen = cellSize / 3;
        int offset = (int)(pulse * UIScaler::S(4));
        g.DrawLine(&cornerPen, cursorX - offset, cursorY - offset, cursorX - offset + cornerLen, cursorY - offset);
        g.DrawLine(&cornerPen, cursorX - offset, cursorY - offset, cursorX - offset, cursorY - offset + cornerLen);
        g.DrawLine(&cornerPen, cursorX + cellSize + offset, cursorY - offset, cursorX + cellSize + offset - cornerLen, cursorY - offset);
        g.DrawLine(&cornerPen, cursorX + cellSize + offset, cursorY - offset, cursorX + cellSize + offset, cursorY - offset + cornerLen);
        g.DrawLine(&cornerPen, cursorX - offset, cursorY + cellSize + offset, cursorX - offset + cornerLen, cursorY + cellSize + offset);
        g.DrawLine(&cornerPen, cursorX - offset, cursorY + cellSize + offset, cursorX - offset, cursorY + cellSize + offset - cornerLen);
        g.DrawLine(&cornerPen, cursorX + cellSize + offset, cursorY + cellSize + offset, cursorX + cellSize + offset - cornerLen, cursorY + cellSize + offset);
        g.DrawLine(&cornerPen, cursorX + cellSize + offset, cursorY + cellSize + offset, cursorX + cellSize + offset, cursorY + cellSize + offset - cornerLen);
        Gdiplus::Pen thinPen(cursorColor, 1.0f);
        g.DrawRectangle(&thinPen, cursorX, cursorY, cellSize, cellSize);
    }

    // 3. GIAI ĐOẠN 2 (GDI): Vẽ quân cờ (X/O) - Sau khi đã xong tất cả GDI+ để tránh Interleaving
    HFONT pieceFont = CreateFont(
        cellSize - 4, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Arial");
    HFONT oldFont = (HFONT)SelectObject(hdc, pieceFont);
    SetBkMode(hdc, TRANSPARENT);

    for (int r = 0; r < size; r++)
    {
        for (int c = 0; c < size; c++)
        {
            if (state->board[r][c] == CELL_EMPTY)
                continue;

            int drawX = offsetX + c * cellSize;
            int drawY = offsetY + r * cellSize;
            RECT cellRect = {drawX, drawY, drawX + cellSize, drawY + cellSize};

            if (state->board[r][c] == CELL_PLAYER1)
            {
                SetTextColor(hdc, ToCOLORREF(Palette::OrangeNormal));
                DrawTextW(hdc, L"X", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            else if (state->board[r][c] == CELL_PLAYER2)
            {
                SetTextColor(hdc, ToCOLORREF(Palette::CyanNormal));
                DrawTextW(hdc, L"O", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        }
    }
    SelectObject(hdc, oldFont);
    DeleteObject(pieceFont);
}

void SetTextColour(HDC hdc, COLORREF colour)
{
    ::SetTextColor(hdc, colour); // Gọi hàm chuẩn của Windows GDI
    SetBkMode(hdc, TRANSPARENT); // Đảm bảo chữ không có nền màu bao quanh
}

void DrawPixelAction(Gdiplus::Graphics &g, int cx, int cy, int size, PlayerState &state)
{
    if (size <= 0)
    {
        return;
    }
    // 1. Cập nhật Frame dựa trên thời gian (Delta Time)
    ULONGLONG now = GetTickCount64();
    if (now - state.lastFrameTime > (ULONGLONG)state.animationSpeed)
    {
        state.currentFrame++;
        state.lastFrameTime = now;
    }

    // 2. Tạo Cache Key (Bao gồm Type, Action, Frame và Flip)
    size_t cacheKey = state.avatarType;
    hash_combine(cacheKey, state.currentAction);
    hash_combine(cacheKey, state.currentFrame);
    hash_combine(cacheKey, state.flipH);
    hash_combine(cacheKey, size);

    if (g_ActionCache.find(cacheKey) == g_ActionCache.end())
    {
        std::string path = "Asset/models/avt_0" + std::to_string(state.avatarType) +
                           "/" + state.currentAction + "/f_" + std::to_string(state.currentFrame) + ".txt";

        PixelModel model = LoadPixelModel(path);

        // Nếu không load được frame này (hết hành động), reset về frame 0
        if (!model.isLoaded && state.currentFrame > 0)
        {
            state.currentFrame = 0;
            path = "Asset/models/avt_0" + std::to_string(state.avatarType) +
                   "/" + state.currentAction + "/f_0.txt";
            model = LoadPixelModel(path);

            // Cập nhật lại cacheKey cho f_0
            cacheKey = state.avatarType;
            hash_combine(cacheKey, state.currentAction);
            hash_combine(cacheKey, 0); // Frame 0
            hash_combine(cacheKey, state.flipH);
            hash_combine(cacheKey, size);

            if (g_ActionCache.find(cacheKey) != g_ActionCache.end() && g_ActionCache[cacheKey] != nullptr)
            {
                g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
                g.DrawImage(g_ActionCache[cacheKey], cx - (int)g_ActionCache[cacheKey]->GetWidth() / 2, cy - (int)g_ActionCache[cacheKey]->GetHeight() / 2);
                return;
            }
        }

        if (!model.isLoaded || model.width == 0)
        {
            // fallback sang avt_00 nếu load thất bại
            if (state.avatarType != 0)
            {
                std::string fallbackPath = "Asset/models/avt_00/" + state.currentAction + "/f_" + std::to_string(state.currentFrame) + ".txt";
                model = LoadPixelModel(fallbackPath);
            }

            if (!model.isLoaded)
            {
                g_ActionCache[cacheKey] = nullptr;
            }
        }

        // Nếu load thành công (hoặc qua fallback)
        if (model.isLoaded && model.width > 0)
        {
            int pSize = size / model.width;
            if (pSize < 1)
                pSize = 1;
            int bw = model.width * pSize + 4;
            int bh = model.height * pSize + 4;

            Gdiplus::Bitmap *bmp = new Gdiplus::Bitmap(bw, bh, PixelFormat32bppARGB);
            Gdiplus::Graphics gBmp(bmp);
            gBmp.SetSmoothingMode(Gdiplus::SmoothingModeNone);

            // Vẽ bóng
            Gdiplus::SolidBrush *shadowBrush = GetCachedBrush(ToGdiColor(Theme::ShadowMed));
            for (int r = 0; r < model.height; r++)
            {
                for (int c = 0; c < model.width; c++)
                {
                    if (model.data[r][c] == 0)
                        continue;
                    int dc = state.flipH ? (model.width - 1 - c) : c;
                    gBmp.FillRectangle(shadowBrush, dc * pSize + 2, r * pSize + 2, pSize, pSize);
                }
            }

            // Vẽ thân
            Gdiplus::Pen pixelPen(ToGdiColor(Theme::ShadowLight), 1.0f);
            for (int r = 0; r < model.height; r++)
            {
                for (int c = 0; c < model.width; c++)
                {
                    int val = model.data[r][c];
                    if (val == 0)
                        continue;
                    Gdiplus::Color color = (val == 7) ? ToGdiColor(Theme::AnimBoot) : (val == 6 ? ToGdiColor(Theme::AnimBall) : GetPaletteColor(state.avatarType, val));
                    Gdiplus::SolidBrush *b = GetCachedBrush(color);
                    int dc = state.flipH ? (model.width - 1 - c) : c;
                    gBmp.FillRectangle(b, dc * pSize, r * pSize, pSize, pSize);

                    gBmp.DrawRectangle(&pixelPen, dc * pSize, r * pSize, pSize, pSize);
                }
            }
            g_ActionCache[cacheKey] = bmp;
        }
    }

    Gdiplus::Bitmap *cachedBmp = g_ActionCache[cacheKey];
    if (cachedBmp)
    {
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBmp, cx - (int)cachedBmp->GetWidth() / 2, cy - (int)cachedBmp->GetHeight() / 2);
    }
}

void ClearUICaches()
{
    // Giải phóng Model Cache
    for (auto &pair : g_ModelCache)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }
    g_ModelCache.clear();

    // Giải phóng Avatar Cache
    for (auto &pair : g_AvatarCache)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }
    g_AvatarCache.clear();

    // Giải phóng Football Cache
    for (auto &pair : g_FootballCache)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }
    g_FootballCache.clear();

    // Giải phóng Trophy Cache
    for (auto &pair : g_TrophyCache)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }
    g_TrophyCache.clear();

    // Giải phóng Clock Cache
    for (auto &pair : g_ClockCache)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }
    g_ClockCache.clear();

    // Giải phóng Action Cache
    for (auto &pair : g_ActionCache)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }
    g_ActionCache.clear();

    // Raw Models
    g_RawModelCache.clear();

    // Brush cache
    for (auto &pair : g_BrushCache)
    {
        if (pair.second)
        {
            delete pair.second;
        }
    }
    g_BrushCache.clear();
}
