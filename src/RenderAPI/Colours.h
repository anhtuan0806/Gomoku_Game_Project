#ifndef GAME_COLORS_H
#define GAME_COLORS_H
#include <windows.h>
#include <gdiplus.h>

// =============================================================
// SmartColor: Tự động tương thích với Win32 COLORREF và Gdiplus::Color
// =============================================================
struct SmartColor {
    BYTE a, r, g, b;

    constexpr SmartColor(BYTE a, BYTE r, BYTE g, BYTE b) : a(a), r(r), g(g), b(b) {}
    constexpr SmartColor(BYTE r, BYTE g, BYTE b) : a(255), r(r), g(g), b(b) {}

    // Implicit cast to Win32 COLORREF (Bo qua Alpha)
    constexpr operator COLORREF() const {
        return RGB(r, g, b);
    }

    // Implicit cast to Gdiplus::Color
    inline operator Gdiplus::Color() const {
        return Gdiplus::Color(a, r, g, b);
    }

    // Explicit alpha modifier (Tao mau moi voi do giam alpha)
    constexpr SmartColor WithAlpha(BYTE newA) const {
        return SmartColor(newA, r, g, b);
    }
};

// =============================================================
// Palette: Bang mau nguyen thuy
// =============================================================
namespace Palette {
    constexpr SmartColor Transparent(0, 0, 0, 0);
    constexpr SmartColor Black(0, 0, 0);
    constexpr SmartColor White(255, 255, 255);
    constexpr SmartColor WhiteSoft(240, 240, 240);

    constexpr SmartColor GrayLightest(245, 245, 245);
    constexpr SmartColor GrayLight(224, 224, 224);
    constexpr SmartColor GrayNormal(158, 158, 158);
    constexpr SmartColor GrayDark(97, 97, 97);
    constexpr SmartColor GrayDarkest(33, 33, 33);

    constexpr SmartColor RedLightest(255, 205, 210);
    constexpr SmartColor RedLight(239, 154, 154);
    constexpr SmartColor RedNormal(244, 67, 54);
    constexpr SmartColor RedDark(211, 47, 47);
    constexpr SmartColor RedDarkest(183, 28, 28);

    constexpr SmartColor PinkLightest(248, 187, 208);
    constexpr SmartColor PinkLight(240, 98, 146);
    constexpr SmartColor PinkNormal(233, 30, 99);
    constexpr SmartColor PinkDark(194, 24, 91);
    constexpr SmartColor PinkDarkest(136, 20, 76);

    constexpr SmartColor PurpleLightest(225, 190, 231);
    constexpr SmartColor PurpleLight(186, 104, 200);
    constexpr SmartColor PurpleNormal(156, 39, 176);
    constexpr SmartColor PurpleDark(123, 31, 162);
    constexpr SmartColor PurpleDarkest(74, 20, 140);

    constexpr SmartColor BlueLightest(187, 222, 251);
    constexpr SmartColor BlueLight(144, 202, 249);
    constexpr SmartColor BlueNormal(33, 150, 243);
    constexpr SmartColor BlueDark(25, 118, 210);
    constexpr SmartColor BlueDarkest(13, 71, 161);

    constexpr SmartColor CyanLightest(178, 235, 242);
    constexpr SmartColor CyanLight(77, 208, 225);
    constexpr SmartColor CyanNormal(0, 188, 212);
    constexpr SmartColor CyanDark(0, 151, 167);
    constexpr SmartColor CyanDarkest(0, 96, 100);

    constexpr SmartColor GreenLightest(200, 230, 201);
    constexpr SmartColor GreenLight(165, 214, 167);
    constexpr SmartColor GreenNormal(76, 175, 80);
    constexpr SmartColor GreenDark(56, 142, 60);
    constexpr SmartColor GreenDarkest(27, 94, 32);

    constexpr SmartColor YellowLightest(255, 249, 196);
    constexpr SmartColor YellowLight(255, 241, 118);
    constexpr SmartColor YellowNormal(255, 235, 59);
    constexpr SmartColor YellowDark(251, 192, 45);
    constexpr SmartColor YellowDarkest(245, 127, 23);

    constexpr SmartColor OrangeLightest(255, 224, 178);
    constexpr SmartColor OrangeLight(255, 183, 77);
    constexpr SmartColor OrangeNormal(255, 152, 0);
    constexpr SmartColor OrangeDark(245, 124, 0);
    constexpr SmartColor OrangeDarkest(230, 81, 0);

    constexpr SmartColor BrownLightest(215, 204, 200);
    constexpr SmartColor BrownLight(161, 136, 126);
    constexpr SmartColor BrownNormal(121, 85, 72);
    constexpr SmartColor BrownDark(93, 64, 55);
    constexpr SmartColor BrownDarkest(62, 39, 35);
    
    constexpr SmartColor DebugMagenta(255, 0, 255);

