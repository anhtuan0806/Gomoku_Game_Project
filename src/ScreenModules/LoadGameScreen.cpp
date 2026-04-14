#include "LoadGameScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
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

    // 2. Bảng Kính (White Glassmorphism) - rộng và cao hơn để chứa đủ nội dung
    int panelW = UIScaler::SX(640);
    int panelH = UIScaler::SY(620);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2;

    Gdiplus::SolidBrush whitePanel(Theme::GlassWhite);
    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(Theme::PanelBlueBorder, 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // 3. Tiêu đề Pixel Banner (Dấu ấn riêng: Băng ghi hình)
    int bannerCX = screenWidth / 2;
    int bannerCY = panelY + UIScaler::SY(40);
    DrawPixelBanner(g, hdc, L"LỊCH SỬ THI ĐẤU", bannerCX, bannerCY, panelW - UIScaler::SX(20),
        Palette::White, RGB(0, 180, 255), "Asset/models/cassette.txt");

    // 4. Vẽ các Slot Save Game
    int slotW    = UIScaler::SX(500);                           // đủ rộng cho text dài
    int slotH    = UIScaler::SY(52);                            // đủ cao để chữ không bị cắt
    int slotX    = panelX + (panelW - slotW) / 2;
    int startY   = panelY + UIScaler::SY(115);
    int spacing  = slotH + UIScaler::SY(10);                    // khoảng cách giữa các slot

    SetBkMode(hdc, TRANSPARENT);

    for (int i = 0; i < TOTAL_LOAD_ITEMS; i++) {
        std::wstring itemText;
        COLORREF color = Palette::GrayDarkest;
        HFONT font = GlobalFont::Bold;
        int yPos = startY + i * spacing;

        if (i == BACK_OPTION) {
            // Nút Quay Lại — căn giữa toàn bộ panel
            itemText = L"== [ TRỞ VỀ KHÁN ĐÀI ] ==";
            if (i == selectedOption) {
                int gCol = (int)(150 + sin(g_GlobalAnimTime * 15.0f) * 105);
                color = RGB(max(0, min(255, 255 - gCol)), 100, 255);
                font = GlobalFont::Title;
            } 
            else {
                color = Palette::BlueDarkest;
            }
            RECT btnRect = { panelX, yPos + UIScaler::SY(6), panelX + panelW, yPos + slotH };
            SetTextColor(hdc, color);
            HFONT oldF = (HFONT)SelectObject(hdc, font);
            DrawTextW(hdc, itemText.c_str(), -1, &btnRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(hdc, oldF);
        } 
        else {
            // --- Slot Save (i = 0..4) ---
            bool exists = CheckSaveExists(i + 1);
            itemText = L"Hồ Sơ " + std::to_wstring(i + 1)
                     + (exists ? L"   ⬛ BĂNG GHI HÌNH" : L"   ▢  BĂNG TRỐNG");

            // Nền hộp slot
            if (i == selectedOption) {
                int aCol = (int)(160 + sin(g_GlobalAnimTime * 10.0f) * 70);
                Gdiplus::SolidBrush slotBrush(Theme::SlotSelected.WithAlpha((BYTE)aCol));
                g.FillRectangle(&slotBrush, slotX, yPos, slotW, slotH);

                // Viền sáng khi chọn
                Gdiplus::Pen selPen(Theme::PanelBlueBorder, 2.0f);
                g.DrawRectangle(&selPen, slotX, yPos, slotW, slotH);

                color = Palette::White;
                itemText = L"▶  " + itemText;
            } 
            else {
                Gdiplus::SolidBrush slotBrush(Theme::SlotNormal);
                g.FillRectangle(&slotBrush, slotX, yPos, slotW, slotH);
                color = exists ? Palette::GrayDarkest : Palette::GrayNormal;
            }

            // Vẽ chữ trong slot — RECT khớp đúng kích thước slot
            SetTextColor(hdc, color);
            HFONT oldF = (HFONT)SelectObject(hdc, font);
            RECT textRect = { slotX + UIScaler::SX(16), yPos, slotX + slotW - UIScaler::SX(8), yPos + slotH };
            DrawTextW(hdc, itemText.c_str(), -1, &textRect,
                DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
            SelectObject(hdc, oldF);
        }
    }

    // 5. Thông báo lỗi / trạng thái (đủ vùng hiển thị)
    if (!statusMessage.empty()) {
        int errY = startY + TOTAL_LOAD_ITEMS * spacing + UIScaler::SY(8);
        // Nền đỏ nhạt
        Gdiplus::SolidBrush errBg(Gdiplus::Color(160, 200, 20, 20));
        g.FillRectangle(&errBg, panelX + UIScaler::SX(10), errY, panelW - UIScaler::SX(20), UIScaler::SY(38));
        DrawTextCentered(hdc, statusMessage, errY + UIScaler::SY(8), screenWidth, Palette::White, GlobalFont::Bold);
    }

    // 6. Gợi ý phím
    DrawTextCentered(hdc, L"W/S: Chọn Hồ Sơ   |   ENTER: Chạy băng   |   ESC: Quay lại",
        screenHeight - UIScaler::SY(50), screenWidth, Palette::White, GlobalFont::Note);
}


void UpdateLoadGameScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, std::wstring& statusMessage, WPARAM wParam) {
    // Bỏ qua nếu không có sự kiện phím (wParam = 0)
    if (wParam == 0) {
        return;
    }

    // Ủy quyền xử lý logic cho ProcessLoadGameInput
    ProcessLoadGameInput(wParam, currentState, playState, selectedOption, statusMessage);
}