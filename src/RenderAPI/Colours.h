#ifndef GAME_COLORS_H
#define GAME_COLORS_H
#include <windows.h>
#include <gdiplus.h>

// =============================================================
// SmartColor: POD struct (Plain Old Data) - Không có phương thức bên trong
// =============================================================
struct SmartColor
{
    BYTE a;
    BYTE r;
    BYTE g;
    BYTE b;
};

// Các hàm bên ngoài tương tác với SmartColor (Đồng bộ phong cách lập trình hàm)
COLORREF ToCOLORREF(const SmartColor &sc);
Gdiplus::Color ToGdiColor(const SmartColor &sc);
SmartColor WithAlpha(const SmartColor &sc, BYTE newAlpha);

// =============================================================
// Palette: Bảng màu nguyên thủy
// =============================================================
namespace Palette
{
    constexpr SmartColor Transparent = {0, 0, 0, 0};
    constexpr SmartColor Black = {255, 0, 0, 0};
    constexpr SmartColor White = {255, 255, 255, 255};
    constexpr SmartColor WhiteSoft = {255, 240, 240, 240};

    constexpr SmartColor GrayLightest = {255, 245, 245, 245};
    constexpr SmartColor GrayLight = {255, 224, 224, 224};
    constexpr SmartColor GrayNormal = {255, 158, 158, 158};
    constexpr SmartColor GrayDark = {255, 97, 97, 97};
    constexpr SmartColor GrayDarkest = {255, 33, 33, 33};

    constexpr SmartColor RedLightest = {255, 255, 205, 210};
    constexpr SmartColor RedLight = {255, 239, 154, 154};
    constexpr SmartColor RedNormal = {255, 244, 67, 54};
    constexpr SmartColor RedDark = {255, 211, 47, 47};
    constexpr SmartColor RedDarkest = {255, 183, 28, 28};

    constexpr SmartColor PinkLightest = {255, 248, 187, 208};
    constexpr SmartColor PinkLight = {255, 240, 98, 146};
    constexpr SmartColor PinkNormal = {255, 233, 30, 99};
    constexpr SmartColor PinkDark = {255, 194, 24, 91};
    constexpr SmartColor PinkDarkest = {255, 13, 71, 161};

    constexpr SmartColor PurpleLightest = {255, 225, 190, 231};
    constexpr SmartColor PurpleLight = {255, 186, 104, 200};
    constexpr SmartColor PurpleNormal = {255, 156, 39, 176};
    constexpr SmartColor PurpleDark = {255, 123, 31, 162};
    constexpr SmartColor PurpleDarkest = {255, 74, 20, 140};

    constexpr SmartColor BlueLightest = {255, 187, 222, 251};
    constexpr SmartColor BlueLight = {255, 144, 202, 249};
    constexpr SmartColor BlueNormal = {255, 33, 150, 243};
    constexpr SmartColor BlueDark = {255, 25, 118, 210};
    constexpr SmartColor BlueDarkest = {255, 13, 71, 161};

    constexpr SmartColor CyanLightest = {255, 178, 235, 242};
    constexpr SmartColor CyanLight = {255, 77, 208, 225};
    constexpr SmartColor CyanNormal = {255, 0, 188, 212};
    constexpr SmartColor CyanDark = {255, 0, 151, 167};
    constexpr SmartColor CyanDarkest = {255, 0, 96, 100};

    constexpr SmartColor GreenLightest = {255, 200, 230, 201};
    constexpr SmartColor GreenLight = {255, 165, 214, 167};
    constexpr SmartColor GreenNormal = {255, 76, 175, 80};
    constexpr SmartColor GreenDark = {255, 56, 142, 60};
    constexpr SmartColor GreenDarkest = {255, 27, 94, 32};

    constexpr SmartColor YellowLightest = {255, 255, 249, 196};
    constexpr SmartColor YellowLight = {255, 255, 241, 118};
    constexpr SmartColor YellowNormal = {255, 255, 235, 59};
    constexpr SmartColor YellowDark = {255, 251, 192, 45};
    constexpr SmartColor YellowDarkest = {255, 245, 127, 23};

    constexpr SmartColor OrangeLightest = {255, 255, 224, 178};
    constexpr SmartColor OrangeLight = {255, 255, 183, 77};
    constexpr SmartColor OrangeNormal = {255, 255, 152, 0};
    constexpr SmartColor OrangeDark = {255, 245, 124, 0};
    constexpr SmartColor OrangeDarkest = {255, 230, 81, 0};

    constexpr SmartColor BrownLightest = {255, 215, 204, 200};
    constexpr SmartColor BrownLight = {255, 161, 136, 126};
    constexpr SmartColor BrownNormal = {255, 121, 85, 72};
    constexpr SmartColor BrownDark = {255, 93, 64, 55};
    constexpr SmartColor BrownDarkest = {255, 62, 39, 35};

