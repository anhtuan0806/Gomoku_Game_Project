#ifndef GAME_COLORS_H
#define GAME_COLORS_H
#include <windows.h>
#include <gdiplus.h>

namespace Colour {
    constexpr COLORREF GRAY_LIGHTEST = RGB(245, 245, 245);
    constexpr COLORREF GRAY_LIGHT = RGB(224, 224, 224);
    constexpr COLORREF GRAY_NORMAL = RGB(158, 158, 158);
    constexpr COLORREF GRAY_DARK = RGB(97, 97, 97);
    constexpr COLORREF GRAY_DARKEST = RGB(33, 33, 33);

    constexpr COLORREF RED_LIGHTEST = RGB(255, 205, 210);
    constexpr COLORREF RED_LIGHT = RGB(239, 154, 154);
    constexpr COLORREF RED_NORMAL = RGB(244, 67, 54);
    constexpr COLORREF RED_DARK = RGB(211, 47, 47);
    constexpr COLORREF RED_DARKEST = RGB(183, 28, 28);

    constexpr COLORREF PINK_LIGHTEST = RGB(248, 187, 208);
    constexpr COLORREF PINK_LIGHT = RGB(240, 98, 146);
    constexpr COLORREF PINK_NORMAL = RGB(233, 30, 99);
    constexpr COLORREF PINK_DARK = RGB(194, 24, 91);
    constexpr COLORREF PINK_DARKEST = RGB(136, 20, 76);

    constexpr COLORREF PURPLE_LIGHTEST = RGB(225, 190, 231);
    constexpr COLORREF PURPLE_LIGHT = RGB(186, 104, 200);
    constexpr COLORREF PURPLE_NORMAL = RGB(156, 39, 176);
    constexpr COLORREF PURPLE_DARK = RGB(123, 31, 162);
    constexpr COLORREF PURPLE_DARKEST = RGB(74, 20, 140);

    constexpr COLORREF BLUE_LIGHTEST = RGB(187, 222, 251);
    constexpr COLORREF BLUE_LIGHT = RGB(144, 202, 249);
    constexpr COLORREF BLUE_NORMAL = RGB(33, 150, 243);
    constexpr COLORREF BLUE_DARK = RGB(25, 118, 210);
    constexpr COLORREF BLUE_DARKEST = RGB(13, 71, 161);

    constexpr COLORREF CYAN_LIGHTEST = RGB(178, 235, 242);
    constexpr COLORREF CYAN_LIGHT = RGB(77, 208, 225);
    constexpr COLORREF CYAN_NORMAL = RGB(0, 188, 212);
    constexpr COLORREF CYAN_DARK = RGB(0, 151, 167);
    constexpr COLORREF CYAN_DARKEST = RGB(0, 96, 100);

    constexpr COLORREF GREEN_LIGHTEST = RGB(200, 230, 201);
    constexpr COLORREF GREEN_LIGHT = RGB(165, 214, 167);
    constexpr COLORREF GREEN_NORMAL = RGB(76, 175, 80);
    constexpr COLORREF GREEN_DARK = RGB(56, 142, 60);
    constexpr COLORREF GREEN_DARKEST = RGB(27, 94, 32);

    constexpr COLORREF YELLOW_LIGHTEST = RGB(255, 249, 196);
    constexpr COLORREF YELLOW_LIGHT = RGB(255, 241, 118);
    constexpr COLORREF YELLOW_NORMAL = RGB(255, 235, 59);
    constexpr COLORREF YELLOW_DARK = RGB(251, 192, 45);
    constexpr COLORREF YELLOW_DARKEST = RGB(245, 127, 23);

    constexpr COLORREF ORANGE_LIGHTEST = RGB(255, 224, 178);
    constexpr COLORREF ORANGE_LIGHT = RGB(255, 183, 77);
    constexpr COLORREF ORANGE_NORMAL = RGB(255, 152, 0);
    constexpr COLORREF ORANGE_DARK = RGB(245, 124, 0);
    constexpr COLORREF ORANGE_DARKEST = RGB(230, 81, 0);

    constexpr COLORREF BROWN_LIGHTEST = RGB(215, 204, 200);
    constexpr COLORREF BROWN_LIGHT = RGB(161, 136, 126);
    constexpr COLORREF BROWN_NORMAL = RGB(121, 85, 72);
    constexpr COLORREF BROWN_DARK = RGB(93, 64, 55);
    constexpr COLORREF BROWN_DARKEST = RGB(62, 39, 35);

