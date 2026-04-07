#include "MatchConfigScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h"
#include <cmath>

static int currentPage = 0; // 0 = Lên Khuôn Trận Đấu, 1 = Lên Sơ Đồ Cầu Thủ

const int PAGE_0_ITEMS = 6;
const int PAGE_1_ITEMS = 6;

static std::wstring editName1 = L"Player 1";
static std::wstring editName2 = L"Player 2";
static int p1AvatarIdx = 0;
static int p2AvatarIdx = 1;
static int activeEditing = 0; // 0: None, 1: P1 Edit, 2: P2 Edit

const std::wstring AVATAR_NAMES[] = { L"Tiền Đạo Số 7", L"Nhạc Trưởng Số 10" };
const std::string AVATAR_PATHS[] = { "Asset/images/avatar_1.png", "Asset/images/avatar_2.png" };

void UpdateMatchConfigScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, WPARAM wParam) {
    if (wParam == 0) return;

    // --- TRƯỜNG HỢP 1: ĐANG NHẬP TÊN (CHỈ Ở TRANG 2) ---
    if (activeEditing != 0 && currentPage == 1) {
        std::wstring& targetName = (activeEditing == 1) ? editName1 : editName2;

        if (wParam == VK_RETURN) { // Nhấn Enter để hoàn tất nhập
            activeEditing = 0;
            return;
        }
        if (wParam == VK_BACK) { // Xóa ký tự
            if (!targetName.empty()) targetName.pop_back();
            return;
        }
        // Nhận ký tự
        if (targetName.length() < 12 && ((wParam >= 'A' && wParam <= 'Z') || (wParam >= 'a' && wParam <= 'z') || (wParam >= '0' && wParam <= '9') || wParam == VK_SPACE)) {
            targetName += (wchar_t)wParam;
        }
        return;
    }

    int totalItems = (currentPage == 0) ? PAGE_0_ITEMS : PAGE_1_ITEMS;

    // --- TRƯỜNG HỢP 2: DI CHUYỂN MENU ---
    if (wParam == 'W' || wParam == 'w' || wParam == VK_UP) {
        do {
            selectedOption = (selectedOption - 1 + totalItems) % totalItems;
        } while (currentPage == 0 && (
            (selectedOption == 2 && playState->matchType == MATCH_PVP) || // Bỏ qua Độ Khó nếu PVP
            (selectedOption == 3 && playState->matchType == MATCH_PVE)    // Bỏ qua Thời gian nếu PVE
        ));
    }
    else if (wParam == 'S' || wParam == 's' || wParam == VK_DOWN) {
        do {
            selectedOption = (selectedOption + 1) % totalItems;
        } while (currentPage == 0 && (
            (selectedOption == 2 && playState->matchType == MATCH_PVP) ||
            (selectedOption == 3 && playState->matchType == MATCH_PVE)
        ));
    }

    int dir = (wParam == 'D' || wParam == 'd' || wParam == VK_RIGHT) ? 1 : ((wParam == 'A' || wParam == 'a' || wParam == VK_LEFT) ? -1 : 0);

    if (currentPage == 0) {
        switch (selectedOption) {
        case 0: // Chế độ
            if (dir != 0) playState->gameMode = (playState->gameMode == MODE_CARO) ? MODE_TIC_TAC_TOE : MODE_CARO;
            break;
        case 1: // PvP / PvE
            if (dir != 0) playState->matchType = (playState->matchType == MATCH_PVP) ? MATCH_PVE : MATCH_PVP;
            break;
        case 2: // Độ khó
            if (playState->matchType == MATCH_PVE && dir != 0) {
                playState->difficulty += dir;
                if (playState->difficulty < 1) playState->difficulty = 3;
                if (playState->difficulty > 3) playState->difficulty = 1;
            }
            break;
        case 3: // Thời gian
            if (dir != 0) {
                playState->countdownTime += dir * 5;
                if (playState->countdownTime < 10) playState->countdownTime = 10;
                if (playState->countdownTime > 60) playState->countdownTime = 60;
            }
            break;
        case 4: // Bo
            if (dir != 0) {
                playState->targetScore += dir * 2;
                if (playState->targetScore < 1) playState->targetScore = 5;
                if (playState->targetScore > 5) playState->targetScore = 1;
            }
            break;
        case 5: // Tiếp Theo
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                currentPage = 1;
                selectedOption = 0;
            }
            break;
        }
    } else {
        bool isPvE = (playState->matchType == MATCH_PVE);
        switch (selectedOption) {
        case 0: // Đổi Avatar P1 (A/D)
            if (dir != 0) p1AvatarIdx = (p1AvatarIdx + dir + 2) % 2;
            break;
        case 1: // Sửa Tên P1 (Enter)
            if (wParam == VK_RETURN) activeEditing = 1;
            break;
        case 2: // Đổi Avatar P2 (A/D)
            if (!isPvE && dir != 0) p2AvatarIdx = (p2AvatarIdx + dir + 2) % 2;
            break;
        case 3: // Sửa Tên P2 (Enter)
            if (!isPvE && wParam == VK_RETURN) activeEditing = 2;
            break;
        case 4: // Quay lại
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                currentPage = 0;
                selectedOption = 5;
            }
            break;
        case 5: // Bắt Đầu
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                playState->p1.name = editName1;
                playState->p1.avatarPath = AVATAR_PATHS[p1AvatarIdx];

                if (isPvE) {
                    std::wstring botName = (playState->difficulty == 1) ? L"Trạm Trưởng Vàng" : (playState->difficulty == 2 ? L"Bot Thiết Giáp" : L"Bóng Đêm Thách Đấu");
                    playState->p2.name = botName;
                    if (playState->difficulty == 1) playState->p2.avatarPath = "Asset/images/bot_easy.png";
                    else if (playState->difficulty == 2) playState->p2.avatarPath = "Asset/images/bot_medium.png";
                    else playState->p2.avatarPath = "Asset/images/bot_hard.png";
                }
                else {
                    playState->p2.name = editName2;
                    playState->p2.avatarPath = AVATAR_PATHS[p2AvatarIdx];
                }

                int bSize = (playState->gameMode == MODE_CARO) ? 15 : 3;
                initNewMatch(playState, playState->gameMode, playState->matchType, bSize, playState->countdownTime, playState->difficulty, playState->targetScore, 15);
                
                // Trả về trạng thái ban đầu cho lần sau
                currentPage = 0;
                selectedOption = 0;
                activeEditing = 0;
                currentState = SCREEN_PLAY;
            }
            break;
        }
    }
}

