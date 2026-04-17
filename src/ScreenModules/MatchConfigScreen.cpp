#include "MatchConfigScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h"
#include "../SystemModules/Localization.h"
#include "../SystemModules/AudioSystem.h"
#include <cmath>

static int currentPage = 0; // 0 = Lên Khuôn Trận Đấu, 1 = Lên Sơ Đồ Cầu Thủ

const int PAGE_0_ITEMS = 6;
const int PAGE_1_ITEMS = 6;

static std::wstring editName1 = L"Player 1";
static std::wstring editName2 = L"Player 2";
static int p1AvatarIdx = 0;
static int p2AvatarIdx = 1;

const std::wstring AVATAR_NAMES[] = {
    L"Tiền Đạo Số 7",      // Type 0
    L"Nhạc Trưởng Số 10",  // Type 1
    L"Ảo Thuật Gia Samba",   // Type 2
};
// Mapping: slot 0-2 -> avatarType (0,1,2)
static const int AVATAR_SLOT_TO_TYPE[3] = { 0, 1, 2 };
const int TOTAL_HUMAN_AVATARS = 3;

static bool isEditingName1 = false;
static bool isEditingName2 = false;
static std::wstring validationMsg = L"";

bool ValidateNames(PlayState* playState) {
    bool isPvE = (playState->matchType == MATCH_PVE);
    
    // Auto-fill logic instead of erroring
    if (editName1.empty()) {
        editName1 = L"Player 1";
    }
    
    // Unify character limit to 1-15 (consistent with Save name)
    if (editName1.length() > 15) {
        validationMsg = L"LỖI: Tên P1 tối đa 15 kí tự!";
        return false;
    }

    if (!isPvE) {
        if (editName2.empty()) {
            editName2 = L"Player 2";
        }
        if (editName2.length() > 15) {
            validationMsg = L"LỖI: Tên P2 tối đa 15 kí tự!";
            return false;
        }
        if (editName1 == editName2 && editName1 != L"Player 1") {
            validationMsg = L"LỖI: Tên hai cầu thủ không được trùng nhau!";
            return false;
        }
    }
    validationMsg = L"";
    return true;
}

