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
    for (int row = 0; row < model.height; ++row)
    {
        if (!std::getline(file, line))
            break;
        std::stringstream ss(line);
        for (int col = 0; col < model.width; ++col)
        {
            int val = 0;
            if (ss >> val)
                model.data[row][col] = val;
        }
    }
    model.isLoaded = true;
    g_RawModelCache[filePath] = model; // Lưu vào cache
    return model;
}

void DrawPixelModel(Gdiplus::Graphics &g, const PixelModel &model, int centerX, int centerY, int totalSize, const std::map<int, Gdiplus::Color> &palette, size_t manualPaletteHash)
{
    if (!model.isLoaded || model.width == 0 || model.height == 0 || totalSize <= 0)
    {
        return;
    }

    // Tự động tính toán kích thước mỗi điểm ảnh dựa trên diện tích mục tiêu
    int pixelSize = totalSize / max(model.width, model.height);
    if (pixelSize < 1)
        pixelSize = 1;

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
        int totalW = model.width * pixelSize;
        int totalH = model.height * pixelSize;

        // Tạo bitmap đệm với khoảng trống nhỏ để tránh răng cưa viền
        Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(totalW + 2, totalH + 2, PixelFormat32bppARGB);
        Gdiplus::Graphics bitmapGraphics(bitmap);
        bitmapGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone); // Giữ pixel sắc nét

        Gdiplus::Pen gridPen(Gdiplus::Color(60, 0, 0, 0), 1.0f);
        for (int row = 0; row < model.height; ++row)
        {
            for (int col = 0; col < model.width; ++col)
            {
                int val = model.data[row][col];
                if (val != 0)
                {
                    auto it = palette.find(val);
                    if (it != palette.end())
                    {
                        Gdiplus::SolidBrush *brushLocal = GetCachedBrush(it->second);
                        bitmapGraphics.FillRectangle(brushLocal, col * pixelSize, row * pixelSize, pixelSize, pixelSize);

                        // Vẽ lưới pixel mờ để giữ phong cách Retro
                        bitmapGraphics.DrawRectangle(&gridPen, col * pixelSize, row * pixelSize, pixelSize, pixelSize);
                    }
                }
            }
        }
        g_ModelCache[key] = bitmap;
    }
    Gdiplus::Bitmap *cachedBitmap = g_ModelCache[key];
    if (cachedBitmap)
    {
        // --- High Performance Rendering Settings ---
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

        // Vẽ bitmap đã cache ra tâm centerX, centerY
        g.DrawImage(cachedBitmap, centerX - (int)cachedBitmap->GetWidth() / 2, centerY - (int)cachedBitmap->GetHeight() / 2);
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

void DrawPixelAvatar(Gdiplus::Graphics &g, int centerX, int centerY, int size, int avatarType)
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
            int bitmapWidth = size + shadowOffset + 4;
            int bitmapHeight = size + shadowOffset + 4;

            Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(bitmapWidth, bitmapHeight, PixelFormat32bppARGB);
            Gdiplus::Graphics bitmapGraphics(bitmap);

            // Vẽ bóng (Shadow)
            Gdiplus::SolidBrush *shadowBrush = GetCachedBrush(ToGdiColor(Theme::ShadowMed));
            for (int row = 0; row < model.height; row++)
            {
                for (int col = 0; col < model.width; col++)
                {
                    if (model.data[row][col] != 0)
                    {
                        bitmapGraphics.FillRectangle(shadowBrush, col * pixelSize + shadowOffset, row * pixelSize + shadowOffset, pixelSize, pixelSize);
                    }
                }
            }

            // Vẽ nhân vật chính
            Gdiplus::Pen pixelPen(ToGdiColor(Theme::ShadowLight), 1.0f);
            for (int row = 0; row < model.height; row++)
            {
                for (int col = 0; col < model.width; col++)
                {
                    int code = model.data[row][col];
                    if (code != 0)
                    {
                        Gdiplus::Color color = GetPaletteColor(avatarType, code);
                        Gdiplus::SolidBrush *brushLocal = GetCachedBrush(color);
                        bitmapGraphics.FillRectangle(brushLocal, col * pixelSize, row * pixelSize, pixelSize, pixelSize);

                        // 2. Chỉ vẽ viền lưới cho các nhân vật ít chi tiết
                        if (avatarType < 6)
                        {
                            bitmapGraphics.DrawRectangle(&pixelPen, col * pixelSize, row * pixelSize, pixelSize, pixelSize);
                        }
                    }
                }
            }
            g_AvatarCache[cacheKey] = bitmap;
        }
    }

    Gdiplus::Bitmap *cachedBitmap = g_AvatarCache[cacheKey];
    if (cachedBitmap)
    {
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBitmap, (float)centerX, (float)centerY);
    }
}