    // --- Dải màu Da (Skin Gradient) ---
    constexpr SmartColor SkinLight(255, 224, 189);
    constexpr SmartColor SkinMid(255, 205, 148);
    constexpr SmartColor SkinShadow(190, 145, 105);

    // --- Dải màu Tóc/Lông mày (Hair Gradient) ---
    constexpr SmartColor HairDarkest(35, 25, 20);
    constexpr SmartColor HairNormal(75, 55, 45);
    constexpr SmartColor HairHighlight(120, 100, 85);
}

// =============================================================
// Theme: Cac mau the hien y nghia (Semantic colors), bao gom Alpha
// =============================================================
namespace Theme {
    // ----- Den mo (Overlay / Shadow) -----
    constexpr SmartColor ShadowLight    = Palette::Black.WithAlpha(30);  // Vien pixel nhat
    constexpr SmartColor ShadowMed      = Palette::Black.WithAlpha(100); // Bong avatar
    constexpr SmartColor ShadowPanel    = Palette::Black.WithAlpha(150); // Menu overlay
    constexpr SmartColor ShadowHeavy    = Palette::Black.WithAlpha(180); // Glassmorphism panel
    constexpr SmartColor ShadowOverlay  = Palette::Black.WithAlpha(200); // Phu man hinh

    // ----- Nen Panel kinh (Glassmorphism) -----
    constexpr SmartColor GlassWhite     = Palette::White.WithAlpha(220); // Panel trang mo
    constexpr SmartColor GlassDark      = SmartColor(200, 15, 20, 30);   // Panel toi (pause)
    constexpr SmartColor GlassGleam     = Palette::White.WithAlpha(150); // Vien trang mo

    // ----- Mau San co (Pitch) -----
    constexpr SmartColor PitchDark      = SmartColor(255, 23, 90, 34);   // Co toi
    constexpr SmartColor PitchLight     = SmartColor(255, 40, 145, 40);  // Co sang
    constexpr SmartColor PitchLine      = Palette::White.WithAlpha(150); // Vach voi
    constexpr SmartColor PitchDot       = Palette::White.WithAlpha(150); // Cham giua san

    // ----- Vien Panel mau chu de -----
    constexpr SmartColor PanelGreenBorder  = SmartColor(200, 34, 139, 34);  
    constexpr SmartColor PanelBlueBorder   = SmartColor(180, 0, 150, 255); 
    constexpr SmartColor PanelOrangeBorder = SmartColor(200, 255, 120, 0); 
    constexpr SmartColor PanelGoldBorder   = SmartColor(200, 255, 215, 0); 
    constexpr SmartColor PanelYellowBorder = SmartColor(255, 255, 200, 0); 

    // ----- Mau cau thu P1 (Cam / Do-Cam) -----
    constexpr SmartColor P1TurnPulse    = SmartColor(30, 255, 150, 0);
    constexpr SmartColor P1TurnBorder   = SmartColor(200, 255, 150, 0);
    constexpr SmartColor P1Watermark    = SmartColor(60, 255, 150, 0);

    // ----- Mau cau thu P2 (Cyan) -----
    constexpr SmartColor P2TurnPulse    = SmartColor(30, 0, 255, 255);
    constexpr SmartColor P2TurnBorder   = SmartColor(200, 0, 255, 255);
    constexpr SmartColor P2Watermark    = SmartColor(60, 0, 255, 255);

    // ----- Mau ban co -----
    constexpr SmartColor LastMoveHighlight = SmartColor(150, 255, 255, 0);
    constexpr SmartColor WinCellFill       = SmartColor(200, 50, 255, 50);
    constexpr SmartColor WinCellBorder     = SmartColor(200, 0, 255, 80);

    // ----- Mau trai bong Pixel -----
    constexpr SmartColor FootballDark   = SmartColor(255, 30, 30, 30);
    constexpr SmartColor FootballLight  = Palette::WhiteSoft;

    // ----- Mau Cup (Trophy) -----
    constexpr SmartColor TrophyRim      = SmartColor(255, 200, 150, 20);
    constexpr SmartColor TrophyBody     = SmartColor(255, 250, 210, 50);
    constexpr SmartColor TrophyShine    = Palette::White.WithAlpha(200);

    // ----- Palette tieu de "CARO" -----
    constexpr SmartColor TitleBorder    = SmartColor(255, 200, 80, 0);
    constexpr SmartColor TitleFill      = SmartColor(255, 255, 220, 0);
    constexpr SmartColor TitleShadow    = Palette::Black.WithAlpha(150);