void UpdateMatchConfigScreen(ScreenState& currentState, PlayState* playState, int& selectedOption, WPARAM wParam) {
    if (wParam == 0) {
        return;
    }

    bool isChar = (wParam & 0x10000);
    wchar_t ch = (wchar_t)(wParam & 0xFFFF);

    if (!isChar) validationMsg = L""; // Xóa lỗi khi người dùng thao tác phím điều hướng

    // --- TRƯỜNG HỢP: ĐANG TRONG CHẾ ĐỘ NHẬP LIỆU TRỰC TIẾP ---
    if (isEditingName1 || isEditingName2) {
        std::wstring& target = isEditingName1 ? editName1 : editName2;
        
        if (!isChar) {
            if (wParam == VK_RETURN || wParam == VK_ESCAPE) {
                isEditingName1 = false;
                isEditingName2 = false;
                ValidateNames(playState); // Kiểm tra ngay sau khi đặt tên
                return;
            }
            if (wParam == VK_BACK) {
                if (!target.empty()) target.pop_back();
                return;
            }
            // Ngăn chặn các phím di chuyển (W,A,S,D, Up, Down...) bị lọt xuống dưới khi đang edit
            return; 
        } else {
            // Nhận ký tự Unicode trực tiếp từ WM_CHAR (UniKey đã xử lý dấu)
            if (ch >= 32 && target.length() < 15) {
                target += ch;
            }
            return;
        }
    }

    // --- TRƯỜNG HỢP: DI CHUYỂN MENU VÀ CÀI ĐẶT ---
    if (isChar) return; // Không xử lý WM_CHAR khi không edit

    int totalItems = (currentPage == 0) ? PAGE_0_ITEMS : PAGE_1_ITEMS;
    
    if (wParam == 'W' || wParam == 'w' || wParam == VK_UP) {
        do {
            selectedOption = (selectedOption - 1 + totalItems) % totalItems;
        } while (
            (currentPage == 0 && (
                (selectedOption == 2 && playState->matchType == MATCH_PVP) || // Bỏ qua Độ Khó nếu PVP
                (selectedOption == 3 && playState->matchType == MATCH_PVE)    // Bỏ qua Thời gian nếu PVE
            )) ||
            (currentPage == 1 && playState->matchType == MATCH_PVE && (selectedOption == 2 || selectedOption == 3)) // Bỏ qua Avatar/Tên P2 nếu PvE
        );
        PlaySFX("sfx_move");
    }
    else if (wParam == 'S' || wParam == 's' || wParam == VK_DOWN) {
        do {
            selectedOption = (selectedOption + 1) % totalItems;
        } while (
            (currentPage == 0 && (
                (selectedOption == 2 && playState->matchType == MATCH_PVP) ||
                (selectedOption == 3 && playState->matchType == MATCH_PVE)
            )) ||
            (currentPage == 1 && playState->matchType == MATCH_PVE && (selectedOption == 2 || selectedOption == 3))
        );
        PlaySFX("sfx_move");
    }

    int dir = (wParam == 'D' || wParam == 'd' || wParam == VK_RIGHT) ? 1 : ((wParam == 'A' || wParam == 'a' || wParam == VK_LEFT) ? -1 : 0);

    if (currentPage == 0) {
        switch (selectedOption) {
        case 0: // Chế độ
            if (dir != 0) {
                playState->gameMode = (playState->gameMode == MODE_CARO) ? MODE_TIC_TAC_TOE : MODE_CARO;
                PlaySFX("sfx_move"); 
            }
            break;
        case 1: // PvP / PvE
            if (dir != 0) {
                playState->matchType = (playState->matchType == MATCH_PVP) ? MATCH_PVE : MATCH_PVP;
                PlaySFX("sfx_move");
            }
            break;
        case 2: // Độ khó
            if (playState->matchType == MATCH_PVE && dir != 0) {
                playState->difficulty += dir;
                if (playState->difficulty < 1) playState->difficulty = 3;
                if (playState->difficulty > 3) playState->difficulty = 1;
                PlaySFX("sfx_move"); 
            }
            break;
        case 3: // Thời gian
            if (dir != 0) {
                playState->countdownTime += dir * 5;
                if (playState->countdownTime < 10) playState->countdownTime = 10;
                if (playState->countdownTime > 60) playState->countdownTime = 60;
                PlaySFX("sfx_move");
            }

            break;
        case 4: // Bo
            if (dir != 0) {
                playState->targetScore += dir * 2;
                if (playState->targetScore < 1) playState->targetScore = 5;
                if (playState->targetScore > 5) playState->targetScore = 1;
                PlaySFX("sfx_move");
            }
            break;
        case 5: // Tiếp Theo
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                currentPage = 1;
                selectedOption = 0;
            }
            break;
        }
    } 
    else {
        bool isPvE = (playState->matchType == MATCH_PVE);
        switch (selectedOption) {
        case 0: // Đổi Avatar P1 (A/D)
            if (dir != 0) {
                p1AvatarIdx = (p1AvatarIdx + dir + TOTAL_HUMAN_AVATARS) % TOTAL_HUMAN_AVATARS;
            }
            break;
        case 1: // Sửa Tên P1 (Enter)
            if (wParam == VK_RETURN) {
                isEditingName1 = true;
                editName1 = L""; // Xóa để nhập mới cho lẹ
            }
            break;
        case 2: // Đổi Avatar P2 (A/D)
            if (!isPvE && dir != 0) {
                p2AvatarIdx = (p2AvatarIdx + dir + TOTAL_HUMAN_AVATARS) % TOTAL_HUMAN_AVATARS;
            }
            break;
        case 3: // Sửa Tên P2 (Enter)
            if (!isPvE && wParam == VK_RETURN) {
                isEditingName2 = true;
                editName2 = L"";
            }
            break;
        case 4: // Quay lại
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                currentPage = 0;
                selectedOption = 5;
            }
            break;
        case 5: // Bắt Đầu
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                if (!ValidateNames(playState)) return;

                // Nếu hợp lệ thì tiến hành vào trận
                playState->p1.name = editName1;
                // Mapping: slot index -> path string cho decodeAvatar
                static const char* SLOT_PATHS[3] = {
                    "avatar_0", "avatar_1", "avatar_2"
                };
                playState->p1.avatarPath = SLOT_PATHS[p1AvatarIdx];

                if (isPvE) {
                    std::wstring botName = (playState->difficulty == 1) ? L"Ảo Thuật Gia Samba" : (playState->difficulty == 2 ? L"Bot Thiết Giáp" : L"Bóng Đêm Thách Đấu");
                    playState->p2.name = botName;
                    playState->p2.avatarPath = "avatar_0"; // All bots use Ronaldo
                }
                else {
                    playState->p2.name = editName2;
                    static const char* SLOT_PATHS[3] = {
                        "avatar_0", "avatar_1", "avatar_2"
                    };
                    playState->p2.avatarPath = SLOT_PATHS[p2AvatarIdx];
                }

                int bSize = (playState->gameMode == MODE_CARO) ? 15 : 3;
                initNewMatch(playState, playState->gameMode, playState->matchType, bSize, playState->countdownTime, playState->difficulty, playState->targetScore, 15);
                
                StopBGM(); // Tắt nhạc menu Champions League
                PlaySFX("sfx_whistle");

                currentPage = 0;
                selectedOption = 0;
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
    RECT rect = { x, y, x + width, y + UIScaler::SY(80) };
    DrawTextW(hdc, text.c_str(), -1, &rect, format | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFont);
}