    // --- Dải màu Da (Skin Gradient) ---
    constexpr SmartColor SkinLight = {255, 255, 224, 189};
    constexpr SmartColor SkinMid = {255, 255, 205, 148};
    constexpr SmartColor SkinShadow = {255, 190, 145, 105};

    // --- Dải màu Tóc/Lông mày (Hair Gradient) ---
    constexpr SmartColor HairDarkest = {255, 35, 25, 20};
    constexpr SmartColor HairNormal = {255, 75, 55, 45};
    constexpr SmartColor HairHighlight = {255, 120, 100, 85};
}

// =============================================================
// Theme: Các màu thể hiện ý nghĩa (Semantic colors), bao gồm Alpha
// =============================================================
namespace Theme
{
    // ----- Đen mờ (Overlay / Shadow) -----
    constexpr SmartColor ShadowLight = {30, 0, 0, 0};    // Viền pixel nhạt
    constexpr SmartColor ShadowMed = {100, 0, 0, 0};     // Bóng avatar
    constexpr SmartColor ShadowPanel = {150, 0, 0, 0};   // Menu overlay
    constexpr SmartColor ShadowHeavy = {180, 0, 0, 0};   // Glassmorphism panel
    constexpr SmartColor ShadowOverlay = {200, 0, 0, 0}; // Phủ màn hình

    // ----- Nền Panel kính (Glassmorphism) -----
    constexpr SmartColor GlassWhite = {220, 255, 255, 255}; // Panel trắng mờ
    constexpr SmartColor GlassDark = {200, 15, 20, 30};     // Panel tối (pause)
    constexpr SmartColor GlassGleam = {150, 255, 255, 255}; // Viền trắng mờ

    // ----- Màu Sân cỏ (Pitch) -----
    constexpr SmartColor PitchDark = {255, 23, 90, 34};    // Cỏ tối
    constexpr SmartColor PitchLight = {255, 40, 145, 40};  // Cỏ sáng
    constexpr SmartColor PitchLine = {150, 255, 255, 255}; // Vạch vôi
    constexpr SmartColor PitchDot = {150, 255, 255, 255};  // Chấm giữa sân

    // ----- Viền Panel màu chủ đề -----
    constexpr SmartColor PanelGreenBorder = {200, 34, 139, 34};
    constexpr SmartColor PanelBlueBorder = {180, 0, 150, 255};
    constexpr SmartColor PanelOrangeBorder = {200, 255, 120, 0};
    constexpr SmartColor PanelGoldBorder = {200, 255, 215, 0};
    constexpr SmartColor PanelYellowBorder = {255, 255, 200, 0};

    // ----- Màu cầu thủ P1 (Cam / Đỏ-Cam) -----
    constexpr SmartColor P1TurnPulse = {30, 255, 150, 0};
    constexpr SmartColor P1TurnBorder = {200, 255, 150, 0};
    constexpr SmartColor P1Watermark = {60, 255, 150, 0};

    // ----- Màu cầu thủ P2 (Cyan) -----
    constexpr SmartColor P2TurnPulse = {30, 0, 255, 255};
    constexpr SmartColor P2TurnBorder = {200, 0, 255, 255};
    constexpr SmartColor P2Watermark = {60, 0, 255, 255};

    // ----- Màu bàn cờ -----
    constexpr SmartColor LastMoveHighlight = {150, 255, 255, 0};
    constexpr SmartColor WinCellFill = {200, 50, 255, 50};
    constexpr SmartColor WinCellBorder = {200, 0, 255, 80};

    // ----- Màu trái bóng Pixel -----
    constexpr SmartColor FootballDark = {255, 30, 30, 30};
    constexpr SmartColor FootballLight = {255, 240, 240, 240};

    // ----- Màu Cúp (Trophy) -----
    constexpr SmartColor TrophyRim = {255, 200, 150, 20};
    constexpr SmartColor TrophyBody = {255, 250, 210, 50};
    constexpr SmartColor TrophyShine = {200, 255, 255, 255};

    // ----- Palette tiêu đề "CARO" -----
    constexpr SmartColor TitleBorder = {255, 200, 80, 0};
    constexpr SmartColor TitleFill = {255, 255, 220, 0};
    constexpr SmartColor TitleShadow = {150, 0, 0, 0};

    // ----- Màu Avatar Pixel -----
    // ----- Màu Avatar Cố định (Named Players) -----
    constexpr SmartColor AvaOutline = {255, 30, 30, 30};
    constexpr SmartColor AvaEye = {255, 0, 0, 0};

    // Bot AI Palettes
    constexpr SmartColor BotEasy_Body = {255, 180, 180, 180};
    constexpr SmartColor BotEasy_Accent = {255, 200, 100, 50};

