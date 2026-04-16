#include "LoadGameScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/SaveLoadSystem.h" 
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/Localization.h"
#include "../ApplicationTypes/GameConstants.h"

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>

// Các trạng thái của màn hình Load Game
enum LoadScreenMode {
    MODE_SELECT_SLOT,    // Đang chọn slot ở bảng bên trái
    MODE_SELECT_ACTION,  // Đang chọn hành động (Tải, Đổi tên, Xóa) ở bảng bên phải
    MODE_EDIT_NAME       // Đang nhập tên mới cho slot (Đổi tên)
};

static LoadScreenMode g_CurrentMode = MODE_SELECT_SLOT;
static int g_SelectedSlot = 0;   // 0 to 4 (5 slots)
static int g_SelectedAction = 0; // 0: Tải, 1: Đổi tên, 2: Xóa
static std::wstring g_EditNameBuffer = L"";
static float g_LoadFeedbackTimer = 0.0f;
static std::wstring g_LoadStatusMsg = L"";

const int MAX_SLOTS = 5;
const int MAX_ACTIONS = 4;
const int BACK_BTN_INDEX = 5; // Nút quay lại nằm dưới cùng danh sách slot

bool ProcessLoadGameInput(WPARAM wParam, ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage) {
    bool hasChanged = true;
    bool isChar = (wParam & 0x10000);
    wchar_t ch = (wchar_t)(wParam & 0xFFFF);

    if (g_CurrentMode == MODE_EDIT_NAME) {
        if (isChar) {
            if (g_EditNameBuffer.length() < 15 && ch >= 32) {
                g_EditNameBuffer += ch;
            }
        } else {
            if (wParam == VK_BACK) {
                if (!g_EditNameBuffer.empty()) g_EditNameBuffer.pop_back();
            } else if (wParam == VK_ESCAPE) {
                g_CurrentMode = MODE_SELECT_ACTION;
            } else if (wParam == VK_RETURN) {
                if (!g_EditNameBuffer.empty()) {
                    if (RenameSave(g_SelectedSlot + 1, g_EditNameBuffer)) {
                        PlaySFX(L"Asset/audio/success.wav");
                        g_LoadStatusMsg = L"Đã đổi tên thành công!";
                        g_LoadFeedbackTimer = 1.0f;
                        g_CurrentMode = MODE_SELECT_ACTION;
                    }
                }
            }
        }
        return true;
    }

    if (isChar) return false;

    if (g_CurrentMode == MODE_SELECT_SLOT) {
        if (wParam == 'W' || wParam == VK_UP) {
            g_SelectedSlot = (g_SelectedSlot - 1 + (MAX_SLOTS + 1)) % (MAX_SLOTS + 1);
        } else if (wParam == 'S' || wParam == VK_DOWN) {
            g_SelectedSlot = (g_SelectedSlot + 1) % (MAX_SLOTS + 1);
        } else if (wParam == VK_RETURN || wParam == VK_SPACE) {
            if (g_SelectedSlot == BACK_BTN_INDEX) {
                currentState = SCREEN_MENU;
                // Reset state
                g_CurrentMode = MODE_SELECT_SLOT;
                g_SelectedSlot = 0;
            } else {
                // Nếu slot có dữ liệu thì cho phép chọn hành động
                if (CheckSaveExists(g_SelectedSlot + 1)) {
                    g_CurrentMode = MODE_SELECT_ACTION;
                    g_SelectedAction = 0;
                } else {
                    g_LoadStatusMsg = L"Slot này chưa có dữ liệu!";
                    g_LoadFeedbackTimer = 1.0f;
                }
            }
        } else if (wParam == VK_ESCAPE) {
            currentState = SCREEN_MENU;
        }
    } else if (g_CurrentMode == MODE_SELECT_ACTION) {
        if (wParam == 'W' || wParam == VK_UP) {
            g_SelectedAction = (g_SelectedAction - 1 + MAX_ACTIONS) % MAX_ACTIONS;
        } else if (wParam == 'S' || wParam == VK_DOWN) {
            g_SelectedAction = (g_SelectedAction + 1) % MAX_ACTIONS;
        } else if (wParam == VK_RETURN || wParam == VK_SPACE) {
            if (g_SelectedAction == 0) { // Tải
                if (LoadMatchData(playState, GetSavePath(g_SelectedSlot + 1))) {
                    PlaySFX(L"Asset/audio/success.wav");
                    playState->status = MATCH_PLAYING;
                    currentState = SCREEN_PLAY;
                    g_CurrentMode = MODE_SELECT_SLOT;
                }
            } else if (g_SelectedAction == 1) { // Đổi tên
                g_EditNameBuffer = GetSaveDisplayName(g_SelectedSlot + 1);
                g_CurrentMode = MODE_EDIT_NAME;
            } else if (g_SelectedAction == 2) { // Xóa
                if (DeleteSave(g_SelectedSlot + 1)) {
                    PlaySFX(L"Asset/audio/success.wav");
                    g_LoadStatusMsg = L"Đã xóa bản lưu!";
                    g_LoadFeedbackTimer = 1.0f;
                    g_CurrentMode = MODE_SELECT_SLOT;
                }
            } else if (g_SelectedAction == 3) { // Quay lại
                g_CurrentMode = MODE_SELECT_SLOT;
            }
        } else if (wParam == VK_ESCAPE) {
            g_CurrentMode = MODE_SELECT_SLOT;
        }
    }

    selectedOption = g_SelectedSlot;
    return hasChanged;
}