    constexpr COLORREF BLACK = RGB(0, 0, 0);
    constexpr COLORREF WHITE = RGB(255, 255, 255);
}

// =============================================================
// GdipColour: Mau Gdiplus duoc dat ten san (A, R, G, B)
// Dung thay cho Gdiplus::Color(a, r, g, b) viet tay trong code
// =============================================================
namespace GdipColour {

    // ----- Mau trong suot -----
    const Gdiplus::Color _TRANSPARENT            (0,   0,   0,   0  );

    // ----- Den / Trang -----
    const Gdiplus::Color BLACK                  (255, 0,   0,   0  );
    const Gdiplus::Color WHITE                  (255, 255, 255, 255);
    const Gdiplus::Color WHITE_SOFT             (255, 240, 240, 240);

    // ----- Den mo (Overlay / Shadow) -----
    const Gdiplus::Color SHADOW_LIGHT           (30,  0,   0,   0  ); // Vien pixel nhat
    const Gdiplus::Color SHADOW_MED             (100, 0,   0,   0  ); // Bong avatar
    const Gdiplus::Color SHADOW_PANEL           (150, 0,   0,   0  ); // Menu overlay
    const Gdiplus::Color SHADOW_HEAVY           (180, 0,   0,   0  ); // Glassmorphism panel
    const Gdiplus::Color SHADOW_OVERLAY         (200, 0,   0,   0  ); // Phu man hinh

    // ----- Nen Panel kinh (Glassmorphism) -----
    const Gdiplus::Color GLASS_WHITE            (220, 255, 255, 255); // Panel trang mo
    const Gdiplus::Color GLASS_DARK             (200, 15,  20,  30 ); // Panel toi (pause)
    const Gdiplus::Color GLASS_GLEAM            (150, 255, 255, 255); // Vien trang mo

    // ----- Mau San co (Pitch) -----
    const Gdiplus::Color PITCH_DARK             (255, 23,  90,  34 ); // Co toi
    const Gdiplus::Color PITCH_LIGHT            (255, 40,  145, 40 ); // Co sang
    const Gdiplus::Color PITCH_LINE             (150, 255, 255, 255); // Vach voi
    const Gdiplus::Color PITCH_DOT              (150, 255, 255, 255); // Cham giua san

    // ----- Vien Panel mau chu de -----
    const Gdiplus::Color PANEL_GREEN_BORDER     (200, 34,  139, 34 ); // Vien xanh la
    const Gdiplus::Color PANEL_BLUE_BORDER      (180, 0,   150, 255); // Vien xanh lam

    // ----- Mau cau thu P1 (Cam / Do-Cam) -----
    const Gdiplus::Color P1_TURN_PULSE          (30,  255, 150, 0  ); // Nhap nhay luot P1
    const Gdiplus::Color P1_TURN_BORDER         (200, 255, 150, 0  ); // Vien khi toi luot P1
    const Gdiplus::Color P1_WATERMARK           (60,  255, 150, 0  ); // Chu cheo nen P1

    // ----- Mau cau thu P2 (Cyan) -----
    const Gdiplus::Color P2_TURN_PULSE          (30,  0,   255, 255); // Nhap nhay luot P2
    const Gdiplus::Color P2_TURN_BORDER         (200, 0,   255, 255); // Vien khi toi luot P2
    const Gdiplus::Color P2_WATERMARK           (60,  0,   255, 255); // Chu cheo nen P2

    // ----- Mau ban co -----
    const Gdiplus::Color LAST_MOVE_HIGHLIGHT    (150, 255, 255, 0  ); // O vua danh (vang nhat)
    const Gdiplus::Color WIN_CELL_FILL          (200, 50,  255, 50 ); // O thang (xanh la)
    const Gdiplus::Color WIN_CELL_BORDER        (200, 0,   255, 80 ); // Vien o thang

    // ----- Mau trai bong Pixel -----
    const Gdiplus::Color FOOTBALL_DARK          (255, 30,  30,  30 ); // Mang den bong
    const Gdiplus::Color FOOTBALL_LIGHT         (255, 240, 240, 240); // Mang trang bong

    // ----- Mau Cup (Trophy) -----
    const Gdiplus::Color TROPHY_RIM             (255, 200, 150, 20 ); // Vien vang dong
    const Gdiplus::Color TROPHY_BODY            (255, 250, 210, 50 ); // Than vang sang
    const Gdiplus::Color TROPHY_SHINE           (255, 255, 255, 200); // Anh sang cup