void DrawPixelFootball(Gdiplus::Graphics &g, int centerX, int centerY, int size)
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
            int pixelSize = size / model.width;
            if (pixelSize < 1)
                pixelSize = 1;
            Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics bitmapGraphics(bitmap);
            for (int row = 0; row < model.height; row++)
            {
                for (int col = 0; col < model.width; col++)
                {
                    int val = model.data[row][col];
                    if (val == 0)
                        continue;
                    Gdiplus::Color color = (val == 1) ? ToGdiColor(Theme::FootballDark) : ToGdiColor(Theme::FootballLight);
                    Gdiplus::SolidBrush brushLocal(color);
                    bitmapGraphics.FillRectangle(&brushLocal, col * pixelSize, row * pixelSize, pixelSize, pixelSize);
                }
            }
            g_FootballCache[size] = bitmap;
        }
    }
    Gdiplus::Bitmap *cachedBitmap = g_FootballCache[size];
    if (cachedBitmap)
    {
        float pulse = 1.0f + sin(g_GlobalAnimTime * 15.0f) * 0.05f;
        int scaledSize = (int)(size * pulse);
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBitmap, centerX - scaledSize / 2, centerY - scaledSize / 2, scaledSize, scaledSize);
    }
}

void DrawPixelTrophy(Gdiplus::Graphics &g, int centerX, int centerY, int size)
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
            int pixelScale = size / model.width;
            if (pixelScale < 1)
                pixelScale = 1;
            Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics bitmapGraphics(bitmap);
            for (int row = 0; row < model.height; row++)
            {
                for (int col = 0; col < model.width; col++)
                {
                    int val = model.data[row][col];
                    if (val == 0)
                        continue;
                    Gdiplus::Color color = (val == 1) ? ToGdiColor(Theme::TrophyRim) : (val == 2 ? ToGdiColor(Theme::TrophyBody) : ToGdiColor(Theme::TrophyShine));
                    Gdiplus::SolidBrush brushLocal(color);
                    bitmapGraphics.FillRectangle(&brushLocal, col * pixelScale, row * pixelScale, pixelScale, pixelScale);
                    Gdiplus::Pen localPen(ToGdiColor(Theme::ShadowLight), 1.0f);
                    bitmapGraphics.DrawRectangle(&localPen, col * pixelScale, row * pixelScale, pixelScale, pixelScale);
                }
            }
            g_TrophyCache[size] = bitmap;
        }
    }
    Gdiplus::Bitmap *cachedBitmap = g_TrophyCache[size];
    if (cachedBitmap)
    {
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBitmap, centerX - size / 2, centerY - size / 2, size, size);
    }
}

void DrawPixelClock(Gdiplus::Graphics &g, int centerX, int centerY, int size, Gdiplus::Color color)
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
            int pixelSize = size / model.width;
            if (pixelSize < 1)
                pixelSize = 1;
            Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(size + 2, size + 2, PixelFormat32bppARGB);
            Gdiplus::Graphics bitmapGraphics(bitmap);
            Gdiplus::SolidBrush darkBrush(Gdiplus::Color(180, 20, 20, 30));
            Gdiplus::SolidBrush shineBrush(Gdiplus::Color(255, 255, 255, 255));
            Gdiplus::SolidBrush mainBrush(color);
            for (int row = 0; row < model.height; row++)
            {
                for (int col = 0; col < model.width; col++)
                {
                    int val = model.data[row][col];
                    if (val == 0)
                        continue;
                    Gdiplus::SolidBrush *brushLocal = &mainBrush;
                    if (val == 1)
                        brushLocal = &darkBrush;
                    if (val == 3)
                        brushLocal = &shineBrush;
                    bitmapGraphics.FillRectangle(brushLocal, col * pixelSize, row * pixelSize, pixelSize, pixelSize);
                    Gdiplus::Pen localPen(Gdiplus::Color(60, 0, 0, 0), 1.0f);
                    bitmapGraphics.DrawRectangle(&localPen, col * pixelSize, row * pixelSize, pixelSize, pixelSize);
                }
            }
            g_ClockCache[key] = bitmap;
        }
    }
    Gdiplus::Bitmap *cachedBitmap = g_ClockCache[key];
    if (cachedBitmap)
    {
        float pulse = 0.8f + sin(g_GlobalAnimTime * 8.0f) * 0.2f;
        int scaledSize = (int)(size * (0.95f + pulse * 0.05f));
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBitmap, centerX - scaledSize / 2, centerY - scaledSize / 2, scaledSize, scaledSize);
    }
}