void RenderLoadGameScreen(HDC hdc, int selectedOption, const std::wstring& statusMessage, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Kích thước bảng chính (Nới rộng thêm 5% nữa theo yêu cầu)
    int panelW = UIScaler::SX(900);
    int panelH = UIScaler::SY(540);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2;

    // Vẽ nền Panel
    Gdiplus::SolidBrush bgBrush(ToGdiColor(Theme::GlassWhite));
    g.FillRectangle(&bgBrush, panelX, panelY, panelW, panelH);
    Gdiplus::Pen borderPen(ToGdiColor(Theme::PanelBlueBorder), 3.0f);
    g.DrawRectangle(&borderPen, panelX, panelY, panelW, panelH);

    // Banner Tiêu đề
    DrawPixelBanner(g, hdc, L"QUẢN LÝ BẢN LƯU", screenWidth / 2, panelY + UIScaler::SY(45), panelW - UIScaler::SX(40),
        ToCOLORREF(Palette::White), RGB(0, 150, 255), "Asset/models/bg/cassette.txt");

    // Tính toán lại tọa độ cho 3 Cột (Nới rộng thêm 5%)
    // Cột 1 (Slots): 210px
    // Cột 2 (Info): 330px (+~45px so với bản trước)
    // Cột 3 (Rename): 280px
    
    int col1X = panelX + UIScaler::SX(30);
    int col1W = UIScaler::SX(210);
    int col2X = col1X + col1W + UIScaler::SX(20);
    int col2W = UIScaler::SX(330);
    int col3X = col2X + col2W + UIScaler::SX(25);
    int col3W = UIScaler::SX(270);

    int startY = panelY + UIScaler::SY(110);
    int slotH = UIScaler::SY(50);
    int spacing = UIScaler::SY(8);

    // Vẽ danh sách slot bên trái
    for (int i = 0; i < MAX_SLOTS + 1; i++) {
        int yPos = startY + i * (slotH + spacing);
        
        if (i == BACK_BTN_INDEX) {
            std::wstring backText = L" [ TRỞ VỀ ] ";
            bool isSel = (g_CurrentMode == MODE_SELECT_SLOT && g_SelectedSlot == i);
            COLORREF color = isSel ? ToCOLORREF(Palette::OrangeNormal) : ToCOLORREF(Palette::BlueDarkest);
            HFONT font = isSel ? GlobalFont::Bold : GlobalFont::Default;
            
            RECT r = { col1X, yPos, col1X + col1W, yPos + slotH };
            SetTextColor(hdc, color);
            HFONT oldF = (HFONT)SelectObject(hdc, font);
            DrawTextW(hdc, backText.c_str(), -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, oldF);
            continue;
        }

        std::wstring displayName = GetSaveDisplayName(i + 1);
        bool exists = CheckSaveExists(i + 1);
        std::wstring slotText = exists ? displayName : L"Trống";
        if (slotText.length() > 12) slotText = slotText.substr(0, 10) + L"...";

        bool isSelected = (g_SelectedSlot == i);
        bool isActive = (g_CurrentMode == MODE_SELECT_SLOT && isSelected);

        Gdiplus::Color boxColor = ToGdiColor(Theme::SlotNormal);
        if (isSelected) {
            boxColor = ToGdiColor(WithAlpha(Theme::SlotSelected, isActive ? (BYTE)220 : (BYTE)120));
        }
        Gdiplus::SolidBrush slotBrush(boxColor);
        g.FillRectangle(&slotBrush, col1X, yPos, col1W, slotH);

        if (isActive) {
            Gdiplus::Pen selPen(ToGdiColor(Theme::PanelYellowBorder), 2.0f);
            g.DrawRectangle(&selPen, col1X, yPos, col1W, slotH);
        }

        COLORREF textColor = exists ? ToCOLORREF(Palette::GrayDarkest) : ToCOLORREF(Palette::GrayNormal);
        if (isSelected) textColor = ToCOLORREF(Palette::White);
        
        std::wstring fullSlotText = L"Slot " + std::to_wstring(i + 1) + L": " + slotText;
        RECT tr = { col1X + UIScaler::SX(8), yPos, col1X + col1W - UIScaler::SX(5), yPos + slotH };
        SetTextColor(hdc, textColor);
        HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Default);
        DrawTextW(hdc, fullSlotText.c_str(), -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldF);
    }

    // Vẽ bảng hành động và chi tiết (Cột giữa)
    if (g_SelectedSlot != BACK_BTN_INDEX) {
        SaveMetadata meta = GetSaveMetadata(g_SelectedSlot + 1);
        
        if (meta.exists) {
            // 1. Tiêu đề mục chi tiết
            RECT rDetailHeader = { col2X, startY, col2X + col2W, startY + UIScaler::SY(25) };
            SetTextColor(hdc, ToCOLORREF(Palette::BlueDarkest));
            HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Bold);
            DrawTextW(hdc, L"THÔNG TIN CHI TIẾT", -1, &rDetailHeader, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // 2. Khung chứa thông tin (Làm to hơn theo yêu cầu)
            int infoH = UIScaler::SY(160);
            Gdiplus::SolidBrush infoBg(ToGdiColor(WithAlpha(Palette::CyanLight, 30)));
            g.FillRectangle(&infoBg, col2X + UIScaler::SX(5), startY + UIScaler::SY(30), col2W - UIScaler::SX(10), infoH);
            Gdiplus::Pen infoBorder(ToGdiColor(WithAlpha(Palette::CyanNormal, 100)), 1.0f);
            g.DrawRectangle(&infoBorder, col2X + UIScaler::SX(5), startY + UIScaler::SY(30), col2W - UIScaler::SX(10), infoH);

            // 3. Hiển thị Metadata
            int textY = startY + UIScaler::SY(38);
            int lineH = UIScaler::SY(28); // Tăng khoảng cách dòng
            
            auto DrawMetaLine = [&](const std::wstring& label, const std::wstring& val, COLORREF valCol) {
                // Vẽ Label (Font Note 28px)
                SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
                SelectObject(hdc, GlobalFont::Note);
                // Offset +4px để căn giữa dòng với font Default (36px)
                TextOutW(hdc, col2X + UIScaler::SX(15), textY + UIScaler::SY(4), label.c_str(), (int)label.length());
                
                // Vẽ Value (Font Default 36px - Vừa phải, không quá to như Bold)
                SetTextColor(hdc, valCol);
                SelectObject(hdc, GlobalFont::Default);
                TextOutW(hdc, col2X + UIScaler::SX(115), textY, val.c_str(), (int)val.length());
                textY += lineH;
            };

            DrawMetaLine(L"Tên lưu:", meta.name, ToCOLORREF(Palette::OrangeNormal));
            DrawMetaLine(L"Thời gian:", meta.timestamp, ToCOLORREF(Palette::BlueDarkest));
            
            std::wstring modeStr = (meta.mode == 0) ? L"Caro 15x15" : L"TicTacToe 3x3";
            DrawMetaLine(L"Chế độ:", modeStr, ToCOLORREF(Palette::GreenNormal));

            std::wstring typeStr = (meta.type == 0) ? L"PvP (Người)" : L"PvE (Máy)";
            DrawMetaLine(L"Đối thủ:", typeStr, ToCOLORREF(Palette::CyanNormal));

            std::wstring scoreStr = std::to_wstring(meta.p1Wins) + L" - " + std::to_wstring(meta.p2Wins);
            DrawMetaLine(L"Tỷ số:", scoreStr, ToCOLORREF(Palette::RedNormal));

            // 4. Các nút hành động (Dời xuống dưới khung thông tin mới)
            const wchar_t* actions[] = { L"TẢI GAME", L"ĐỔI TÊN", L"XÓA", L"BỎ CHỌN" };
            int actionStartY = startY + UIScaler::SY(205);
            
            for (int j = 0; j < MAX_ACTIONS; j++) {
                int actY = actionStartY + j * UIScaler::SY(42);
                bool actSel = (g_CurrentMode == MODE_SELECT_ACTION && g_SelectedAction == j);
                
                Gdiplus::SolidBrush actBrush(actSel ? ToGdiColor(Theme::SlotSelected) : ToGdiColor(Theme::SlotNormal));
                g.FillRectangle(&actBrush, col2X + UIScaler::SX(15), actY, col2W - UIScaler::SX(30), UIScaler::SY(38));
                
                if (actSel) {
                    Gdiplus::Pen actPen(ToGdiColor(Theme::PanelYellowBorder), 2.0f);
                    g.DrawRectangle(&actPen, col2X + UIScaler::SX(15), actY, col2W - UIScaler::SX(30), UIScaler::SY(38));
                }

                COLORREF actColor = actSel ? ToCOLORREF(Palette::White) : ToCOLORREF(Palette::GrayDarkest);
                RECT rAct = { col2X + UIScaler::SX(15), actY, col2X + col2W - UIScaler::SX(15), actY + UIScaler::SY(38) };
                SetTextColor(hdc, actColor);
                SelectObject(hdc, GlobalFont::Bold);
                DrawTextW(hdc, actions[j], -1, &rAct, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
            SelectObject(hdc, oldF);

            // Cột 3: Khung Đổi tên (Cột Phải)
            if (g_CurrentMode == MODE_EDIT_NAME) {
                Gdiplus::SolidBrush renameBg(ToGdiColor(Theme::GlassWhite));
                g.FillRectangle(&renameBg, col3X, startY, col3W, UIScaler::SY(180));
                Gdiplus::Pen renameBorder(ToGdiColor(Theme::PanelYellowBorder), 2.0f);
                g.DrawRectangle(&renameBorder, col3X, startY, col3W, UIScaler::SY(180));

                RECT rEditTitle = { col3X, startY + UIScaler::SY(10), col3X + col3W, startY + UIScaler::SY(40) };
                SetTextColor(hdc, ToCOLORREF(Palette::OrangeNormal));
                SelectObject(hdc, GlobalFont::Bold);
                DrawTextW(hdc, L"ĐỔI TÊN SLOT", -1, &rEditTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                int boxH = UIScaler::SY(45);
                int boxY = startY + UIScaler::SY(60);
                Gdiplus::SolidBrush whiteBox(Gdiplus::Color(255, 255, 255, 255));
                g.FillRectangle(&whiteBox, col3X + UIScaler::SX(10), boxY, col3W - UIScaler::SX(20), boxH);
                Gdiplus::Pen boxPen(ToGdiColor(Palette::GrayNormal), 1.0f);
                g.DrawRectangle(&boxPen, col3X + UIScaler::SX(10), boxY, col3W - UIScaler::SX(20), boxH);

                extern float g_GlobalAnimTime;
                bool showCursor = ((int)(g_GlobalAnimTime * 2.5f) % 2 == 0);
                std::wstring displayBuffer = g_EditNameBuffer + (showCursor ? L"_" : L" ");
                
                RECT rEditBuffer = { col3X + UIScaler::SX(10), boxY, col3X + col3W - UIScaler::SX(10), boxY + boxH + UIScaler::SY(10) };
                SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
                DrawTextW(hdc, displayBuffer.c_str(), -1, &rEditBuffer, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

                RECT rEditHint = { col3X, startY + UIScaler::SY(140), col3X + col3W, startY + UIScaler::SY(175) };
                SetTextColor(hdc, ToCOLORREF(Palette::GrayDark));
                SelectObject(hdc, GlobalFont::Note);
                DrawTextW(hdc, L"[Enter] Lưu | [Esc] Hủy", -1, &rEditHint, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }
        } else {
            RECT rEmpty = { col2X, startY + UIScaler::SY(80), col2X + col2W, startY + UIScaler::SY(200) };
            SetTextColor(hdc, ToCOLORREF(Palette::GrayNormal));
            HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Bold);
            DrawTextW(hdc, L"SLOT TRỐNG", -1, &rEmpty, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, oldF);
        }
    }

    // Hiển thị status message
    if (!g_LoadStatusMsg.empty()) {
        int statusY = panelY + panelH - UIScaler::SY(60);
        DrawTextCentered(hdc, g_LoadStatusMsg, statusY, screenWidth, ToCOLORREF(Palette::RedNormal), GlobalFont::Bold);
    }

    // Gợi ý phím
    std::wstring hints = L"W/S: Di chuyển  |  ENTER: Xác nhận  |  ESC: Quay lại";
    DrawTextCentered(hdc, hints, screenHeight - UIScaler::SY(40), screenWidth, ToCOLORREF(Palette::White), GlobalFont::Note);
}

void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage, WPARAM wParam) {
    // Xử lý timer phản hồi
    extern float g_GlobalAnimTime;
    static double lastTime = 0;
    double dt = 0.016; // Giả định 60fps nếu không có dt thực
    
    if (g_LoadFeedbackTimer > 0) {
        g_LoadFeedbackTimer -= (float)dt;
        if (g_LoadFeedbackTimer <= 0) {
            g_LoadFeedbackTimer = 0;
            g_LoadStatusMsg = L"";
        }
    }

    if (wParam == 0) return;
    ProcessLoadGameInput(wParam, currentState, playState, selectedOption, statusMessage);
}