    constexpr SmartColor BotMed_Body = {255, 120, 200, 120};
    constexpr SmartColor BotMed_Dark = {255, 50, 150, 50};
    constexpr SmartColor BotMed_Darkest = {255, 20, 100, 20};

    constexpr SmartColor BotHard_Body = {255, 80, 40, 40};
    constexpr SmartColor BotHard_Shirt = {255, 250, 30, 30};
    constexpr SmartColor BotHard_Dark = {255, 20, 20, 20};

    // Animation: Boot (giày) và Football (bóng)
    constexpr SmartColor AnimBoot = {255, 40, 30, 20};
    constexpr SmartColor AnimBall = {255, 25, 25, 25};

    // ----- Khung nhập tên (Save name box) -----
    constexpr SmartColor SaveBoxBorder = {255, 255, 165, 0};

    // ----- Slot load game -----
    constexpr SmartColor SlotSelected = {150, 255, 180, 50};
    constexpr SmartColor SlotNormal = {100, 200, 200, 200};

    // ----- Thanh trượt âm lượng (Volume Bar) -----
    constexpr SmartColor BarTrack = {255, 200, 200, 200};
    constexpr SmartColor BarFillSelected = {255, 255, 120, 0};
    constexpr SmartColor BarFillNormal = {255, 50, 150, 250};

    // ----- Flash khán đài -----
    constexpr SmartColor CameraFlash = {255, 255, 255, 255}; // (alpha dùng động)

    // ----- Banner thắng (Victory Banner) -----
    constexpr SmartColor BannerBgBase = {220, 0, 0, 0};

    // ----- Pitch board -----
    constexpr SmartColor BoardPitch = {255, 56, 142, 60};
    constexpr SmartColor BoardBorder = {255, 255, 255, 255};

    // --- Cập nhật ID chuẩn (1-8) cho các Cầu thủ ---

    // Ronaldo (P_CR7)
    constexpr SmartColor CR7_Outline = {255, 0, 0, 0};     // ID 1
    constexpr SmartColor CR7_SkinL = {255, 255, 224, 189}; // ID 2
    constexpr SmartColor CR7_SkinM = {255, 255, 205, 148}; // ID 3
    constexpr SmartColor CR7_SkinD = {255, 190, 145, 105}; // ID 4
    constexpr SmartColor CR7_Eye = {255, 0, 0, 0};         // ID 5
    constexpr SmartColor CR7_HairD = {255, 35, 25, 20};    // ID 6
    constexpr SmartColor CR7_Teeth = {255, 255, 255, 255}; // ID 7
    constexpr SmartColor CR7_HairN = {255, 75, 55, 45};    // ID 8

    // Messi (P_MES) - Sáng hơn CR7
    constexpr SmartColor MES_Outline = {255, 0, 0, 0};      // ID 1
    constexpr SmartColor MES_SkinL = {255, 255, 235, 205};  // ID 2
    constexpr SmartColor MES_SkinM = {255, 255, 215, 165};  // ID 3
    constexpr SmartColor MES_SkinD = {255, 210, 165, 125};  // ID 4
    constexpr SmartColor MES_Eye = {255, 0, 0, 0};          // ID 5
    constexpr SmartColor MES_HairD = {255, 65, 45, 35};     // ID 6
    constexpr SmartColor MES_Teeth = {255, 255, 255, 255};  // ID 7
    constexpr SmartColor MES_HairN = {255, 95, 75, 65};     // ID 8
    constexpr SmartColor MES_ShirtL = {255, 117, 190, 233}; // ID 15
    constexpr SmartColor MES_ShirtD = {255, 60, 140, 190};  // ID 16
    constexpr SmartColor MES_Shorts = {255, 20, 20, 30};    // ID 17

    // Neymar (P_NEY) - Đậm/Nâu hơn CR7
    constexpr SmartColor NEY_Outline = {255, 0, 0, 0};     // ID 1
    constexpr SmartColor NEY_SkinL = {255, 245, 210, 175}; // ID 2
    constexpr SmartColor NEY_SkinM = {255, 235, 185, 130}; // ID 3
    constexpr SmartColor NEY_SkinD = {255, 170, 125, 85};  // ID 4
    constexpr SmartColor NEY_Eye = {255, 0, 0, 0};         // ID 5
    constexpr SmartColor NEY_HairD = {255, 25, 15, 10};    // ID 6
    constexpr SmartColor NEY_Teeth = {255, 255, 255, 255}; // ID 7
    constexpr SmartColor NEY_HairN = {255, 65, 45, 35};    // ID 8
    constexpr SmartColor NEY_Shirt = {255, 255, 235, 59};  // ID 13
    constexpr SmartColor NEY_ShirtAcc = {255, 0, 150, 0};  // ID 18
    constexpr SmartColor NEY_Shorts = {255, 0, 80, 180};   // ID 15
}

#endif // GAME_COLORS_H