// Hàm hỗ trợ vẽ text theo ô (Cột)
void DrawColText(HDC hdc, const std::wstring& text, int x, int y, int width, COLORREF color, HFONT font, UINT format) {
    SetTextColor(hdc, color);
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    SetBkMode(hdc, TRANSPARENT);
    // Tăng vùng đệm chiều cao từ 42 lên 80 để các Font to (GlobalFont::Title) không bị xén đỉnh/đáy
    RECT rect = { x, y, x + width, y + 80 };
    DrawTextW(hdc, text.c_str(), -1, &rect, format | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}

void RenderMatchConfigScreen(HDC hdc, int selectedOption, const PlayState* config, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    
    // Mảng 1: Lấy nền Procedural Stadium
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Khung Trắng Kính (White Glassmorphism)
    Gdiplus::SolidBrush whitePanel(GdipColour::GLASS_WHITE);
    int panelW = 750;
    int panelH = 500;
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2 - 10;

    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    // Vien xanh la mem mai quanh panel
    Gdiplus::Pen panelPen(GdipColour::PANEL_GREEN_BORDER, 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // Xử lý 2 Trang
    if (currentPage == 0) {
        DrawTextCentered(hdc, L"--- 1. HỒ SƠ CHIẾN THUẬT ---", panelY + 30, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

        int col1X = panelX + 30;
        int col1W = 320;
        int col2X = panelX + 370;
        int col2W = 350;
        int startY = panelY + 120;
        int spacing = 50;

        std::wstring labels[] = {
            L"Khổ sân thi đấu: ",
            L"Bảng Bốc Thăm: ",
            L"Cấp độ Máy: ",
            L"Kiểm soát bóng (Lượt): ",
            L"Thể thức phân định: "
        };
        std::wstring values[] = {
            std::wstring(config->gameMode == MODE_CARO ? L"< Caro Hiện Đại 15x15 >" : L"< Tic-Tac-Toe Sân Cỏ 3x3 >"),
            std::wstring(config->matchType == MATCH_PVP ? L"< Kinh Điển Cùng Lò (PvP) >" : L"< Thách Đấu Máy (PvE) >"),
            std::wstring(config->difficulty == 1 ? L"< Phân Hạng Đồng >" : (config->difficulty == 2 ? L"< Phân Hạng Vàng >" : L"< Thách Đấu >")),
            L"< " + std::to_wstring(config->countdownTime) + L" Giây >",
            L"< Chạm " + std::to_wstring(config->targetScore) + L" Bàn >"
        };

        bool isPvE_p0 = (config->matchType == MATCH_PVE);
        int drawRow = 0; // Hàng thực tế được vẽ
        for (int i = 0; i < 5; i++) {
            // Ẩn Thời Gian khi chế độ PvE (AI tự tính toán)
            if (i == 3 && isPvE_p0) continue;

            COLORREF valColor = Colour::GRAY_DARKEST;
            if (i == 2 && config->matchType == MATCH_PVP) {
                valColor = Colour::GRAY_NORMAL;
                values[i] = L"[ Vô Hiệu Hóa ]";
            }
            if (i == selectedOption) {
                int rCol = (int)(180 + sin(g_GlobalAnimTime * 12.0f) * 75);
                valColor = RGB(255, max(0, min(255, 255 - rCol)), 0); 
            }
            
            HFONT fontVal = (i == selectedOption) ? GlobalFont::Bold : GlobalFont::Default;
            DrawColText(hdc, labels[i], col1X, startY + drawRow * spacing, col1W, Colour::GRAY_DARKEST, GlobalFont::Bold, DT_RIGHT);
            DrawColText(hdc, values[i], col2X, startY + drawRow * spacing, col2W, valColor, fontVal, DT_LEFT);
            drawRow++;
        }

        // Nút Tiếp Theo
        COLORREF nextColor = Colour::BLUE_DARKEST;
        if (selectedOption == 5) {
            int gCol = (int)(150 + sin(g_GlobalAnimTime * 15.0f) * 105);
            nextColor = RGB(max(0, min(255, 255 - gCol)), 100, 255); // Pulse Blue/Cyan
        }
        DrawTextCentered(hdc, L"==> [ TIẾP THEO ] ==>", startY + 5 * spacing + 40, screenWidth, nextColor, (selectedOption == 5) ? GlobalFont::Title : GlobalFont::Bold);
    }
    else {
        DrawTextCentered(hdc, L"--- 2. HỒ SƠ TUYỂN THỦ ---", panelY + 30, screenWidth, Colour::BLUE_DARKEST, GlobalFont::Title);

        int halfW = panelW / 2;
        int avaSize = 130;
        int avaY = panelY + 120;
        bool isPvE = (config->matchType == MATCH_PVE);

        // --- CỘT TRÁI (P1) ---
        // Draw Watermark Parallax P1 (Kéo lùi góc dưới để tạo cảm giác chéo không gian)
        DrawColText(hdc, L"ĐỘI P1", panelX - 15, avaY + 30, halfW, RGB(255, 230, 220), GlobalFont::Title, DT_CENTER);
        
        int avaP1X = panelX + (halfW - avaSize) / 2;
        // Sử dụng Avatar Pixel Procedural ĐÈ lên chữ mờ ở dưới
        DrawPixelAvatar(g, avaP1X, avaY, avaSize, p1AvatarIdx);

        // Mục 0 = Đổi Avatar P1, Mục 1 = Sửa Tên P1
        COLORREF avaP1Col = (selectedOption == 0) ? RGB(255, 120, 0) : Colour::GRAY_DARKEST;
        int textY = avaY + avaSize + 10;
        if (selectedOption == 0) {
            int pulse = (int)(sin(g_GlobalAnimTime * 10.0f) * 3);
            DrawPixelFootball(g, panelX + 30, textY + 40 + pulse, 24);
            DrawPixelFootball(g, panelX + halfW - 30, textY + 40 - pulse, 24);
        }
        // Tên Avatar (selectedOption==0 = đang chọn avatar)
        DrawColText(hdc, AVATAR_NAMES[p1AvatarIdx], panelX, textY, halfW, avaP1Col, selectedOption == 0 ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);
        
        // Tên Player (selectedOption==1 = đang chọn tên)
        COLORREF nameP1Col = (selectedOption == 1) ? RGB(255, 120, 0) : Colour::GRAY_DARKEST;
        std::wstring p1DispName = editName1 + ((activeEditing == 1) ? L"_" : L"");
        DrawColText(hdc, L"Tên: " + p1DispName, panelX, avaY + avaSize + 50, halfW, nameP1Col, selectedOption == 1 ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);

        // --- CỘT PHẢI (P2) ---
        // Watermark Parallax P2
        DrawColText(hdc, L"ĐỘI P2", panelX + halfW + 15, avaY + 30, halfW, RGB(220, 245, 255), GlobalFont::Title, DT_CENTER);
        
        int avaP2X = panelX + halfW + (halfW - avaSize) / 2;
        std::wstring p2BotName = L"";
        int aiAvatarIdx = p2AvatarIdx;
        
        if (isPvE) {
            if (config->difficulty == 1) { aiAvatarIdx = 2; p2BotName = L"Máy Đồng Bài"; }
            else if (config->difficulty == 2) { aiAvatarIdx = 3; p2BotName = L"Máy Vàng Trắng"; }
            else { aiAvatarIdx = 4; p2BotName = L"Vua Thách Đấu"; }
        }

        // Đè lưới Avatar lên trên chữ nền Watermark
        DrawPixelAvatar(g, avaP2X, avaY, avaSize, aiAvatarIdx);

        // Mục 2 = Sửa Tên P2, Mục 3 = Đổi Avatar P2 (đối xứng với P1)
        COLORREF avaP2Col = (selectedOption == 2 && !isPvE) ? RGB(0, 200, 255) : (isPvE ? Colour::GRAY_NORMAL : Colour::GRAY_DARKEST);
        int textP2Y = avaY + avaSize + 10;
        if (selectedOption == 2 && !isPvE) {
            int pulse = (int)(sin(g_GlobalAnimTime * 10.0f) * 3);
            DrawPixelFootball(g, panelX + halfW + 30, textP2Y + 40 + pulse, 24);
            DrawPixelFootball(g, panelX + panelW - 30, textP2Y + 40 - pulse, 24);
        }
        // Tên Avatar P2 (selectedOption==2 = đang chọn avatar P2)
        std::wstring p2AvaStr = isPvE ? L"[ ĐÃ KHÓA ]" : AVATAR_NAMES[p2AvatarIdx];
        DrawColText(hdc, p2AvaStr, panelX + halfW, textP2Y, halfW, avaP2Col, (selectedOption == 2 && !isPvE) ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);
        
        // Tên Player P2 (selectedOption==3 = đang chọn tên P2)
        COLORREF nameP2Col = (selectedOption == 3 && !isPvE) ? RGB(0, 200, 255) : (isPvE ? Colour::GRAY_NORMAL : Colour::GRAY_DARKEST);
        std::wstring p2DispName = isPvE ? p2BotName : editName2 + ((activeEditing == 2) ? L"_" : L"");
        DrawColText(hdc, L"Tên: " + p2DispName, panelX + halfW, avaY + avaSize + 50, halfW, nameP2Col, (selectedOption == 3 && !isPvE) ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);

        // --- BUTTONS BÊN DƯỚI DÀN HÀNG NGANG ---
        int botY = panelY + panelH - 80;
        
        COLORREF backCol = (selectedOption == 4) ? RGB(255, 0, 0) : Colour::GRAY_DARK; // Màu đỏ khi back
        DrawColText(hdc, L"< QUAY LẠI CÀI ĐẶT SÂN", panelX + 30, botY, halfW - 30, backCol, selectedOption == 4 ? GlobalFont::Bold : GlobalFont::Default, DT_LEFT);

        COLORREF startCol = Colour::GREEN_DARK;
        if (selectedOption == 5) {
            int gCol = (int)(150 + sin(g_GlobalAnimTime * 20.0f) * 105);
            startCol = RGB(0, max(0, min(255, gCol)), 0); 
        }
        // Nới rộng RECT về bên trái để khi phóng to ko bị chém mất chữ 'X' do DT_RIGHT đẩy văng ra ngoài
        DrawColText(hdc, L"XUỐNG SÂN BẮT ĐẦU >", panelX, botY, panelW - 30, startCol, selectedOption == 5 ? GlobalFont::Title : GlobalFont::Bold, DT_RIGHT);
    }
    
    DrawTextCentered(hdc, (activeEditing != 0) ? L"Gõ tên và ấn Enter để chốt" : L"A/D: Thay đổi  |  Enter: Xác nhận  |  ESC: Về Menu", screenHeight - 60, screenWidth, Colour::WHITE, GlobalFont::Note);
}