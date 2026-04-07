#include "LoadGameScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../SystemModules/SaveLoadSystem.h" 
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/Localization.h"
#include "../ApplicationTypes/GameConstants.h"

const int BACK_OPTION = 5;
const int TOTAL_LOAD_ITEMS = 6;

bool ProcessLoadGameInput(WPARAM wParam, ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage) {
    bool hasChanged = false;

    if (wParam == 'W' || wParam == VK_UP) {
        selectedOption = (selectedOption - 1 + TOTAL_LOAD_ITEMS) % TOTAL_LOAD_ITEMS;
        statusMessage = L"";
        hasChanged = true;
    }
    else if (wParam == 'S' || wParam == VK_DOWN) {
        selectedOption = (selectedOption + 1) % TOTAL_LOAD_ITEMS;
        statusMessage = L"";
        hasChanged = true;
    }
    else if (wParam == VK_RETURN || wParam == VK_SPACE) {
        if (selectedOption == BACK_OPTION) {
            currentState = SCREEN_MENU;
            statusMessage = L"";
        }
        else {
            // 1. Lấy đúng đường dẫn từ SaveLoadSystem
            std::wstring filename = GetSavePath(selectedOption + 1);

            // 2. Kiểm tra file tồn tại trước
            if (!CheckSaveExists(selectedOption + 1)) {
                statusMessage = L"Lỗi: Slot này chưa có dữ liệu!";
                PlaySFX(L"Asset/audio/error.wav");
            }
            else {
                // 3. Thực hiện Load
                if (LoadMatchData(playState, filename.c_str())) {
                    PlaySFX(L"Asset/audio/success.wav");

                    // CẬP NHẬT TRẠNG THÁI TRẬN ĐẤU (Rất quan trọng)
                    playState->status = MATCH_PLAYING;

                    currentState = SCREEN_PLAY;
                    statusMessage = L"";
                }
                else {
                    statusMessage = L"Lỗi: File lưu bị hỏng hoặc phiên bản cũ. Hãy chơi và lưu lại!";
                    PlaySFX(L"Asset/audio/error.wav");
                }
            }
        }
        hasChanged = true;
    }
    else if (wParam == VK_ESCAPE) {
        currentState = SCREEN_MENU;
        hasChanged = true;
    }

    return hasChanged;
}

void RenderLoadGameScreen(HDC hdc, int selectedOption, const std::wstring& statusMessage, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    // 1. Vẽ nền Procedural Stadium
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // 2. Bảng Kính (White Glassmorphism)
    Gdiplus::SolidBrush whitePanel(GdipColour::GLASS_WHITE);
    int panelW = 550;
    int panelH = 550;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2;

    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(GdipColour::PANEL_BLUE_BORDER, 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // 3. Tiêu đề
    int titleY = panelY + 40;
    DrawTextCentered(hdc, L"--- LỊCH SỬ THI ĐẤU ---", titleY, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

    // 4. Vẽ các Slot Save Game
    int startY = panelY + 120;
    int spacing = 55;

    for (int i = 0; i < TOTAL_LOAD_ITEMS; i++) {
        std::wstring itemText;
        COLORREF color = Colour::GRAY_DARKEST;
        HFONT font = GlobalFont::Bold;
        int yPos = startY + i * spacing;

        if (i == BACK_OPTION) {
            itemText = L"== [ TRỞ VỀ KHÁN ĐÀI ] ==";
            if (i == selectedOption) {
                int gCol = (int)(150 + sin(g_GlobalAnimTime * 15.0f) * 105);
                color = RGB(max(0, min(255, 255 - gCol)), 100, 255);
            }
            DrawTextCentered(hdc, itemText, yPos + 20, screenWidth, color, (i == selectedOption ? GlobalFont::Title : GlobalFont::Bold));
        }
        else {
            bool exists = CheckSaveExists(i + 1);
            itemText = L"Hồ Sơ Trận Đấu " + std::to_wstring(i + 1) + (exists ? L"  [ BĂNG GHI HÌNH ]" : L"  [ BĂNG TRỐNG ]");

            int slotWidth = 400;
            int slotX = panelX + (panelW - slotWidth) / 2;
            
            // Vẽ hộp Slot (Glass button giả)
            if (i == selectedOption) {
                int aCol = (int)(150 + sin(g_GlobalAnimTime * 10.0f) * 80);
                Gdiplus::SolidBrush slotBrush(GdipColour::WithAlpha(GdipColour::SLOT_SELECTED, (BYTE)aCol));
                g.FillRectangle(&slotBrush, slotX, yPos, slotWidth, 40);
                color = Colour::WHITE;
                itemText = L">> " + itemText + L" <<";
            } else {
                Gdiplus::SolidBrush slotBrush(GdipColour::SLOT_NORMAL);
                g.FillRectangle(&slotBrush, slotX, yPos, slotWidth, 40);
                color = exists ? Colour::GRAY_DARKEST : Colour::GRAY_NORMAL;
            }

            // Gọi DrawText W
            SetTextColor(hdc, color);
            HFONT oldFont = (HFONT)SelectObject(hdc, font);
            SetBkMode(hdc, TRANSPARENT);
            RECT textRect = { slotX, yPos, slotX + slotWidth, yPos + 40 };
            DrawTextW(hdc, itemText.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(hdc, oldFont);
        }
    }

    // Thông báo lỗi
    if (!statusMessage.empty()) {
        DrawTextCentered(hdc, statusMessage, startY + TOTAL_LOAD_ITEMS * spacing + 10, screenWidth, Colour::RED_NORMAL, GlobalFont::Bold);
    }

    DrawTextCentered(hdc, L"W/S: Chọn Hồ Sơ | ENTER: Chạy băng | ESC: Quay lại", screenHeight - 60, screenWidth, Colour::WHITE, GlobalFont::Note);
}

void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage, WPARAM wParam) {
    // Bỏ qua nếu không có sự kiện phím (wParam = 0)
    if (wParam == 0) return;

    // Ủy quyền xử lý logic cho ProcessLoadGameInput
    ProcessLoadGameInput(wParam, currentState, playState, selectedOption, statusMessage);
}