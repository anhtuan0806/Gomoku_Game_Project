#include "PlayScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h"
#include "../GameLogic/BotAI.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/SaveLoadSystem.h"
#include "../SystemModules/ConfigLoader.h"

#include "../ApplicationTypes/GameConstants.h"
#include <future>
#include <cmath>

extern bool UpdateCountdown(PlayState* state, double dt);

static std::future<std::pair<int, int>> g_AIFuture;
static bool g_AIsCalculating = false;

static int cachedCellSize = -1;
static Sprite g_CachedX = {nullptr, 0, 0};
static Sprite g_CachedO = {nullptr, 0, 0};
static float g_WinAnimTime = 0.0f;

bool UpdatePlayLogic(PlayState* state, double dt) {
    bool needsRedraw = false;

    if (state->status == MATCH_FINISHED) {
        g_WinAnimTime += (float)dt;
        needsRedraw = true;
    } else {
        g_WinAnimTime = 0.0f;
    }

    if (state->status == MATCH_PLAYING) {
        if (state->matchType == MATCH_PVE) {
            state->matchDuration += (float)dt;
            needsRedraw = true; // For timer display update
        } else {
            // Nếu một giây trôi qua, UpdateCountdown trả về true
            if (UpdateCountdown(state, dt)) {
                needsRedraw = true;

                // Nếu sau khi trừ giây mà thời gian bằng 0 -> Hết lượt, đổi người chơi
                if (state->timeRemaining <= 0) {
                    switchTurn(state);
                }
            }
        }
    }

    if (state->status == MATCH_PLAYING && state->matchType == MATCH_PVE && !state->isP1Turn) {
        if (!g_AIsCalculating) {
            g_AIsCalculating = true;
            PlayState localState = *state; 
            g_AIFuture = std::async(std::launch::async, [localState]() {
                int r, c;
                calculateAIMove(&localState, localState.difficulty, r, c);
                return std::make_pair(r, c);
            });
        } else if (g_AIFuture.valid() && g_AIFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            auto bestMove = g_AIFuture.get();
            g_AIsCalculating = false;
            if (processMove(state, bestMove.first, bestMove.second)) {
                needsRedraw = true;
            }
        }
    }

    return needsRedraw;
}

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState, GameConfig* config) {
    bool hasChanged = false;

    if (state->status == MATCH_PAUSED) {
        // TRƯỜNG HỢP 1: ĐANG NHẬP TÊN 
        if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
            if (wParam == VK_ESCAPE) {
                g_CurrentSubMenu = SUB_SAVE_SELECT;
                return true;
            }
            if (wParam == VK_RETURN) {
                if (g_SaveNameInput.empty()) {
                    g_SaveNameInput = L"NEW_SAVE";
                }
                else if (SaveMatchData(state, GetSavePath(g_SaveSlotSelected + 1).c_str())) {
                    PlaySFX(L"Asset/audio/success.wav");
                    g_CurrentSubMenu = SUB_MAIN; 
                    g_SaveNameInput = L"";
                }
                return true;
            }
            if (wParam == VK_BACK) {
                if (!g_SaveNameInput.empty()) { 
                    g_SaveNameInput.pop_back(); hasChanged = true; 
                }
            }
            else if (g_SaveNameInput.length() < MAX_SAVE_NAME_LEN) {
                if ((wParam >= '0' && wParam <= '9') || (wParam >= 'A' && wParam <= 'Z') || wParam == VK_SPACE) {
                    g_SaveNameInput += (wchar_t)wParam;
                    hasChanged = true;
                }
            }
            return hasChanged;
        }

        // TRƯỜNG HỢP 2: CHỌN SLOT LƯU
        if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            if (wParam == VK_ESCAPE) { g_CurrentSubMenu = SUB_MAIN; return true; }
            if (wParam == 'W' || wParam == VK_UP) {
                g_SaveSlotSelected = (g_SaveSlotSelected - 1 < 0) ? MAX_SAVE_SLOTS - 1 : g_SaveSlotSelected - 1;
                return true;
            }
            if (wParam == 'S' || wParam == VK_DOWN) {
                g_SaveSlotSelected = (g_SaveSlotSelected + 1 >= MAX_SAVE_SLOTS) ? 0 : g_SaveSlotSelected + 1;
                return true;
            }
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                g_CurrentSubMenu = SUB_SAVE_NAME_ENTRY;
                g_SaveNameInput = L"";
                return true;
            }
            return false;
        }

        // TRƯỜNG HỢP 3: MENU PAUSE CHÍNH
        if (wParam == VK_ESCAPE) { 
            state->status = MATCH_PLAYING;     
            return true; 
        }

        if (wParam == 'W' || wParam == VK_UP) {
            g_PauseSelected = (g_PauseSelected - 1 < 0) ? TOTAL_PAUSE_ITEMS - 1 : g_PauseSelected - 1;
            hasChanged = true;
        }
        else if (wParam == 'S' || wParam == VK_DOWN) {
            g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            hasChanged = true;
        }
        else if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_RIGHT || wParam == VK_LEFT) {
            int dir = (wParam == VK_LEFT) ? -1 : 1;
            switch (g_PauseSelected) {
            case 0: 
                state->status = MATCH_PLAYING; 
                break; 
            case 1: 
                config->isBgmEnabled = !config->isBgmEnabled; 
                if (!config->isBgmEnabled) { 
                    StopBGM(); 
                }
                break; 
            case 2:
                config->sfxVolume += dir * 10;
                if (config->sfxVolume > 100) {
                    config->sfxVolume = 100;
                }
                if (config->sfxVolume < 0) {
                    config->sfxVolume = 0;
                }
                break;
            case 3: 
                g_CurrentSubMenu = SUB_SAVE_SELECT; 
                break;
            case 4: 
                SaveConfig(config, "Asset/config.ini");
                currentState = SCREEN_MENU;
                ResetPlayScreenStatics(); 
                break;
            }
            hasChanged = true;
        }
        return hasChanged;
    }

    if (state->status == MATCH_PLAYING) {
        if (g_AIsCalculating) return false; // Khóa bản đồ khi AI đang tính toán

        if (wParam == VK_ESCAPE) {
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }

        if (wParam == 'Q' || wParam == 'q') {
            undoMove(state);
            return true;
        }
        if (wParam == 'E' || wParam == 'e') {
            redoMove(state);
            return true;
        }

        if ((wParam == 'W' || wParam == VK_UP) && state->cursorRow > 0) { 
            state->cursorRow--; 
            hasChanged = true; 
        }
        if ((wParam == 'S' || wParam == VK_DOWN) && state->cursorRow < state->boardSize - 1) { 
            state->cursorRow++; 
            hasChanged = true; 
        }
        if ((wParam == 'A' || wParam == VK_LEFT) && state->cursorCol > 0) { 
            state->cursorCol--; 
            hasChanged = true; 
        }
        if ((wParam == 'D' || wParam == VK_RIGHT) && state->cursorCol < state->boardSize - 1) { 
            state->cursorCol++; 
            hasChanged = true; 
        }
        if (wParam == VK_RETURN || wParam == VK_SPACE) {
            if (processMove(state, state->cursorRow, state->cursorCol)) { 
                hasChanged = true; 
            }
        }
    }

    if (state->status == MATCH_FINISHED) {
        if (wParam == 'Y' || wParam == 'y') {
			// Reset về cấu hình trận đấu ban đầu để bắt đầu ván mới
			startNextRound(state);
            hasChanged = true;
        }
        else if (wParam == 'N' || wParam == 'n' || wParam == VK_ESCAPE) {
            currentState = SCREEN_MENU;
            hasChanged = true;
        }
    }

    return hasChanged;
}