    // ----- Mau Avatar Pixel -----
    constexpr SmartColor AvaOutline     = SmartColor(255, 30, 30, 30);
    constexpr SmartColor AvaEye         = Palette::Black;
    // P1
    constexpr SmartColor AvaP1Skin      = SmartColor(255, 255, 200, 150);
    constexpr SmartColor AvaP1Shirt     = SmartColor(255, 220, 20, 20);
    constexpr SmartColor AvaP1Accent    = Palette::White;
    // P2
    constexpr SmartColor AvaP2Skin      = SmartColor(255, 255, 220, 180);
    constexpr SmartColor AvaP2Shirt     = SmartColor(255, 30, 100, 255);
    constexpr SmartColor AvaP2Accent    = SmartColor(255, 255, 220, 0);
    // Bot Easy
    constexpr SmartColor AvaBotEasyBody = SmartColor(255, 180, 180, 180);
    constexpr SmartColor AvaBotEasyAccent = SmartColor(255, 200, 100, 50);
    // Bot Medium
    constexpr SmartColor AvaBotMedBody  = SmartColor(255, 120, 200, 120);
    constexpr SmartColor AvaBotMedDark  = SmartColor(255, 50, 150, 50);
    constexpr SmartColor AvaBotMedDarkest = SmartColor(255, 20, 100, 20);
    // Bot Hard
    constexpr SmartColor AvaBotHardBody = SmartColor(255, 80, 40, 40);
    constexpr SmartColor AvaBotHardShirt = SmartColor(255, 250, 30, 30);
    constexpr SmartColor AvaBotHardDark = SmartColor(255, 20,  20, 20);
    // P3 - Tiền Đạo Số 9 (Áo Tím / Hồng)
    constexpr SmartColor AvaP3Skin      = SmartColor(255, 255, 195, 145);
    constexpr SmartColor AvaP3Shirt     = SmartColor(255, 150,  30, 200);
    constexpr SmartColor AvaP3Accent    = SmartColor(255, 255, 120, 220);
    // P4 - Thủ Môn (Áo Vàng / Găng Xanh Neon)
    constexpr SmartColor AvaP4Skin      = SmartColor(255, 245, 200, 140);
    constexpr SmartColor AvaP4Shirt     = SmartColor(255, 230, 200,  10);
    constexpr SmartColor AvaP4Accent    = SmartColor(255,  20, 220, 100);
    // P5 - Hậu Vệ (Áo Bạc / Bạc Hà)
    constexpr SmartColor AvaP5Skin      = SmartColor(255, 255, 215, 160);
    constexpr SmartColor AvaP5Shirt     = SmartColor(255, 180, 220, 215);
    constexpr SmartColor AvaP5Accent    = SmartColor(255,  40, 200, 180);
    // P6 - Đội Trưởng (Áo Đỏ Đậm / Vàng Kim)
    constexpr SmartColor AvaP6Skin      = SmartColor(255, 255, 190, 130);
    constexpr SmartColor AvaP6Shirt     = SmartColor(255, 180,  15,  15);
    constexpr SmartColor AvaP6Accent    = SmartColor(255, 255, 205,   0);
    // Animation: Boot (giày) và Football (bóng)
    constexpr SmartColor AnimBoot       = SmartColor(255,  40,  30,  20);
    constexpr SmartColor AnimBall       = SmartColor(255,  25,  25,  25);

    // ----- Khung nhap ten (Save name box) -----
    constexpr SmartColor SaveBoxBorder  = SmartColor(255, 255, 165, 0);

    // ----- Slot load game -----
    constexpr SmartColor SlotSelected   = SmartColor(150, 255, 180, 50);
    constexpr SmartColor SlotNormal     = SmartColor(100, 200, 200, 200);

    // ----- Thanh truot am luong (Volume Bar) -----
    constexpr SmartColor BarTrack       = SmartColor(255, 200, 200, 200);
    constexpr SmartColor BarFillSelected= SmartColor(255, 255, 120, 0);
    constexpr SmartColor BarFillNormal  = SmartColor(255, 50, 150, 250);

    // ----- Flash khan dai -----
    constexpr SmartColor CameraFlash    = Palette::White; // (alpha dung dong)

    // ----- Banner thang (Victory Banner) -----
    constexpr SmartColor BannerBgBase   = Palette::Black.WithAlpha(220);

    // ----- Pitch board -----
    constexpr SmartColor BoardPitch     = SmartColor(255, 56, 142, 60);
    constexpr SmartColor BoardBorder    = Palette::White;

    // Cập nhật ID cho Cầu thủ Ronaldo (P_CR7)
    constexpr SmartColor CR7_Outline = Palette::Black;        // ID 1
    constexpr SmartColor CR7_SkinL = Palette::SkinLight;    // ID 2
    constexpr SmartColor CR7_SkinM = Palette::SkinMid;      // ID 3
    constexpr SmartColor CR7_SkinD = Palette::SkinShadow;   // ID 4
    constexpr SmartColor CR7_HairD = Palette::HairDarkest;  // ID 5
    constexpr SmartColor CR7_HairN = Palette::HairNormal;   // ID 6
    constexpr SmartColor CR7_Teeth = Palette::White;        // ID 7
    constexpr SmartColor CR7_Eye = Palette::Black;        // ID 8
}

#endif // GAME_COLORS_H