void RenderMatchConfigScreen(HDC hdc, int selectedOption, const PlayState* config, int screenWidth, int screenHeight) {
    Gdiplus::Graphics g(hdc);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);

    // Mảng 1: Lấy nền Procedural Stadium
    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Khung Trắng Kính (White Glassmorphism)
    Gdiplus::SolidBrush whitePanel(ToGdiColor(Theme::GlassWhite));
    int panelW = UIScaler::SX(750);
    int panelH = UIScaler::SY(500);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2 - UIScaler::SY(10);

    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    // Vien xanh la mem mai quanh panel
    Gdiplus::Pen panelPen(ToGdiColor(Theme::PanelGreenBorder), 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    // Xử lý 2 Trang
    if (currentPage == 0) {
        // --- ĐÃ GẮN ĐA NGÔN NGỮ ---
        DrawTextCentered(hdc, GetText("config_page1").c_str(), panelY + UIScaler::SY(30), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Title);

        int col1X = panelX + UIScaler::SX(30);
        int col1W = UIScaler::SX(320);
        int col2X = panelX + UIScaler::SX(370);
        int col2W = UIScaler::SX(350);
        int startY = panelY + UIScaler::SY(120);
        int spacing = UIScaler::SY(50);

        // --- ĐÃ GẮN ĐA NGÔN NGỮ ---
        std::wstring labels[] = {
            GetText("config_size"),
            GetText("config_type"),
            GetText("match_diff"),
            GetText("config_time"),
            GetText("config_bo")
        };

        std::wstring values[] = {
            std::wstring(config->gameMode == MODE_CARO ? L"< Caro 15x15 >" : L"< Tic-Tac-Toe 3x3 >"),
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
            // Ẩn Độ Khó khi chế độ PvP (không có máy)
            if (i == 2 && config->matchType == MATCH_PVP) continue;

            COLORREF valColor = ToCOLORREF(Palette::GrayDarkest);
            if (i == selectedOption) {
                int rCol = (int)(180 + sin(g_GlobalAnimTime * 12.0f) * 75);
                valColor = RGB(255, max(0, min(255, 255 - rCol)), 0);
            }

            HFONT fontVal = (i == selectedOption) ? GlobalFont::Bold : GlobalFont::Default;
            DrawColText(hdc, labels[i], col1X, startY + drawRow * spacing, col1W, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold, DT_RIGHT);
            DrawColText(hdc, values[i], col2X, startY + drawRow * spacing, col2W, valColor, fontVal, DT_LEFT);
            drawRow++;
        }

        // Nút Tiếp Theo (ĐÃ GẮN ĐA NGÔN NGỮ)
        COLORREF nextColor = ToCOLORREF(Palette::BlueDarkest);
        if (selectedOption == 5) {
            int gCol = (int)(150 + sin(g_GlobalAnimTime * 15.0f) * 105);
            nextColor = RGB(max(0, min(255, 255 - gCol)), 100, 255); // Pulse Blue/Cyan
        }
        DrawTextCentered(hdc, GetText("config_next").c_str(), startY + 5 * spacing + UIScaler::SY(40), screenWidth, nextColor, (selectedOption == 5) ? GlobalFont::Title : GlobalFont::Bold);
    }
    else {
        // --- ĐÃ GẮN ĐA NGÔN NGỮ ---
        DrawTextCentered(hdc, GetText("config_page2").c_str(), panelY + UIScaler::SY(30), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Title);

        int halfW = panelW / 2;
        int avaSize = UIScaler::S(160); // Tăng từ 130
        int avaY = panelY + UIScaler::SY(120);
        bool isPvE = (config->matchType == MATCH_PVE);

        // --- CỘT TRÁI (P1) ---
        // Draw Watermark Parallax P1 (Kéo lùi góc dưới để tạo cảm giác chéo không gian)
        DrawColText(hdc, L"ĐỘI P1", panelX - UIScaler::SX(15), avaY + UIScaler::SY(30), halfW, RGB(255, 230, 220), GlobalFont::Title, DT_CENTER);

        int avaP1X = panelX + (halfW - avaSize) / 2;
        // Sử dụng Avatar Pixel Procedural ĐÈ lên chữ mờ ở dưới
        DrawPixelAvatar(g, avaP1X, avaY, avaSize, AVATAR_SLOT_TO_TYPE[p1AvatarIdx]);

        // Mục 0 = Đổi Avatar P1, Mục 1 = Sửa Tên P1
        COLORREF avaP1Col = (selectedOption == 0) ? RGB(255, 120, 0) : ToCOLORREF(Palette::GrayDarkest);
        int textY = avaY + avaSize + UIScaler::SY(10);
        if (selectedOption == 0) {
            int pulse = UIScaler::SY((int)(sin(g_GlobalAnimTime * 10.0f) * 3));
            DrawPixelFootball(g, panelX + UIScaler::SX(30), textY + UIScaler::SY(40) + pulse, UIScaler::S(24));
            DrawPixelFootball(g, panelX + halfW - UIScaler::SX(30), textY + UIScaler::SY(40) - pulse, UIScaler::S(24));
        }
        // Tên Avatar (slot 0..5 -> hiển thị tên tương ứng)
        DrawColText(hdc, AVATAR_NAMES[p1AvatarIdx], panelX, textY, halfW, avaP1Col, selectedOption == 0 ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);
        // Chỉ thị slot (ví dụ “< 3/6 >”)
        std::wstring p1SlotStr = L"< " + std::to_wstring(p1AvatarIdx + 1) + L"/" + std::to_wstring(TOTAL_HUMAN_AVATARS) + L" >";
        DrawColText(hdc, p1SlotStr, panelX, textY + UIScaler::SY(28), halfW, avaP1Col, GlobalFont::Note, DT_CENTER);

        // Tên Player (selectedOption==1 = đang chọn tên) - (ĐÃ GẮN ĐA NGÔN NGỮ)
        COLORREF nameP1Col = (selectedOption == 1) ? RGB(255, 120, 0) : ToCOLORREF(Palette::GrayDarkest);
        bool isActualEditing1 = isEditingName1;
        bool showCursor1 = isActualEditing1 && ((int)(g_GlobalAnimTime * 2.5f) % 2 == 0);
        // Luôn cộng thêm 1 ký tự (gạch hoặc khoảng trắng) khi đang edit để giữ vị trí trung tâm cố định
        std::wstring p1DispName = editName1 + (isActualEditing1 ? (showCursor1 ? L"_" : L" ") : L"");
        DrawColText(hdc, GetText("config_pname") + p1DispName, panelX, avaY + avaSize + UIScaler::SY(50), halfW, nameP1Col, selectedOption == 1 ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);

        // --- CỘT PHẢI (P2) ---
        // Watermark Parallax P2
        DrawColText(hdc, L"ĐỘI P2", panelX + halfW + UIScaler::SX(15), avaY + UIScaler::SY(30), halfW, RGB(220, 245, 255), GlobalFont::Title, DT_CENTER);

        int avaP2X = panelX + halfW + (halfW - avaSize) / 2;
        std::wstring p2BotName = L"";
        int aiAvatarIdx = p2AvatarIdx;

        if (isPvE) {
            aiAvatarIdx = 0; // Forced to Ronaldo
            if (config->difficulty == 1) { p2BotName = L"Máy Đồng Bài"; }
            else if (config->difficulty == 2) { p2BotName = L"Máy Vàng Trắng"; }
            else { p2BotName = L"Vua Thách Đấu"; }
        }

        // Đè lưới Avatar lên trên chữ nền Watermark
        int p2AvaType = isPvE ? aiAvatarIdx : AVATAR_SLOT_TO_TYPE[p2AvatarIdx];
        DrawPixelAvatar(g, avaP2X, avaY, avaSize, p2AvaType);

        // Mục 2 = Sửa Tên P2, Mục 3 = Đổi Avatar P2 (đối xứng với P1)
        COLORREF avaP2Col = (selectedOption == 2 && !isPvE) ? RGB(0, 200, 255) : (isPvE ? ToCOLORREF(Palette::GrayNormal) : ToCOLORREF(Palette::GrayDarkest));
        int textP2Y = avaY + avaSize + UIScaler::SY(10);
        if (selectedOption == 2 && !isPvE) {
            int pulse = UIScaler::SY((int)(sin(g_GlobalAnimTime * 10.0f) * 3));
            DrawPixelFootball(g, panelX + halfW + UIScaler::SX(30), textP2Y + UIScaler::SY(40) + pulse, UIScaler::S(24));
            DrawPixelFootball(g, panelX + panelW - UIScaler::SX(30), textP2Y + UIScaler::SY(40) - pulse, UIScaler::S(24));
        }
        // Tên Avatar P2 (selectedOption==2 = đang chọn avatar P2) - (ĐÃ GẮN ĐA NGÔN NGỮ)
        std::wstring p2AvaStr = isPvE ? GetText("config_locked") : AVATAR_NAMES[p2AvatarIdx];
        DrawColText(hdc, p2AvaStr, panelX + halfW, textP2Y, halfW, avaP2Col, (selectedOption == 2 && !isPvE) ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);
        // Chỉ thị slot P2
        if (!isPvE) {
            std::wstring p2SlotStr = L"< " + std::to_wstring(p2AvatarIdx + 1) + L"/" + std::to_wstring(TOTAL_HUMAN_AVATARS) + L" >";
            DrawColText(hdc, p2SlotStr, panelX + halfW, textP2Y + UIScaler::SY(28), halfW,
                (selectedOption == 2) ? RGB(0, 200, 255) : ToCOLORREF(Palette::GrayDark), GlobalFont::Note, DT_CENTER);
        }

        // Tên Player P2 (selectedOption==3 = đang chọn tên P2) - (ĐÃ GẮN ĐA NGÔN NGỮ)
        COLORREF nameP2Col = (selectedOption == 3 && !isPvE) ? RGB(0, 200, 255) : (isPvE ? ToCOLORREF(Palette::GrayNormal) : ToCOLORREF(Palette::GrayDarkest));
        bool isActualEditing2 = isEditingName2;
        bool showCursor2 = isActualEditing2 && ((int)(g_GlobalAnimTime * 2.5f) % 2 == 0);
        std::wstring p2DispName = isPvE ? p2BotName : (editName2 + (isActualEditing2 ? (showCursor2 ? L"_" : L" ") : L""));
        DrawColText(hdc, GetText("config_pname") + p2DispName, panelX + halfW, avaY + avaSize + UIScaler::SY(50), halfW, nameP2Col, (selectedOption == 3 && !isPvE) ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);

        // --- BUTTONS BÊN DƯỚI DÀN HÀNG NGANG ---
        int botY = panelY + panelH - UIScaler::SY(80);

        // (ĐÃ GẮN ĐA NGÔN NGỮ)
        COLORREF backCol = (selectedOption == 4) ? RGB(255, 0, 0) : ToCOLORREF(Palette::GrayDark); // Màu đỏ khi back
        DrawColText(hdc, GetText("config_back"), panelX + UIScaler::SX(30), botY, halfW - UIScaler::SX(30), backCol, selectedOption == 4 ? GlobalFont::Bold : GlobalFont::Default, DT_LEFT);

        // (ĐÃ GẮN ĐA NGÔN NGỮ)
        COLORREF startCol = ToCOLORREF(Palette::GreenDark);
        if (selectedOption == 5) {
            int gCol = (int)(150 + sin(g_GlobalAnimTime * 20.0f) * 105);
            startCol = RGB(0, max(0, min(255, gCol)), 0);
        }
        // Nới rộng RECT về bên trái để khi phóng to ko bị chém mất chữ 'X' do DT_RIGHT đẩy văng ra ngoài
        DrawColText(hdc, GetText("config_start"), panelX, botY, panelW - UIScaler::SX(30), startCol, selectedOption == 5 ? GlobalFont::Title : GlobalFont::Bold, DT_RIGHT);

        // --- ANIMATION NGOÀI PANEL (tả = P1, hữu = P2) ---
        {
            // Vị trí Y căn giữa theo panel, đẩy xuống một chút để đứng thăng bằng
            int animSize = UIScaler::S(360);
            int animCY = panelY + panelH / 2 + UIScaler::SY(50);

            // P1: bên trái panel — animation IDLE, nhìn vào trong (flipH=false)
            static PlayerState cfgP1State;
            cfgP1State.avatarType = AVATAR_SLOT_TO_TYPE[p1AvatarIdx];
            cfgP1State.currentAction = "idle";
            cfgP1State.flipH = false;
            int leftCX = panelX / 2; // giữa khoảng trắng bên trái
            DrawPixelAction(g, leftCX, animCY, animSize, cfgP1State);

            // P2: bên phải panel — animation IDLE, nhìn vào trong (flipH=true)
            static PlayerState cfgP2State;
            int p2Type = isPvE ? 0 : AVATAR_SLOT_TO_TYPE[p2AvatarIdx];

            cfgP2State.avatarType = p2Type;
            cfgP2State.currentAction = "idle";
            cfgP2State.flipH = true;
            int rightCX = panelX + panelW + (screenWidth - panelX - panelW) / 2;
            DrawPixelAction(g, rightCX, animCY, animSize, cfgP2State);
        }
    }

    // --- THÔNG BÁO LỖI VALIDATION ---
    if (!validationMsg.empty()) {
        DrawTextCentered(hdc, validationMsg, panelY + panelH - UIScaler::SY(45), screenWidth, RGB(255, 50, 50), GlobalFont::Bold);
    }

    // Giữ nguyên dòng hướng dẫn gốc để bạn không phải sửa file txt lúc khuya
    DrawTextCentered(hdc, (isEditingName1 || isEditingName2) ? L"Gõ tên và ấn Enter để chốt" : L"A/D: Thay đổi  |  Enter: Nhập Tên Trực Tiếp  |  ESC: Về Menu", screenHeight - UIScaler::SY(60), screenWidth, ToCOLORREF(Palette::White), GlobalFont::Note);
}