void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const Sprite& spriteX, const Sprite& spriteO, const GameConfig* config) {
    // 1. Vẽ bàn cờ và thông tin trận đấu
    Gdiplus::Graphics g(hdc);
    
    // Nền sân vận động Procedural
    DrawProceduralStadium(g, screenWidth, screenHeight);
    
    // Lambda dịch chuỗi path sang hệ Avatar Matrix
    auto decodeAvatar = [](const std::string& path) -> int {
        if (path.find("avatar_1") != std::string::npos) return 0;
        if (path.find("avatar_2") != std::string::npos) return 1;
        if (path.find("bot_easy") != std::string::npos) return 2;
        if (path.find("bot_medium") != std::string::npos) return 3;
        if (path.find("bot_hard") != std::string::npos) return 4;
        return 0; 
    };

    // Tính toán kích thước ô cờ động (Responsive) dành diện tích cho 2 cột Profile (Ít nhất 150px mỗi bên)
    int availableHeight = screenHeight - 120; // Trừ hao khoảng trống Header trên cùng
    int availableWidth = screenWidth - 300;   // Trừ hao 2 lề Tab
    int maxBoardSize = availableWidth < availableHeight ? availableWidth : availableHeight;
    if (maxBoardSize < 200) maxBoardSize = 200; 
    
    int dynamicCellSize = maxBoardSize / state->boardSize;
    int boardPixelSize = state->boardSize * dynamicCellSize;

    if (dynamicCellSize != cachedCellSize) {
        PreScaleSprite(spriteX, g_CachedX, dynamicCellSize, dynamicCellSize);
        PreScaleSprite(spriteO, g_CachedO, dynamicCellSize, dynamicCellSize);
        cachedCellSize = dynamicCellSize;
    }

    int startX = (screenWidth - boardPixelSize) / 2;
    int startY = (screenHeight - boardPixelSize) / 2 + 40; // Đẩy xuống một xíu cho header
    
    HFONT hOldFont = (HFONT)SelectObject(hdc, GlobalFont::Default);
    SetBkMode(hdc, TRANSPARENT);

    // --- KHU VỰC TRÁI (PLAYER 1) ---
    Gdiplus::SolidBrush shadowBrush(GdipColour::SHADOW_HEAVY); // Khung den mo (Glassmorphism)
    int leftTabW = startX - 20;
    g.FillRectangle(&shadowBrush, 10, startY, leftTabW, boardPixelSize);

    // Animation Den pha Panel khi toi luot
    if (state->status == MATCH_PLAYING && state->isP1Turn) {
        int alpha = (int)(30 + sin(g_GlobalAnimTime * 8.0f) * 30.0f);
        Gdiplus::SolidBrush p1TurnPulse(GdipColour::WithAlpha(GdipColour::P1_TURN_PULSE, (BYTE)alpha));
        g.FillRectangle(&p1TurnPulse, 10, startY, leftTabW, boardPixelSize);

        Gdiplus::Pen p1Pen(GdipColour::P1_TURN_BORDER, 3.0f);
        g.DrawRectangle(&p1Pen, 10, startY, leftTabW, boardPixelSize);
    }

    int avaSize = 120;
    int avaX_L = 10 + (leftTabW - avaSize) / 2;
    std::wstring p1NameW = state->p1.name;

    // Draw P1 Watermark Background (Phong cách chữ chéo bị đè nghệ thuật)
    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font waterFont(&fontFamily, 64, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush waterBrushL(GdipColour::P1_WATERMARK);
    g.TranslateTransform((Gdiplus::REAL)(avaX_L + avaSize/2), (Gdiplus::REAL)(startY + 80));
    g.RotateTransform(-30.0f);
    Gdiplus::StringFormat alignCenter;
    alignCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
    alignCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(p1NameW.c_str(), -1, &waterFont, Gdiplus::PointF(0, 0), &alignCenter, &waterBrushL);
    g.ResetTransform();

    // Draw Procedural P1 Avatar
    DrawPixelAvatar(g, avaX_L, startY + 20, avaSize, decodeAvatar(state->p1.avatarPath));
    
    SetTextColor(hdc, Colour::ORANGE_NORMAL); // Chữ nổi bật
    SelectObject(hdc, GlobalFont::Bold);
    RECT textRectL1 = { 10, startY + avaSize + 35, 10 + leftTabW, startY + avaSize + 75 };
    DrawTextW(hdc, p1NameW.c_str(), -1, &textRectL1, DT_CENTER | DT_SINGLELINE);
    
    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, Colour::WHITE); // Chữ số liệu trắng sữa
    std::wstring p1Piece = L"Cầu thủ: X";
    RECT textRectL2 = { 10, startY + avaSize + 70, 10 + leftTabW, startY + avaSize + 110 };
    DrawTextW(hdc, p1Piece.c_str(), -1, &textRectL2, DT_CENTER | DT_SINGLELINE);

    std::wstring p1Wins = L"Bàn thắng: " + std::to_wstring(state->p1.totalWins);
    RECT textRectL3 = { 10, startY + avaSize + 100, 10 + leftTabW, startY + avaSize + 140 };
    DrawTextW(hdc, p1Wins.c_str(), -1, &textRectL3, DT_CENTER | DT_SINGLELINE);

    // --- KHU VỰC PHẢI (PLAYER 2) ---
    int rightTabStartX = startX + boardPixelSize + 10;
    int rightTabW = screenWidth - rightTabStartX - 10;
    g.FillRectangle(&shadowBrush, rightTabStartX, startY, rightTabW, boardPixelSize);

    // Animation Den pha Panel khi toi luot P2
    if (state->status == MATCH_PLAYING && !state->isP1Turn) {
        int alpha = (int)(30 + sin(g_GlobalAnimTime * 8.0f) * 30.0f);
        Gdiplus::SolidBrush p2TurnPulse(GdipColour::WithAlpha(GdipColour::P2_TURN_PULSE, (BYTE)alpha));
        g.FillRectangle(&p2TurnPulse, rightTabStartX, startY, rightTabW, boardPixelSize);

        Gdiplus::Pen p2Pen(GdipColour::P2_TURN_BORDER, 3.0f);
        g.DrawRectangle(&p2Pen, rightTabStartX, startY, rightTabW, boardPixelSize);
    }

    int avaX_R = rightTabStartX + (rightTabW - avaSize) / 2;
    std::wstring p2NameW = state->p2.name;

    // Draw P2 Watermark Background
    Gdiplus::SolidBrush waterBrushR(GdipColour::P2_WATERMARK);
    g.TranslateTransform((Gdiplus::REAL)(avaX_R + avaSize/2), (Gdiplus::REAL)(startY + 80));
    g.RotateTransform(30.0f);
    g.DrawString(p2NameW.c_str(), -1, &waterFont, Gdiplus::PointF(0, 0), &alignCenter, &waterBrushR);
    g.ResetTransform();

    // Draw Procedural P2 Avatar
    DrawPixelAvatar(g, avaX_R, startY + 20, avaSize, decodeAvatar(state->p2.avatarPath));

    SetTextColor(hdc, Colour::CYAN_NORMAL); // P2 chữ Cyan
    SelectObject(hdc, GlobalFont::Bold);
    RECT textRectR1 = { rightTabStartX, startY + avaSize + 35, rightTabStartX + rightTabW, startY + avaSize + 75 };
    DrawTextW(hdc, p2NameW.c_str(), -1, &textRectR1, DT_CENTER | DT_SINGLELINE);
    
    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, Colour::WHITE);
    std::wstring p2Piece = L"Cầu thủ: O";
    RECT textRectR2 = { rightTabStartX, startY + avaSize + 70, rightTabStartX + rightTabW, startY + avaSize + 110 };
    DrawTextW(hdc, p2Piece.c_str(), -1, &textRectR2, DT_CENTER | DT_SINGLELINE);

    std::wstring p2Wins = L"Bàn thắng: " + std::to_wstring(state->p2.totalWins);
    RECT textRectR3 = { rightTabStartX, startY + avaSize + 100, rightTabStartX + rightTabW, startY + avaSize + 140 };
    DrawTextW(hdc, p2Wins.c_str(), -1, &textRectR3, DT_CENTER | DT_SINGLELINE);

    // --- KHU VỰC BÀN CỜ (GIỮA) ---
    Gdiplus::SolidBrush pitchBrush(GdipColour::BOARD_PITCH);
    g.FillRectangle(&pitchBrush, startX, startY, boardPixelSize, boardPixelSize);

    Gdiplus::Pen pitchBorder(GdipColour::BOARD_BORDER, 3);
    g.DrawRectangle(&pitchBorder, startX, startY, boardPixelSize, boardPixelSize);
    
    DrawGameBoard(hdc, state, dynamicCellSize, startX, startY, g_CachedX, g_CachedO);

    // --- KHU VỰC HEADER (TRÊN CÙNG) ---
    std::wstring boText = L"THỂ THỨC: BO" + std::to_wstring(state->targetScore) + L" - LƯỢT: " + (state->isP1Turn ? p1NameW : p2NameW);
    COLORREF turnColor = state->isP1Turn ? Colour::RED_NORMAL : Colour::BLUE_NORMAL;
    DrawTextCentered(hdc, boText, 10, screenWidth, turnColor, GlobalFont::Bold);

    std::wstring timeText;
    if (state->matchType == MATCH_PVE) {
        int minutes = (int)state->matchDuration / 60;
        int seconds = (int)state->matchDuration % 60;
        wchar_t buffer[64];
        swprintf(buffer, 64, L"Thời gian đã chơi: %02d:%02d", minutes, seconds);
        timeText = buffer;
    } else {
        timeText = L"Thời gian lượt: " + std::to_wstring(state->timeRemaining) + L"s";
    }
    DrawTextCentered(hdc, timeText, 50, screenWidth, Colour::GRAY_DARK, GlobalFont::Default);
    
    // Ghi chú tính năng Đi Lại
    if (state->matchType == MATCH_PVE) {
        DrawTextCentered(hdc, L"Phím [Q]: Đi lại (Undo) | Phím [E]: Hoàn tác (Redo)", screenHeight - 40, screenWidth, Colour::GRAY_NORMAL, GlobalFont::Default);
    }
    
    SelectObject(hdc, hOldFont);

    // 2. Lớp phủ Pause Menu (Kính Đen Sa Bàn)
    if (state->status == MATCH_PAUSED) {
        Gdiplus::Graphics g(hdc);
        
        // Cần g_GlobalAnimTime để làm hiệu ứng nếu import từ UIComponents
        extern float g_GlobalAnimTime; 

        // Phủ mờ không gian (Shadow Overlay cho bàn cờ)
        Gdiplus::SolidBrush shadowBrush2(GdipColour::SHADOW_HEAVY);
        g.FillRectangle(&shadowBrush2, 0, 0, screenWidth, screenHeight);

        // Khung kinh den (Dark Glassmorphism)
        int menuW = 550, menuH = 550;
        int menuX = (screenWidth - menuW) / 2;
        int menuY = (screenHeight - menuH) / 2;
        Gdiplus::SolidBrush bgBrush(GdipColour::GLASS_DARK);
        g.FillRectangle(&bgBrush, menuX, menuY, menuW, menuH);

        Gdiplus::Pen glassGleam(GdipColour::GLASS_GLEAM, 2.0f);
        g.DrawRectangle(&glassGleam, menuX, menuY, menuW, menuH);

        if (g_CurrentSubMenu == SUB_MAIN) {
            // VẼ MENU CHÍNH
            DrawTextCentered(hdc, L"== HỘP TÁC CHIẾN ==", menuY + 40, screenWidth, Colour::CYAN_NORMAL, GlobalFont::Title);
            const wchar_t* labels[] = { L"Tiếp tục thi đấu", L"Nhạc nền: ", L"Âm lượng: ", L"Ghi hình trận", L"Rời phòng thay đồ" };
            for (int i = 0; i < TOTAL_PAUSE_ITEMS; i++) {
                std::wstring itemText = labels[i];
                // Thêm giá trị động cho các setting
                if (i == 1) itemText += (config->isBgmEnabled ? L"< BẬT >" : L"< TẮT >");
                if (i == 2) itemText += std::to_wstring(config->sfxVolume) + L"%";

                COLORREF color = Colour::GRAY_NORMAL;
                HFONT font = GlobalFont::Default;

                if (i == g_PauseSelected) {
                    int rCol = (int)(180 + sin(g_GlobalAnimTime * 12.0f) * 75);
                    color = RGB(255, max(0, min(255, 255 - rCol)), 0); 
                    font = GlobalFont::Bold;
                    itemText = L">> " + itemText + L" <<";
                }

                DrawTextCentered(hdc, itemText, menuY + 120 + i * 55, screenWidth, color, font);
            }
        }
        else if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            // VẼ MENU CHỌN SLOT LƯU
            DrawTextCentered(hdc, L"--- LƯU BĂNG GHI HÌNH ---", menuY + 40, screenWidth, Colour::ORANGE_NORMAL, GlobalFont::Title);

            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                bool exists = CheckSaveExists(i + 1);
                std::wstring slotLabel = L"Băng số " + std::to_wstring(i + 1) + (exists ? L"  [Đã Ghi]" : L"  [Trống]");

                COLORREF color = (i == g_SaveSlotSelected) ? Colour::YELLOW_NORMAL : Colour::GRAY_NORMAL;
                HFONT font = (i == g_SaveSlotSelected) ? GlobalFont::Bold : GlobalFont::Default;
                
                if (i == g_SaveSlotSelected) slotLabel = L"-> " + slotLabel + L" <-";
                DrawTextCentered(hdc, slotLabel, menuY + 130 + i * 55, screenWidth, color, font);
            }
            DrawTextCentered(hdc, L"[ ENTER ] Xác nhận lưu", menuY + 430, screenWidth, Colour::GREEN_NORMAL, GlobalFont::Default);
            DrawTextCentered(hdc, L"[ ESC ] Quay lại Menu", menuY + 460, screenWidth, Colour::GRAY_LIGHT, GlobalFont::Note);
        }
        else if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
            // GIAO DIỆN NHẬP TÊN
            DrawTextCentered(hdc, L"DÁN NHÃN CUỘN BĂNG", menuY + 60, screenWidth, Colour::CYAN_NORMAL, GlobalFont::Title);

            // Vẽ khung nhập liệu (Phá form Trắng làm nền Xám tối đi một chút)
            int boxW = 350, boxH = 50;
            int boxX = (screenWidth - boxW) / 2;
            int boxY = menuY + 200;

            HBRUSH hBoxBrush = CreateSolidBrush(RGB(40, 40, 50));
            RECT rectBox = { boxX, boxY, boxX + boxW, boxY + boxH };
            FillRect(hdc, &rectBox, hBoxBrush);
            DeleteObject(hBoxBrush);
            
            Gdiplus::Pen tbPen(GdipColour::SAVE_BOX_BORDER, 2.0f);
            g.DrawRectangle(&tbPen, boxX, boxY, boxW, boxH);

            // Hiển thị nội dung user đang gõ
            std::wstring displayText = g_SaveNameInput + L"_"; 
            DrawTextCentered(hdc, displayText.c_str(), boxY + 12, screenWidth, Colour::WHITE, GlobalFont::Bold);

            DrawTextCentered(hdc, L"Nhấn [ ENTER ] để Hoàn Vàng", menuY + 380, screenWidth, Colour::YELLOW_NORMAL, GlobalFont::Default);
            DrawTextCentered(hdc, L"Nhấn [ ESC ] để Hủy băng", menuY + 430, screenWidth, Colour::RED_NORMAL, GlobalFont::Note);
        }
    }
    else if (state->status == MATCH_FINISHED) {
        std::wstring winMsg;
        COLORREF winColor;
        COLORREF winGlow;

        if (state->winner == CELL_PLAYER1) { 
            winMsg = L"⚽ " + p1NameW + L" ĐÃ GHI BÀN QUYẾT ĐỊNH! ⚽"; 
            winColor = Colour::ORANGE_NORMAL;
            winGlow = RGB(255, 180, 0);
        }
        else if (state->winner == CELL_PLAYER2) { 
            winMsg = L"⚽ " + p2NameW + L" ĐÃ GHI BÀN QUYẾT ĐỊNH! ⚽"; 
            winColor = Colour::CYAN_NORMAL;
            winGlow = RGB(0, 220, 255);
        }
        else { 
            winMsg = L"⚽ TRẬN ĐẤU HÒA — KHÔNG AI GHI BÀN! ⚽";
            winColor = Colour::WHITE;
            winGlow = RGB(200, 200, 200);
        }

        // ---- BANNER THẮNG: chỉ chiếm vùng Header (0 -> startY), KHÔNG che bàn cờ ----
        int bannerH = startY; // Vùng header ngay trên bàn cờ
        
        // Nền banner mờ (glassmorphism tối)
        float bPulse = 0.5f + sin(g_WinAnimTime * 3.0f) * 0.5f;
        int bgAlpha = (int)(160 + bPulse * 60);
        Gdiplus::SolidBrush bannerBg(GdipColour::WithAlpha(GdipColour::BLACK, (BYTE)bgAlpha));
        g.FillRectangle(&bannerBg, 0, 0, screenWidth, bannerH);

        // Viền sáng pulse dưới banner
        int lineAlpha = (int)(150 + bPulse * 105);
        BYTE r_glow = GetRValue(winGlow), g_glow = GetGValue(winGlow), b_glow = GetBValue(winGlow);
        Gdiplus::Pen glowLine(Gdiplus::Color(lineAlpha, r_glow, g_glow, b_glow), 3.0f);
        g.DrawLine(&glowLine, 0, bannerH - 2, screenWidth, bannerH - 2);

        // Trái bóng nhỏ bay ngang qua Banner (fmod giúp loop)
        int ballX = -60 + (int)(fmod(g_WinAnimTime * 300.0f, (float)(screenWidth + 120)));
        int ballY = bannerH / 2 + (int)(sin(g_WinAnimTime * 6.0f) * 12.0f);
        DrawPixelFootball(g, ballX, ballY, 40);

        // Chữ thông báo thắng — căn giữa trong banner
        int msgY = bannerH / 2 - 20;
        DrawTextCentered(hdc, winMsg, msgY, screenWidth, winColor, GlobalFont::Title);

        // Chú thích nhỏ phía dưới chữ thắng
        DrawTextCentered(hdc, L"Nhấn 'Y' để Đá lại  |  'N' / ESC để Rút lui", msgY + 45, screenWidth, Colour::GRAY_LIGHT, GlobalFont::Note);
    }
}

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam, GameConfig* config) {
    if (wParam == 0) { 
        return; 
    }
    ProcessPlayInput(wParam, state, currentState, config);
}

void ResetPlayScreenStatics() {
    g_CurrentSubMenu = SUB_MAIN;
    g_PauseSelected = 0;
    g_SaveSlotSelected = 0;
    g_SaveNameInput = L"";
}