// =============================================================
// DrawPixelBanner: Tiêu đề procedural thay chữ thuần
// Nền gradient tối + viền sáng pulse + 2 icon bóng 2 bên + chữ GDI
// =============================================================
void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text,
                     int centerX, int centerY, int panelWidth, COLORREF textColor, COLORREF glowColor)
{
    DrawPixelBanner(g, hdc, text, centerX, centerY, panelWidth, textColor, glowColor, "");
}

void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text,
                     int centerX, int centerY, int panelWidth, COLORREF textColor, COLORREF glowColor, const std::string &iconModelPath)
{
    int bannerW = panelWidth - UIScaler::SX(24);
    int bannerH = UIScaler::SY(50);
    int bannerX = centerX - bannerW / 2;
    int bannerY = centerY - bannerH / 2;

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
    BYTE gr = GetRValue(glowColor);
    BYTE gg = GetGValue(glowColor);
    BYTE gb = GetBValue(glowColor);
    Gdiplus::Pen topLine(Gdiplus::Color(lineA, gr, gg, gb), 2.5f);
    Gdiplus::Pen botLine(Gdiplus::Color((BYTE)(lineA * 0.6f), gr, gg, gb), 1.5f);
    g.DrawLine(&topLine, bannerX, bannerY, bannerX + bannerW, bannerY);
    g.DrawLine(&botLine, bannerX, bannerY + bannerH, bannerX + bannerW, bannerY + bannerH);

    Gdiplus::SolidBrush cornerBrush(Gdiplus::Color(80, gr, gg, gb));
    g.FillRectangle(&cornerBrush, bannerX, bannerY, 4, bannerH);
    g.FillRectangle(&cornerBrush, bannerX + bannerW - 4, bannerY, 4, bannerH);

    // 3. Icon trang trí (Football mặc định hoặc Tùy chỉnh)
    int iconY = centerY;
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

void DrawProceduralStadium(Gdiplus::Graphics &g, int screenWidth, int screenHeight, bool shouldShowFlashes, bool shouldAnimate)
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
        for (int stripeY = 0; stripeY < screenHeight; stripeY += stripeHeight)
        {
            bool isDark = (stripeY / stripeHeight) % 2 == 0;
            Gdiplus::SolidBrush stripeBrush(isDark ? ToGdiColor(Theme::PitchDark) : ToGdiColor(Theme::PitchLight));
            gP.FillRectangle(&stripeBrush, 0, stripeY, screenWidth, stripeHeight);
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

    if (shouldAnimate)
    {
        // 2. Hiệu ứng Camera Flash (CHỈ vẽ ở MenuScreen khi shouldShowFlashes = true)
        if (shouldShowFlashes)
        {
            const int flashCount = 15;
            for (int flashIndex = 0; flashIndex < flashCount; ++flashIndex)
            {
                int flashX = (flashIndex * 918273) % screenWidth;
                int flashY = (flashIndex * 374621) % (screenHeight / 3);
                float flashPhase = (flashIndex * 137) % 314 / 50.0f;

                float rawPulse = sin(g_GlobalAnimTime * 15.0f + flashPhase);
                if (rawPulse > 0.92f)
                { // Ngưỡng cao hơn để chớp dứt khoát hơn
                    int alpha = (int)((rawPulse - 0.92f) * 12.5f * 255);
                    alpha = max(0, min(255, alpha));

                    Gdiplus::SolidBrush flashBrush(Gdiplus::Color((BYTE)alpha, 255, 255, 255));
                    // Vẽ một hình chữ thập đơn giản thay vì 3 hình chữ nhật
                    g.FillRectangle(&flashBrush, flashX - 1, flashY - 6, 2, 12);
                    g.FillRectangle(&flashBrush, flashX - 6, flashY - 1, 12, 2);
                }
            }
        }

        // 3. Rạch gió (Wind Streaks) bay ngang sân cỏ
        {
            const int WIND_COUNT = 8;
            static const struct WindLine
            {
                float yFrac;
                float speed;
                int len;
                int alpha;
            } WIND_LINES[] = {
                {0.18f, 55.0f, 160, 45}, {0.31f, 40.0f, 220, 50}, {0.44f, 60.0f, 140, 40}, {0.60f, 50.0f, 180, 45}, {0.72f, 45.0f, 200, 50}, {0.85f, 62.0f, 150, 42}, {0.24f, 80.0f, 90, 30}, {0.52f, 75.0f, 110, 35}};

            for (int windIndex = 0; windIndex < WIND_COUNT; ++windIndex)
            {
                const WindLine &windLine = WIND_LINES[windIndex];
                int windY = (int)(windLine.yFrac * screenHeight);
                int windX = (int)fmod(windLine.speed * g_GlobalAnimTime + windIndex * (screenWidth / (float)WIND_COUNT), (float)(screenWidth + windLine.len)) - windLine.len;

                Gdiplus::SolidBrush windBrush(Gdiplus::Color((BYTE)windLine.alpha, 255, 255, 255));
                g.FillRectangle(&windBrush, windX, windY, windLine.len, UIScaler::SY(2));
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
                for (int cloudIndex = 0; cloudIndex < 3; ++cloudIndex)
                {
                    int cloudSize = (int)(screenWidth * clouds[cloudIndex][2]);
                    int cloudCenterX = (int)fmod(clouds[cloudIndex][0] * g_GlobalAnimTime + cloudIndex * (screenWidth / 3.0f), (float)(screenWidth + UIScaler::SX(200)));
                    int cloudCenterY = (int)(clouds[cloudIndex][1] * screenHeight) + (int)(sin(g_GlobalAnimTime * 0.8f + cloudIndex) * UIScaler::SY(4));
                    DrawPixelModel(g, cloudModel, cloudCenterX, cloudCenterY, cloudSize, cloudPalette, 9991); // 9991 is clouds fixed palette hash
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
                    float speed = 0.0f;
                    float xFraction = 0.0f;
                    Gdiplus::Color color = Gdiplus::Color(0, 0, 0, 0);
                    Gdiplus::Color shadow = Gdiplus::Color(0, 0, 0, 0);
                } balloonDefs[] = {
                    {28.0f, 0.10f, Gdiplus::Color(210, 230, 50, 50), Gdiplus::Color(255, 255, 180, 180)},
                    {20.0f, 0.35f, Gdiplus::Color(210, 50, 120, 220), Gdiplus::Color(255, 160, 200, 255)},
                    {35.0f, 0.60f, Gdiplus::Color(210, 50, 200, 80), Gdiplus::Color(255, 160, 255, 180)},
                    {24.0f, 0.82f, Gdiplus::Color(210, 220, 160, 30), Gdiplus::Color(255, 255, 230, 140)}};
                for (int balloonIndex = 0; balloonIndex < 4; ++balloonIndex)
                {
                    int balloonCenterX = (int)(balloonDefs[balloonIndex].xFraction * screenWidth) + (int)(sin(g_GlobalAnimTime * 1.2f + balloonIndex * 1.1f) * UIScaler::SX(18));
                    int balloonCenterY = screenHeight - (int)fmod(balloonDefs[balloonIndex].speed * g_GlobalAnimTime + balloonIndex * (screenHeight / 4.0f), (float)(screenHeight + UIScaler::SY(120)));

                    std::map<int, Gdiplus::Color> balloonPalette = {{1, Gdiplus::Color(200, 30, 30, 30)}, {2, balloonDefs[balloonIndex].color}, {3, balloonDefs[balloonIndex].shadow}};
                    // Balloons palette is unique per balloon but constant over time.
                    // Use a key based on balloon index
                    DrawPixelModel(g, balloonModel, balloonCenterX, balloonCenterY, UIScaler::S(48), balloonPalette, 8880 + balloonIndex);
                }
            }
        }
    }
}

void DrawTextCentered(HDC hdc, const std::wstring &text, int posY, int rightX, COLORREF color, HFONT hFont, int leftX)
{
    HFONT fontToUse = (hFont != nullptr) ? hFont : GlobalFont::Default;
    HFONT hOldFont = (HFONT)SelectObject(hdc, fontToUse);
    SetTextColor(hdc, color);
    SetBkMode(hdc, TRANSPARENT);
    RECT rect = {leftX, posY, rightX, posY + 100};
    DrawTextW(hdc, text.c_str(), -1, &rect, DT_CENTER | DT_SINGLELINE | DT_NOPREFIX);

    SelectObject(hdc, hOldFont);
}

void DrawGameBoard(Gdiplus::Graphics &g, HDC hdc, const PlayState *state, int cellSize, int offsetX, int offsetY)
{
    int size = state->boardSize;
    int boardPixelLength = size * cellSize;

    // 1. Vẽ lưới (Grid) - GDI thuần
    HPEN hPen = CreatePen(PS_SOLID, max(1, UIScaler::S(2)), ToCOLORREF(Palette::White));
    HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

    for (int index = 0; index <= size; ++index)
    {
        int currX = offsetX + index * cellSize;
        int currY = offsetY + index * cellSize;
        MoveToEx(hdc, offsetX, currY, NULL);
        LineTo(hdc, offsetX + boardPixelLength, currY);
        MoveToEx(hdc, currX, offsetY, NULL);
        LineTo(hdc, currX, offsetY + boardPixelLength);
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
            int drawY = offsetY + wCell.first * cellSize;
            g.FillRectangle(winBrush, drawX + 1, drawY + 1, cellSize - 2, cellSize - 2);
            g.DrawRectangle(&winPen, drawX + 2, drawY + 2, cellSize - 4, cellSize - 4);
        }
    }

    // Highlight nước đi cuối & Con trỏ
    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            int drawX = offsetX + col * cellSize;
            int drawY = offsetY + row * cellSize;

            if (row == state->lastMoveRow && col == state->lastMoveCol)
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
        Gdiplus::Color cursorColor = state->isPlayer1Turn ? ToGdiColor(Palette::OrangeNormal) : ToGdiColor(Palette::CyanNormal);

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

    for (int row = 0; row < size; row++)
    {
        for (int col = 0; col < size; col++)
        {
            if (state->board[row][col] == CELL_EMPTY)
                continue;

            int drawX = offsetX + col * cellSize;
            int drawY = offsetY + row * cellSize;
            RECT cellRect = {drawX, drawY, drawX + cellSize, drawY + cellSize};

            if (state->board[row][col] == CELL_PLAYER1)
            {
                SetTextColor(hdc, ToCOLORREF(Palette::OrangeNormal));
                DrawTextW(hdc, L"X", -1, &cellRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            else if (state->board[row][col] == CELL_PLAYER2)
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

void DrawPixelAction(Gdiplus::Graphics &g, int centerX, int centerY, int size, PlayerState &state)
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
                g.DrawImage(g_ActionCache[cacheKey], centerX - (int)g_ActionCache[cacheKey]->GetWidth() / 2, centerY - (int)g_ActionCache[cacheKey]->GetHeight() / 2);
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
            int pixelSize = size / model.width;
            if (pixelSize < 1)
                pixelSize = 1;
            int bitmapWidth = model.width * pixelSize + 4;
            int bitmapHeight = model.height * pixelSize + 4;

            Gdiplus::Bitmap *bitmap = new Gdiplus::Bitmap(bitmapWidth, bitmapHeight, PixelFormat32bppARGB);
            Gdiplus::Graphics bitmapGraphics(bitmap);
            bitmapGraphics.SetSmoothingMode(Gdiplus::SmoothingModeNone);

            // Vẽ bóng
            Gdiplus::SolidBrush *shadowBrush = GetCachedBrush(ToGdiColor(Theme::ShadowMed));
            for (int row = 0; row < model.height; row++)
            {
                for (int col = 0; col < model.width; col++)
                {
                    if (model.data[row][col] == 0)
                        continue;
                    int drawColIndex = state.flipH ? (model.width - 1 - col) : col;
                    bitmapGraphics.FillRectangle(shadowBrush, drawColIndex * pixelSize + 2, row * pixelSize + 2, pixelSize, pixelSize);
                }
            }

            // Vẽ thân
            Gdiplus::Pen pixelPen(ToGdiColor(Theme::ShadowLight), 1.0f);
            for (int row = 0; row < model.height; row++)
            {
                for (int col = 0; col < model.width; col++)
                {
                    int val = model.data[row][col];
                    if (val == 0)
                        continue;
                    Gdiplus::Color color = (val == 7) ? ToGdiColor(Theme::AnimBoot) : (val == 6 ? ToGdiColor(Theme::AnimBall) : GetPaletteColor(state.avatarType, val));
                    Gdiplus::SolidBrush *brushLocal = GetCachedBrush(color);
                    int drawColIndex = state.flipH ? (model.width - 1 - col) : col;
                    bitmapGraphics.FillRectangle(brushLocal, drawColIndex * pixelSize, row * pixelSize, pixelSize, pixelSize);

                    bitmapGraphics.DrawRectangle(&pixelPen, drawColIndex * pixelSize, row * pixelSize, pixelSize, pixelSize);
                }
            }
            g_ActionCache[cacheKey] = bitmap;
        }
    }

    Gdiplus::Bitmap *cachedBitmap = g_ActionCache[cacheKey];
    if (cachedBitmap)
    {
        g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
        g.DrawImage(cachedBitmap, centerX - (int)cachedBitmap->GetWidth() / 2, centerY - (int)cachedBitmap->GetHeight() / 2);
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