    // ----- Palette tieu de "CARO" -----
    const Gdiplus::Color TITLE_BORDER           (255, 200, 80,  0  ); // Vien cam dam
    const Gdiplus::Color TITLE_FILL             (255, 255, 220, 0  ); // Loi vang choi
    const Gdiplus::Color TITLE_SHADOW           (150, 0,   0,   0  ); // Bong do 3D

    // ----- Mau Avatar Pixel -----
    const Gdiplus::Color AVA_OUTLINE            (255, 30,  30,  30 ); // Vien toc (code 1)
    const Gdiplus::Color AVA_EYE                (255, 0,   0,   0  ); // Mat (code 5)
    // P1 - Do Trang
    const Gdiplus::Color AVA_P1_SKIN            (255, 255, 200, 150);
    const Gdiplus::Color AVA_P1_SHIRT           (255, 220, 20,  20 );
    const Gdiplus::Color AVA_P1_ACCENT          (255, 255, 255, 255);
    // P2 - Lam Vang
    const Gdiplus::Color AVA_P2_SKIN            (255, 255, 220, 180);
    const Gdiplus::Color AVA_P2_SHIRT           (255, 30,  100, 255);
    const Gdiplus::Color AVA_P2_ACCENT          (255, 255, 220, 0  );
    // Bot Easy - Thep / Dong
    const Gdiplus::Color AVA_BOT_EASY_BODY      (255, 180, 180, 180);
    const Gdiplus::Color AVA_BOT_EASY_ACCENT    (255, 200, 100, 50 );
    // Bot Medium - Luc
    const Gdiplus::Color AVA_BOT_MED_BODY       (255, 120, 200, 120);
    const Gdiplus::Color AVA_BOT_MED_DARK       (255, 50,  150, 50 );
    const Gdiplus::Color AVA_BOT_MED_DARKEST    (255, 20,  100, 20 );
    // Bot Hard - Den Do
    const Gdiplus::Color AVA_BOT_HARD_BODY      (255, 80,  40,  40 );
    const Gdiplus::Color AVA_BOT_HARD_SHIRT     (255, 250, 30,  30 );
    const Gdiplus::Color AVA_BOT_HARD_DARK      (255, 20,  20,  20 );
    // Mau loi (debug)
    const Gdiplus::Color DEBUG_MAGENTA          (255, 255, 0,   255);

    // ----- Khung nhap ten (Save name box) -----
    const Gdiplus::Color SAVE_BOX_BORDER        (255, 255, 165, 0  ); // Cam vang

    // ----- Slot load game -----
    const Gdiplus::Color SLOT_SELECTED          (150, 255, 180, 50 ); // Slot dang chon (alpha co dinh)
    const Gdiplus::Color SLOT_NORMAL            (100, 200, 200, 200); // Slot binh thuong

    // ----- Thanh truot am luong (Volume Bar) -----
    const Gdiplus::Color BAR_TRACK              (255, 200, 200, 200); // Nen thanh truot xam sang
    const Gdiplus::Color BAR_FILL_SELECTED      (255, 255, 120, 0  ); // Doan da to -- muc chon -- Cam
    const Gdiplus::Color BAR_FILL_NORMAL        (255, 50,  150, 250); // Doan da to -- khong chon -- Xanh lam

    // ----- Flash khan dai -----
    const Gdiplus::Color CAMERA_FLASH           (255, 255, 255, 255); // Tia chop trang (alpha dung BYTE dong)

    // ----- Banner thang (Victory Banner) -----
    const Gdiplus::Color BANNER_BG_BASE         (220, 0,   0,   0  ); // Nen glassmorphism banner (alpha co dinh)

    // ----- Pitch board -----
    const Gdiplus::Color BOARD_PITCH            (255, 56,  142, 60 ); // Xanh san co muot
    const Gdiplus::Color BOARD_BORDER           (255, 255, 255, 255); // Vien trang
}

// =============================================================
// GdipColour::WithAlpha - Lay mau tu GdipColour nhung doi alpha
// Dung cho hieu ung animation alpha dong (pulse, fade)
// =============================================================
namespace GdipColour {
    inline Gdiplus::Color WithAlpha(const Gdiplus::Color& base, BYTE alpha) {
        return Gdiplus::Color(alpha, base.GetR(), base.GetG(), base.GetB());
    }
}

#endif // GAME_COLORS_H