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
#include <iostream>
#include <string>
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <cmath>

extern bool UpdateCountdown(PlayState* state, double dt);

static std::future<std::pair<int, int>> g_AIFuture;
static bool g_AIsCalculating = false;

static int cachedCellSize = -1;
static Sprite g_CachedX = {nullptr, 0, 0};
static Sprite g_CachedO = {nullptr, 0, 0};
static float g_WinAnimTime = 0.0f;
static bool isEditingSaveName = false;

bool UpdatePlayLogic(PlayState* state, double dt) {
    bool needsRedraw = false;

    if (state->status == MATCH_FINISHED) {
        g_WinAnimTime += (float)dt;
        needsRedraw = true;
    } 
    else {
        g_WinAnimTime = 0.0f;
    }

    if (state->status == MATCH_PLAYING) {
        if (state->matchType == MATCH_PVE) {
            state->matchDuration += (float)dt;
            needsRedraw = true; // For timer display update
        } 
        else {
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
        } 
        else if (g_AIFuture.valid() && g_AIFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
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
            bool isChar = (wParam & 0x10000);
            wchar_t ch = (wchar_t)(wParam & 0xFFFF);

            if (isChar) {
                if (ch >= 32 && g_SaveNameInput.length() < 20) {
                    g_SaveNameInput += ch;
                }
                return true;
            }

            if (wParam == VK_ESCAPE) {
                g_CurrentSubMenu = SUB_SAVE_SELECT;
                return true;
            }
            if (wParam == VK_BACK) {
                if (!g_SaveNameInput.empty()) g_SaveNameInput.pop_back();
                return true;
            }
            if (wParam == VK_RETURN) {
                if (g_SaveNameInput.empty()) {
                    g_SaveNameInput = L"NEW_SAVE";
                }
                
                std::wstring customPath = L"Asset/save/" + g_SaveNameInput + L".bin";

                if (SaveMatchData(state, customPath)) {
                    PlaySFX(L"Asset/audio/success.wav");
                    g_CurrentSubMenu = SUB_MAIN; 
                    g_SaveNameInput = L"";
                }
                return true;
            }
            return true; // Chặn mọi phím khác (W,A,S,D...) không cho lọt xuống di chuyển menu
        }

        // TRƯỜNG HỢP 2: CHỌN SLOT LƯU
        if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            if (wParam == VK_ESCAPE) { 
                g_CurrentSubMenu = SUB_MAIN; 
                return true; 
            }
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
            // Bỏ qua mục Âm lượng (index 2) nếu Nhạc nền (index 1) đang OFF
            if (g_PauseSelected == 2 && !config->isBgmEnabled) {
                g_PauseSelected = (g_PauseSelected - 1 < 0) ? TOTAL_PAUSE_ITEMS - 1 : g_PauseSelected - 1;
            }
            hasChanged = true;
        }
        else if (wParam == 'S' || wParam == VK_DOWN) {
            g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            // Bỏ qua mục Âm lượng (index 2) nếu Nhạc nền (index 1) đang OFF
            if (g_PauseSelected == 2 && !config->isBgmEnabled) {
                g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            }
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
        if (g_AIsCalculating) {
            return false;
        }// Khóa bản đồ khi AI đang tính toán

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
            int winRequired = state->targetScore / 2 + 1;
            bool matchOver = (state->p1.totalWins >= winRequired || state->p2.totalWins >= winRequired);
            
            if (matchOver) {
                // Nếu đã thắng cả trận, reset điểm số để đá trận mới
                state->p1.totalWins = 0;
                state->p2.totalWins = 0;
            }
            
            // Bắt đầu ván đấu mới (hoặc ván đầu tiên của trận mới)
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
        if (path.find("avatar_1") != std::string::npos) {
            return 0;
        }
        if (path.find("avatar_2") != std::string::npos) {
            return 1;
        }
        if (path.find("bot_easy") != std::string::npos) {
            return 2;
        }
        if (path.find("bot_medium") != std::string::npos) {
            return 3;
        }
        if (path.find("bot_hard") != std::string::npos) {
            return 4;
        }
        return 0; 
    };

    // Tính toán kích thước ô cờ động (Responsive) dành diện tích cho 2 cột Profile (Ít nhất 150px mỗi bên)
    int availableHeight = screenHeight - 120; // Trừ hao khoảng trống Header trên cùng
    int availableWidth = screenWidth - 300;   // Trừ hao 2 lề Tab
    int maxBoardSize = availableWidth < availableHeight ? availableWidth : availableHeight;
    if (maxBoardSize < 200) {
        maxBoardSize = 200;
    }
    
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

    // THIẾT KẾ TIMER MỚI: Floating Minimalist (Không khung để tránh đè bàn cờ)
    int timerY = 55; 
    
    // Hiệu ứng rung lắc (Shake) khi thời gian sắp hết (<= 3s)
    int shakeX = 0, shakeY = 0;
    COLORREF timerColor = Colour::CYAN_LIGHT; 
    bool isWarning = (state->matchType != MATCH_PVE && state->timeRemaining <= 3);

    if (isWarning) {
        float shakeFreq = 25.0f;
        float shakeAmp = 4.0f;
        shakeX = (int)(sin(g_GlobalAnimTime * shakeFreq) * shakeAmp);
        shakeY = (int)(cos(g_GlobalAnimTime * shakeFreq * 0.7f) * shakeAmp);
        timerColor = Colour::RED_NORMAL; 
    }

    // Tính toán chiều ngang cụm Floating (Icon + Chữ) để căn giữa
    int clockSize = 34;
    std::wstring timeText;
    if (state->matchType == MATCH_PVE) {
        int minutes = (int)state->matchDuration / 60;
        int seconds = (int)state->matchDuration % 60;
        wchar_t buffer[64];
        swprintf(buffer, 64, L"ĐÃ CHƠI: %02d:%02d", minutes, seconds);
        timeText = buffer;
    } 
    else {
        timeText = L"THỜI GIAN: " + std::to_wstring(state->timeRemaining) + L"s";
    }

    // Ước lượng chiều rộng text để căn giữa cả cụm
    int textEstimatedW = (int)timeText.length() * 12 + 20;
    int groupTotalW = clockSize + 15 + textEstimatedW;
    int groupStartX = (screenWidth - groupTotalW) / 2;

    // 1. Vẽ biểu tượng Đồng hồ Pixel Art (Lơ lửng)
    float pulse = 0.7f + sin(g_GlobalAnimTime * 8.0f) * 0.3f;
    BYTE neonA = (BYTE)(150 + pulse * 105);
    Gdiplus::Color neonColor = isWarning ? Gdiplus::Color(neonA, 255, 0, 0) : Gdiplus::Color(neonA, 0, 200, 255);
    DrawPixelClock(g, groupStartX + shakeX + clockSize/2, timerY + shakeY + 28, clockSize, neonColor);

    // 2. In chữ kỹ thuật số (Lơ lửng)
    HFONT oldT = (HFONT)SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, timerColor);
    RECT rTimer = { groupStartX + shakeX + clockSize + 10, timerY + shakeY + 8, screenWidth, timerY + shakeY + 50 };
    DrawTextW(hdc, timeText.c_str(), -1, &rTimer, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, oldT);
    
    // Ghi chú tính năng Đi Lại
    if (state->matchType == MATCH_PVE) {
        DrawTextCentered(hdc, L"Phím [Q]: Đi lại (Undo) | Phím [E]: Hoàn tác (Redo)", screenHeight - 40, screenWidth, Colour::GRAY_NORMAL, GlobalFont::Default);
    }
    
    SelectObject(hdc, hOldFont);

    // 2. Lớp phủ Pause Menu (Phong cách Light Glassmorphism)
    if (state->status == MATCH_PAUSED) {
        Gdiplus::Graphics g(hdc);
        extern float g_GlobalAnimTime; 

        // 1. Phủ mờ không gian (Lớp kính trắng đục)
        Gdiplus::SolidBrush shadowBrush2(Gdiplus::Color(100, 255, 255, 255));
        g.FillRectangle(&shadowBrush2, 0, 0, screenWidth, screenHeight);

        // 2. Khung Pause Menu (Màu Vàng Neon Warning)
        int menuW = 580, menuH = 500;
        int menuX = (screenWidth - menuW) / 2;
        int menuY = (screenHeight - menuH) / 2;
        
        Gdiplus::SolidBrush bgBrush(GdipColour::GLASS_WHITE);
        g.FillRectangle(&bgBrush, menuX, menuY, menuW, menuH);

        Gdiplus::Pen yellowBorder(GdipColour::PANEL_YELLOW_BORDER, 4.0f);
        g.DrawRectangle(&yellowBorder, menuX, menuY, menuW, menuH);

        // 3. Watermark Còi mờ ẩn dưới nền
        static std::map<int, Gdiplus::Color> waterPalette;
        if (waterPalette.empty()) {
            waterPalette[1] = Gdiplus::Color(30, 0, 0, 0);   // Viền cực mờ
            waterPalette[2] = Gdiplus::Color(40, 255, 200, 0); // Màu vàng nhạt mờ
            waterPalette[3] = Gdiplus::Color(40, 255, 255, 255);
        }
        static PixelModel whistleMod = LoadPixelModel("Asset/models/whistle.txt");
        DrawPixelModel(g, whistleMod, screenWidth / 2, screenHeight / 2, 250, waterPalette);

        if (g_CurrentSubMenu == SUB_MAIN) {
            // Header: Thanh Banner Tạm dừng
            DrawPixelBanner(g, hdc, L"TẠM DỪNG", screenWidth / 2, menuY + 40,
                menuW - 20, Colour::WHITE, RGB(255, 180, 0), "Asset/models/whistle.txt");

            const wchar_t* labels[] = { L"TRỞ LẠI SÂN", L"NHẠC NỀN: ", L"ÂM LƯỢNG NHẠC", L"GHI HÌNH TRẬN ĐẤU", L"KẾT THÚC TRẬN ĐẤU" };
            for (int i = 0; i < TOTAL_PAUSE_ITEMS; i++) {
                std::wstring itemText = labels[i];
                COLORREF color = Colour::GRAY_DARKEST;
                HFONT font = GlobalFont::Default;
                bool isDisabled = (i == 2 && !config->isBgmEnabled);

                if (i == 1) {
                    // Hiển thị trạng thái Bật/Tắt bằng khối màu
                    itemText += (config->isBgmEnabled ? L" [ BẬT ]" : L" [ TẮT ]");
                }

                if (i == g_PauseSelected && !isDisabled) {
                    int wave = (int)(180 + sin(g_GlobalAnimTime * 10.0f) * 75);
                    color = RGB(255, wave, 0); 
                    font = GlobalFont::Bold;
                    if (i != 2) itemText = L">> " + itemText + L" <<";
                }

                if (isDisabled) {
                    color = RGB(150, 150, 150); 
                    font = GlobalFont::Default;
                    itemText = labels[i] + std::wstring(L" [ KHÓA ]");
                }

                int itemY = menuY + 140 + i * 55;
                DrawTextCentered(hdc, (i == 2) ? labels[i] : itemText, itemY, screenWidth, color, font);

                // Vẽ Slider cho mục Âm Lượng
                if (i == 2) {
                    int barW = 220;
                    int barH = 8;
                    int barX = (screenWidth - barW) / 2;
                    int barY = itemY + 38;

                    // Thanh nền (Mờ)
                    Gdiplus::SolidBrush bgSlider(Gdiplus::Color(60, 0, 0, 0));
                    g.FillRectangle(&bgSlider, barX, barY, barW, barH);

                    // Thanh giá trị (Phát sáng)
                    float percent = config->sfxVolume / 100.0f;
                    Gdiplus::Color activeColor = isDisabled ? Gdiplus::Color(100, 150, 150, 150) : Gdiplus::Color(255, 255, 200, 0);
                    Gdiplus::SolidBrush valBrush(activeColor);
                    g.FillRectangle(&valBrush, barX, barY, (int)(barW * percent), barH);

                    // Con trỏ Handle (Hình tròn)
                    int handleX = barX + (int)(barW * percent);
                    Gdiplus::Color hColor = isDisabled ? Gdiplus::Color(255, 100, 100, 100) : Gdiplus::Color(255, 255, 255, 255);
                    Gdiplus::SolidBrush hBrush(hColor);
                    g.FillEllipse(&hBrush, handleX - 8, barY - 4, 16, 16);

                    // Aura phát sáng cho con trỏ nếu đang chọn
                    if (i == g_PauseSelected && !isDisabled) {
                        Gdiplus::Pen aura(Gdiplus::Color(150, 255, 200, 0), 2);
                        g.DrawEllipse(&aura, handleX - 10, barY - 6, 20, 20);
                    }
                }
            }
        }
        else if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            DrawTextCentered(hdc, L"--- LƯU BĂNG GHI HÌNH ---", menuY + 40, screenWidth, Colour::ORANGE_NORMAL, GlobalFont::Title);

            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                bool exists = CheckSaveExists(i + 1);
                std::wstring slotLabel = L"Băng số " + std::to_wstring(i + 1) + (exists ? L"  [Đã Ghi]" : L"  [Trống]");

                COLORREF color = (i == g_SaveSlotSelected) ? Colour::BLUE_DARKEST : Colour::GRAY_DARKEST;
                HFONT font = (i == g_SaveSlotSelected) ? GlobalFont::Bold : GlobalFont::Default;
                
                if (i == g_SaveSlotSelected) {
                    slotLabel = L"-> " + slotLabel + L" <-";
                }
                DrawTextCentered(hdc, slotLabel, menuY + 130 + i * 55, screenWidth, color, font);
            }
            DrawTextCentered(hdc, L"[ ENTER ] Xác nhận lưu", menuY + 430, screenWidth, Colour::GREEN_NORMAL, GlobalFont::Default);
            DrawTextCentered(hdc, L"[ ESC ] Quay lại Menu", menuY + 460, screenWidth, Colour::GRAY_DARK, GlobalFont::Note);
        }
        else if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
            DrawTextCentered(hdc, L"DÁN NHÃN CUỘN BĂNG", menuY + 60, screenWidth, Colour::CYAN_NORMAL, GlobalFont::Title);

            int boxW = 350, boxH = 50;
            int boxX = (screenWidth - boxW) / 2;
            int boxY = menuY + 200;

            HBRUSH hBoxBrush = CreateSolidBrush(RGB(240, 240, 250));
            RECT rectBox = { boxX, boxY, boxX + boxW, boxY + boxH };
            FillRect(hdc, &rectBox, hBoxBrush);
            DeleteObject(hBoxBrush);
            
            Gdiplus::Pen tbPen(GdipColour::WithAlpha(GdipColour::TITLE_BORDER, 180), 2.0f);
            g.DrawRectangle(&tbPen, boxX, boxY, boxW, boxH);

            std::wstring displayText = L"Tên: " + g_SaveNameInput + L"_"; 
            DrawTextCentered(hdc, displayText.c_str(), boxY + 12, screenWidth, Colour::GRAY_DARKEST, GlobalFont::Bold);

            DrawTextCentered(hdc, L"Gõ Tên Băng Ghi Hình Trực Tiếp (Hỗ Trợ Dấu)", menuY + 380, screenWidth, Colour::GREEN_NORMAL, GlobalFont::Default);
            DrawTextCentered(hdc, L"Nhấn [ ENTER ] để Lưu  |  [ ESC ] để Hủy", menuY + 430, screenWidth, Colour::RED_NORMAL, GlobalFont::Note);
        }
    }
    else if (state->status == MATCH_FINISHED) {
        std::wstring winMsg;
        COLORREF winColor;
        COLORREF winGlow;

        int winRequired = state->targetScore / 2 + 1;
        bool matchOver = (state->p1.totalWins >= winRequired || state->p2.totalWins >= winRequired);

        if (state->winner == CELL_PLAYER1) { 
            winMsg = matchOver ? (L"🏆 " + p1NameW + L" ĐÃ GIÀNH CUP VÔ ĐỊCH! 🏆") : (L"⚽ " + p1NameW + L" ĐÃ GHI BÀN THẮNG QUYẾT ĐỊNH! ⚽");
            winColor = Colour::ORANGE_NORMAL;
            winGlow = RGB(255, 120, 0);
        }
        else if (state->winner == CELL_PLAYER2) { 
            winMsg = matchOver ? (L"🏆 " + p2NameW + L" ĐÃ GIÀNH CUP VÔ ĐỊCH! 🏆") : (L"⚽ " + p2NameW + L" ĐÃ GHI BÀN THẮNG QUYẾT ĐỊNH! ⚽");
            winColor = Colour::CYAN_NORMAL;
            winGlow = RGB(0, 150, 255);
        }
        else { 
            winMsg = L"⚽ TRẬN ĐẤU HÒA — KHÔNG AI GHI BÀN! ⚽";
            winColor = Colour::GRAY_DARK;
            winGlow = RGB(150, 150, 150);
        }

        int bannerH = startY; 
        
        float bPulse = 0.5f + sin(g_WinAnimTime * 3.0f) * 0.5f;
        int bgAlpha = (int)(180 + bPulse * 40); 
        Gdiplus::SolidBrush bannerBg(Gdiplus::Color(bgAlpha, 255, 255, 255));
        g.FillRectangle(&bannerBg, 0, 0, screenWidth, bannerH);

        int lineAlpha = (int)(200 + bPulse * 55);
        BYTE r_glow = GetRValue(winGlow), g_glow = GetGValue(winGlow), b_glow = GetBValue(winGlow);
        Gdiplus::Pen glowLine(Gdiplus::Color(lineAlpha, r_glow, g_glow, b_glow), 4.0f);
        g.DrawLine(&glowLine, 0, bannerH - 2, screenWidth, bannerH - 2);

        int ballX = -60 + (int)(fmod(g_WinAnimTime * 300.0f, (float)(screenWidth + 120)));
        int ballY = bannerH / 2 + (int)(sin(g_WinAnimTime * 6.0f) * 12.0f);
        DrawPixelFootball(g, ballX, ballY, 40);

        int msgY = bannerH / 2 - 40;
        DrawTextCentered(hdc, winMsg, msgY, screenWidth, winColor, GlobalFont::Title);

        std::wstring hintText = matchOver ? L"Nhấn phím 'Y' để đá trận mới  |  ESC để Rút lui" : L"Nhấn phím 'Y' để đá ván tiếp theo  |  ESC để Rút lui";
        DrawTextCentered(hdc, hintText, msgY + 52, screenWidth, Colour::GRAY_DARK, GlobalFont::Note);
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