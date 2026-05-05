#include "PlayScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../GameLogic/GameEngine.h"
#include "../GameLogic/BotAI.h"
#include "../SystemModules/AudioSystem.h"
#include "../SystemModules/SaveLoadSystem.h"
#include "../SystemModules/ConfigLoader.h"
#include "../SystemModules/Localization.h"
#include "../ApplicationTypes/GameConstants.h"
#include <future>
#include <cmath>
#include <iostream>
#include <string>
#include <windows.h>
#include <fcntl.h>
#include <io.h>

extern bool updateCountdown(PlayState *state, double dt);

/** @file PlayScreen.cpp
 *  @brief Màn chơi chính: cập nhật logic trận đấu, xử lý input và hiển thị trạng thái trận.
 *
 *  Chức năng chính:
 *  - `UpdatePlayLogic`: cập nhật trạng thái trận theo delta-time (AI, timer, animations).
 *  - `ProcessPlayInput`: xử lý phím trong các trạng thái (playing, paused, summary, etc.).
 */

static std::future<std::pair<int, int>> g_AIFuture;
static bool g_AIsCalculating = false;
static float g_WinAnimTime = 0.0f;
static bool isEditingSaveName = false;
static MatchStatus g_PrePauseStatus = MATCH_PLAYING;
static int g_SummarySelected = 0;
static float g_SaveFeedbackTimer = 0.0f;
static std::wstring g_SaveStatusMsg = L"";

/** @brief Cập nhật logic chơi mỗi khung thời gian.
 *  @param state Trạng thái trận đấu hiện tại (được cập nhật bởi hàm).
 *  @param dt Khoảng thời gian (giây) kể từ lần cập nhật trước.
 *  @return `true` nếu cần vẽ lại màn hình (có thay đổi hiển thị), ngược lại `false`.
 *  @note Hàm xử lý: thời gian trận, thời gian nắm bóng, kích hoạt AI (async), xử lý timeout và animation thắng.
 */
bool UpdatePlayLogic(PlayState *state, double dt)
{
    bool needsRedraw = false;

    if (state->status == MATCH_FINISHED)
    {
        g_WinAnimTime += (float)dt;
        needsRedraw = true;
    }
    else
    {
        g_WinAnimTime = 0.0f;
    }

    if (state->status == MATCH_PLAYING)
    {
        state->matchDuration += (float)dt; // Cập nhật cho mọi chế độ chơi

        // Cộng dồn thời gian giữ bóng theo lượt
        if (state->isPlayer1Turn)
        {
            state->player1.totalTimePossessed += (float)dt;
        }
        else
        {
            state->player2.totalTimePossessed += (float)dt;
        }

        if (state->matchType == MATCH_PVE)
        {
            needsRedraw = true; // For timer display update
        }
        else
        {
            // Nếu một giây trôi qua, updateCountdown trả về true
            if (updateCountdown(state, dt))
            {
                needsRedraw = true;

                // Nếu sau khi trừ giây mà thời gian bằng 0 -> Hết lượt, đổi người chơi
                if (state->timeRemaining <= 0)
                {
                    playSfx("sfx_timeout");
                    switchTurn(state);
                }
            }
        }
    }

    if (state->status == MATCH_PLAYING && state->matchType == MATCH_PVE && !state->isPlayer1Turn)
    {
        if (!g_AIsCalculating)
        {
            g_AIsCalculating = true;
            PlayState localState = *state;
            g_AIFuture = std::async(std::launch::async, [localState]()
                                    {
                int r, c;
                    calculateComputerMove(&localState, localState.difficulty, r, c);
                return std::make_pair(r, c); });
        }
        else if (g_AIFuture.valid() && g_AIFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            auto bestMove = g_AIFuture.get();
            g_AIsCalculating = false;
            if (processMove(state, bestMove.first, bestMove.second))
            {
                playSfx("sfx_place");
                needsRedraw = true;
            }
        }
    }

    if (g_SaveFeedbackTimer > 0.0f)
    {
        g_SaveFeedbackTimer -= (float)dt;
        if (g_SaveFeedbackTimer <= 0.0f)
        {
            g_SaveFeedbackTimer = 0.0f;
            g_SaveStatusMsg = L"";
            // Nếu lưu xong và hết thời gian hiển thị, quay lại menu chính của pause
            if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY)
            {
                g_CurrentSubMenu = SUB_MAIN;
            }
        }
        needsRedraw = true;
    }

    return needsRedraw;
}

/** @brief Xử lý sự kiện phím trong màn chơi.
 *  @param wParam Mã phím/flags (WM_KEY/WM_CHAR encoded).
 *  @param state Trạng thái trận đấu để đọc/ghi.
 *  @param currentState Tham chiếu trạng thái màn hình (có thể chuyển màn hình từ hàm này).
 *  @param config Cấu hình game (bật/tắt SFX/BGM, âm lượng...).
 *  @return `true` nếu trạng thái đã thay đổi và cần cập nhật UI; `false` nếu không.
 *  @note Hàm xử lý nhiều ngữ cảnh: nhập tên lưu, chọn slot lưu, menu pause, playing, finished, summary.
 */
bool ProcessPlayInput(WPARAM wParam, PlayState *state, ScreenState &currentState, GameConfig *config)
{
    bool hasChanged = false;

    // Phát hiện nút giữ (Autorepeat) từ bit 0x20000 mà main.cpp đã encode
    bool isRepeat = (wParam & 0x20000) != 0;
    bool isCharMsg = (wParam & 0x10000) != 0;
    WPARAM rawKey = wParam & 0xFFFF; // Lấy mã phím thực tế

    // Cơ chế Throttling: Giới hạn tốc độ di chuyển khi nhấn giữ
    static ULONGLONG lastMoveTime = 0;
    ULONGLONG now = GetTickCount64();

    // Nếu là nhấn giữ, chỉ cho phép thực hiện 10 lần mỗi giây (100ms/vước di chuyển)
    // Nếu là nhấn lần đầu (isRepeat=false), cho phép chạy ngay lập tức
    bool canMove = (now - lastMoveTime > (ULONGLONG)(isRepeat ? 100 : 80));

    if (isCharMsg && !(state->status == MATCH_PAUSED && g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY))
    {
        return false; // Only handle WM_CHAR in save-name entry
    }

    if (state->status == MATCH_PAUSED)
    {
        // Trường hợp: đang nhập tên lưu (SUB_SAVE_NAME_ENTRY)
        if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY)
        {
            bool isChar = (wParam & 0x10000);
            wchar_t ch = (wchar_t)(wParam & 0xFFFF);

            if (isChar)
            {
                // Giới hạn số ký tự theo cấu hình, 0-9, a-z, A-Z, Space và Vietnamese (qua WM_CHAR)
                if (g_SaveNameInput.length() < MAX_SAVE_NAME_LEN)
                {
                    // Cho phép các ký tự in được và có nghĩa
                    if (ch >= 32)
                    {
                        g_SaveNameInput += ch;
                    }
                }
                return true;
            }

            if (wParam == VK_ESCAPE)
            {
                g_CurrentSubMenu = SUB_SAVE_SELECT;
                return true;
            }
            if (wParam == VK_BACK)
            {
                if (!g_SaveNameInput.empty())
                    g_SaveNameInput.pop_back();
                return true;
            }
            if (wParam == VK_RETURN)
            {
                if (g_SaveNameInput.empty())
                {
                    g_SaveNameInput = L"NewGame";
                }

                // Thực hiện lưu vào đúng slot đã chọn (slot_1 đến slot_5)
                std::wstring path = GetSavePath(g_SaveSlotSelected + 1);
                state->saveName = g_SaveNameInput;

                if (SaveMatchData(state, path))
                {
                    playSfx("sfx_success");
                    g_SaveStatusMsg = GetText("msg_save_success");
                    g_SaveFeedbackTimer = 1.5f; // Hiện thông báo trong 1.5s
                }
                else
                {
                    playSfx("sfx_error");
                    g_SaveStatusMsg = GetText("msg_save_error");
                    g_SaveFeedbackTimer = 1.5f;
                }
                return true;
            }
            return true;
        }

        // Trường hợp: chọn slot lưu (SUB_SAVE_SELECT)
        if (g_CurrentSubMenu == SUB_SAVE_SELECT)
        {
            if (wParam == VK_ESCAPE)
            {
                g_CurrentSubMenu = SUB_MAIN;
                return true;
            }
            if (wParam == 'W' || wParam == VK_UP)
            {
                if (!canMove)
                    return false;
                g_SaveSlotSelected = (g_SaveSlotSelected - 1 < 0) ? MAX_SAVE_SLOTS - 1 : g_SaveSlotSelected - 1;
                if (!isRepeat)
                    playSfx("sfx_move");
                lastMoveTime = now;
                return true;
            }
            if (wParam == 'S' || wParam == VK_DOWN)
            {
                if (!canMove)
                    return false;
                g_SaveSlotSelected = (g_SaveSlotSelected + 1 >= MAX_SAVE_SLOTS) ? 0 : g_SaveSlotSelected + 1;
                if (!isRepeat)
                    playSfx("sfx_move");
                lastMoveTime = now;
                return true;
            }
            if (wParam == VK_RETURN || wParam == VK_SPACE)
            {
                playSfx("sfx_select");
                g_CurrentSubMenu = SUB_SAVE_NAME_ENTRY;
                g_SaveNameInput = L"";
                return true;
            }
            return false;
        }

        // Trường hợp: menu pause chính
        if (wParam == VK_ESCAPE)
        {
            state->status = g_PrePauseStatus;
            return true;
        }

        if (rawKey == 'W' || rawKey == VK_UP)
        {
            if (!canMove)
                return false;
            g_PauseSelected = (g_PauseSelected - 1 < 0) ? TOTAL_PAUSE_ITEMS - 1 : g_PauseSelected - 1;
            // Bỏ qua mục Âm lượng (index 2) nếu SFX (index 1) đang OFF
            if (g_PauseSelected == 2 && !config->isSfxEnabled)
            {
                g_PauseSelected = (g_PauseSelected - 1 < 0) ? TOTAL_PAUSE_ITEMS - 1 : g_PauseSelected - 1;
            }
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        else if (rawKey == 'S' || rawKey == VK_DOWN)
        {
            if (!canMove)
                return false;
            g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            // Bỏ qua mục Âm lượng (index 2) nếu SFX (index 1) đang OFF
            if (g_PauseSelected == 2 && !config->isSfxEnabled)
            {
                g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            }
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        else if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_RIGHT || wParam == VK_LEFT)
        {
            int dir = (wParam == VK_LEFT) ? -1 : 1;
            switch (g_PauseSelected)
            {
            case 0:
                state->status = g_PrePauseStatus;
                break;
            case 1:
                config->isSfxEnabled = !config->isSfxEnabled;
                break;
            case 2:
                config->sfxVolume += dir * 10;
                if (config->sfxVolume > 100)
                {
                    config->sfxVolume = 100;
                }
                if (config->sfxVolume < 0)
                {
                    config->sfxVolume = 0;
                }
                break;
            case 3:
                g_CurrentSubMenu = SUB_SAVE_SELECT;
                break;
            case 4:
                SaveConfig(config, "Asset/config.ini");
                currentState = SCREEN_MENU;
                if (config->isBgmEnabled)
                    playBgm("Asset/audio/c1.mp3");
                ResetPlayScreenStatics();
                break;
            }
            if (g_PauseSelected != 3)
            {
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            else
                playSfx("sfx_select");
            hasChanged = true;
        }
        return hasChanged;
    }

    if (state->status == MATCH_PLAYING)
    {
        if (g_AIsCalculating)
        {
            return false;
        } // Khóa bản đồ khi AI đang tính toán

        if (wParam == VK_ESCAPE)
        {
            playSfx("sfx_select");
            g_PrePauseStatus = state->status;
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }

        if (wParam == 'Q' || wParam == 'q')
        {
            undoMove(state);
            if (!isRepeat)
                playSfx("sfx_move");
            return true;
        }
        if (wParam == 'E' || wParam == 'e')
        {
            redoMove(state);
            if (!isRepeat)
                playSfx("sfx_move");
            return true;
        }

        if ((rawKey == 'W' || rawKey == VK_UP) && state->cursorRow > 0)
        {
            if (!canMove)
                return false;
            state->cursorRow--;
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        if ((rawKey == 'S' || rawKey == VK_DOWN) && state->cursorRow < state->boardSize - 1)
        {
            if (!canMove)
                return false;
            state->cursorRow++;
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        if ((rawKey == 'A' || rawKey == VK_LEFT) && state->cursorCol > 0)
        {
            if (!canMove)
                return false;
            state->cursorCol--;
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        if ((rawKey == 'D' || rawKey == VK_RIGHT) && state->cursorCol < state->boardSize - 1)
        {
            if (!canMove)
                return false;
            state->cursorCol++;
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        if (wParam == VK_RETURN || wParam == VK_SPACE)
        {
            if (processMove(state, state->cursorRow, state->cursorCol))
            {
                playSfx("sfx_place");
                return true;
            }
            else
            {
                playSfx("sfx_error");
            }
        }
    }
    else if (state->status == MATCH_FINISHED)
    {
        if (g_WinAnimTime < 0.5f)
        {
            return false; // Ignore input for 0.5 seconds so player can see the board
        }

        if (wParam == VK_ESCAPE)
        {
            playSfx("sfx_select");
            g_PrePauseStatus = state->status;
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }
        else
        {
            state->status = MATCH_SUMMARY;
            g_SummarySelected = 0;
            return true;
        }
    }
    else if (state->status == MATCH_SUMMARY)
    {
        if (wParam == VK_ESCAPE)
        {
            playSfx("sfx_select");
            g_PrePauseStatus = state->status;
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }

        if (wParam == 'W' || wParam == 'w' || wParam == VK_UP)
        {
            if (!canMove)
                return false;
            g_SummarySelected = (g_SummarySelected - 1 < 0) ? 2 : g_SummarySelected - 1;
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        else if (wParam == 'S' || wParam == 's' || wParam == VK_DOWN)
        {
            if (!canMove)
                return false;
            g_SummarySelected = (g_SummarySelected + 1 > 2) ? 0 : g_SummarySelected + 1;
            if (!isRepeat)
                playSfx("sfx_move");
            lastMoveTime = now;
            hasChanged = true;
        }
        else if (wParam == VK_RETURN || wParam == VK_SPACE)
        {
            stopSfx("sfx_crowd");
            stopSfx("sfx_whistle");
            stopSfx("sfx_siu");

            if (g_SummarySelected == 0)
            {
                int winRequired = state->targetScore / 2 + 1;
                bool matchOver = (state->player1.totalWins >= winRequired || state->player2.totalWins >= winRequired);
                if (matchOver)
                {
                    state->player1.totalWins = 0;
                    state->player2.totalWins = 0;
                }
                startNextRound(state);
                playSfx("sfx_whistle");
            }
            else if (g_SummarySelected == 1)
            {
                playSfx("sfx_select");
                g_PrePauseStatus = state->status;
                state->status = MATCH_PAUSED;
                g_CurrentSubMenu = SUB_SAVE_SELECT;
            }
            else if (g_SummarySelected == 2)
            {
                playSfx("sfx_select");
                currentState = SCREEN_MENU;
                if (config->isBgmEnabled)
                    playBgm("Asset/audio/c1.mp3");
            }
            hasChanged = true;
        }
    }

    return hasChanged;
}

void RenderPlayScreen(HDC hdc, const PlayState *state, int screenWidth, int screenHeight, const GameConfig *config)
{
    // 1. Vẽ bàn cờ và thông tin trận đấu
    Gdiplus::Graphics g(hdc);

    // Nền sân vận động Procedural
    DrawProceduralStadium(g, screenWidth, screenHeight, false, false);

    // Lambda dịch chuỗi path sang hệ Avatar Matrix (hỗ trợ 6 avatar 0-5)
    auto decodeAvatar = [](const std::string &path) -> int
    {
        if (path.find("avatar_") != std::string::npos)
        {
            try
            {
                return std::stoi(path.substr(7));
            }
            catch (...)
            {
                return 0;
            }
        }
        if (path.find("bot_easy") != std::string::npos)
            return 0;
        if (path.find("bot_medium") != std::string::npos)
            return 0;
        if (path.find("bot_hard") != std::string::npos)
            return 0;
        return 0;
    };

    if (state->status == MATCH_SUMMARY)
    {
        Gdiplus::SolidBrush shadowBrush2(Gdiplus::Color(210, 245, 250, 255)); // Kính trong suốt màu trắng/xanh nhạt
        g.FillRectangle(&shadowBrush2, 0, 0, screenWidth, screenHeight);

        DrawPixelBanner(g, hdc, GetText("summary_title").c_str(), screenWidth / 2, UIScaler::SY(60), UIScaler::SX(600), ToCOLORREF(Palette::White), RGB(0, 100, 255), "Asset/models/bg/football.txt");

        int panelW = UIScaler::SX(350);
        int panelH = UIScaler::SY(450);
        int p1X = screenWidth / 2 - panelW - UIScaler::SX(40);
        int p2X = screenWidth / 2 + UIScaler::SX(40);
        int panelY = UIScaler::SY(120);

        auto drawSummaryPanel = [&](int x, const PlayerMatchInfo &playerInfo, bool isWinner, bool flipModel)
        {
            Gdiplus::SolidBrush bgBrush(isWinner ? ToGdiColor(WithAlpha(Palette::YellowLight, 235)) : ToGdiColor(WithAlpha(Palette::GrayLightest, 235)));
            Gdiplus::Pen border(isWinner ? ToGdiColor(Palette::OrangeNormal) : ToGdiColor(Palette::GrayNormal), 4.0f);
            g.FillRectangle(&bgBrush, x, panelY, panelW, panelH);
            g.DrawRectangle(&border, x, panelY, panelW, panelH);

            if (isWinner)
            {
                // Hiệu ứng pháo hoa giấy (Confetti) màu vàng
                for (int i = 0; i < 25; i++)
                {
                    int px = x + (i * 1234) % panelW;
                    int py = panelY + (int)fmod(g_GlobalAnimTime * 180.0f + i * 37, (float)panelH);
                    Gdiplus::SolidBrush confB(Gdiplus::Color(150, 255, 200, 0));
                    g.FillRectangle(&confB, px, py, UIScaler::S(4), UIScaler::S(4));
                }
                // Ribbon "CHIẾN THẮNG"
                Gdiplus::SolidBrush ribbonBrush(ToGdiColor(Palette::OrangeNormal));
                g.FillRectangle(&ribbonBrush, x - UIScaler::SX(10), panelY + UIScaler::SY(15), panelW + UIScaler::SX(20), UIScaler::SY(35));
                SetTextColor(hdc, ToCOLORREF(Palette::White));
                SelectObject(hdc, GlobalFont::Bold);
                RECT rRibbon = {x, panelY + UIScaler::SY(17), x + panelW, panelY + UIScaler::SY(50)};
                DrawTextW(hdc, (L"== " + GetText("summary_win") + L" ==").c_str(), -1, &rRibbon, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            }

            // Ten nguoi choi
            std::wstring title = playerInfo.name;
            SetTextColor(hdc, isWinner ? ToCOLORREF(Palette::RedDarkest) : ToCOLORREF(Palette::GrayDarkest));
            SelectObject(hdc, GlobalFont::Bold);
            RECT rTitle = {x, panelY + (isWinner ? UIScaler::SY(55) : UIScaler::SY(15)), x + panelW, panelY + (isWinner ? UIScaler::SY(95) : UIScaler::SY(55))};
            DrawTextW(hdc, title.c_str(), -1, &rTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            // Avatar & Trophy
            PlayerState pState;
            pState.avatarType = decodeAvatar(playerInfo.avatarPath);
            pState.flipH = flipModel;
            pState.currentAction = isWinner ? "win" : "sad";
            if (state->winner == 0)
                pState.currentAction = "idle";
            pState.currentFrame = (int)(g_GlobalAnimTime * 8.0f) % 4;

            int avaSize = UIScaler::S(140);
            int avaY = panelY + (isWinner ? UIScaler::SY(165) : UIScaler::SY(135));
            DrawPixelAction(g, x + panelW / 2, avaY, avaSize, pState);

            if (isWinner)
            {
                DrawPixelTrophy(g, x + panelW - UIScaler::SX(50), avaY - UIScaler::SY(40), UIScaler::S(65));
            }

            // --- Stats với Icon ---
            SelectObject(hdc, GlobalFont::Default);
            int sY = panelY + UIScaler::SY(265);
            int sH = UIScaler::SY(35);
            int iconX = x + UIScaler::SX(40);
            int textX = iconX + UIScaler::SX(45);

            auto DrawStatRow = [&](const std::wstring &label, int val, Gdiplus::Color color, int iconType)
            {
                if (iconType == 1)
                    DrawPixelFootball(g, iconX, sY + sH / 2, UIScaler::S(24));
                else if (iconType == 2)
                    DrawPixelClock(g, iconX, sY + sH / 2, UIScaler::S(24), color);

                SetTextColor(hdc, ToCOLORREF(SmartColor{color.GetA(), color.GetR(), color.GetG(), color.GetB()}));
                RECT r = {textX, sY, x + panelW - UIScaler::SX(20), sY + sH};

                std::wstring fullStr = label + L": " + std::to_wstring(val);
                if (iconType == 2)
                {
                    int mins = val / 60, secs = val % 60;
                    wchar_t buf[32];
                    swprintf(buf, 32, L"%s: %02d:%02d", label.c_str(), mins, secs);
                    fullStr = buf;
                }
                DrawTextW(hdc, fullStr.c_str(), -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
                sY += sH;
            };

            DrawStatRow(GetText("summary_dribble"), playerInfo.movesCount, ToGdiColor(Palette::BlueDark), 1);
            DrawStatRow(GetText("summary_poss"), (int)playerInfo.totalTimePossessed, ToGdiColor(Palette::GreenDark), 2);
            DrawStatRow(GetText("summary_goal"), playerInfo.totalWins, ToGdiColor(Palette::OrangeDarkest), 0);
            DrawStatRow(GetText("summary_concede"), (&playerInfo == &state->player1 ? state->player2.totalWins : state->player1.totalWins), ToGdiColor(Palette::RedNormal), 0);
            DrawStatRow(GetText("summary_bowin"), playerInfo.matchWins, ToGdiColor(Palette::PurpleDark), 0);
        };

        bool p1Win = (state->winner == CELL_PLAYER1);
        bool p2Win = (state->winner == CELL_PLAYER2);

        drawSummaryPanel(p1X, state->player1, p1Win, false);
        drawSummaryPanel(p2X, state->player2, p2Win, true);

        int minutes = (int)state->matchDuration / 60;
        int seconds = (int)state->matchDuration % 60;
        wchar_t timeBuffer[64];
        swprintf(timeBuffer, 64, L"%02d:%02d", minutes, seconds);
        std::wstring totalTimeStr = GetText("play_time") + timeBuffer;
        DrawTextCentered(hdc, totalTimeStr, panelY + panelH + UIScaler::SY(30), screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold);

        std::wstring options[] = {GetText("summary_continue"), GetText("summary_save"), GetText("summary_exit")};
        int optY = panelY + panelH + UIScaler::SY(65);
        int btnW = UIScaler::SX(220);
        int btnH = UIScaler::SY(45);

        for (int i = 0; i < 3; i++)
        {
            bool isSelected = (i == g_SummarySelected);
            int bY = optY + i * UIScaler::SY(52);

            if (isSelected)
            {
                Gdiplus::SolidBrush btnBg(Gdiplus::Color(180, 255, 60, 0));
                g.FillRectangle(&btnBg, screenWidth / 2 - btnW / 2, bY, btnW, btnH);
                Gdiplus::Pen btnPen(ToGdiColor(Palette::White), 2.0f);
                g.DrawRectangle(&btnPen, screenWidth / 2 - btnW / 2, bY, btnW, btnH);
            }

            COLORREF color = isSelected ? ToCOLORREF(Palette::White) : ToCOLORREF(Palette::GrayDarkest);
            HFONT font = isSelected ? GlobalFont::Bold : GlobalFont::Default;
            DrawTextCentered(hdc, options[i], bY + UIScaler::SY(6), screenWidth, color, font);
        }
        return;
    }

    int availableHeight = screenHeight - UIScaler::SY(120);
    int availableWidth = screenWidth - UIScaler::SX(300);
    int maxBoardSize = availableWidth < availableHeight ? availableWidth : availableHeight;
    int minBoardSize = UIScaler::S(200);
    if (maxBoardSize < minBoardSize)
        maxBoardSize = minBoardSize;

    int dynamicCellSize = maxBoardSize / state->boardSize;
    int boardPixelSize = state->boardSize * dynamicCellSize;

    int startX = (screenWidth - boardPixelSize) / 2;
    int startY = (screenHeight - boardPixelSize) / 2 + UIScaler::SY(40);

    HFONT hOldFont = (HFONT)SelectObject(hdc, GlobalFont::Default);
    SetBkMode(hdc, TRANSPARENT);

    Gdiplus::SolidBrush shadowBrush(ToGdiColor(Theme::ShadowHeavy));
    int leftTabW = startX - UIScaler::SX(20);
    int tabMarginX = UIScaler::SX(10);
    g.FillRectangle(&shadowBrush, tabMarginX, startY, leftTabW, boardPixelSize);

    if (state->status == MATCH_PLAYING && state->isPlayer1Turn)
    {
        int alpha = (int)(30 + sin(g_GlobalAnimTime * 8.0f) * 30.0f);
        Gdiplus::SolidBrush *p1TurnPulse = GetCachedBrush(ToGdiColor(WithAlpha(Theme::P1TurnPulse, (BYTE)alpha)));
        g.FillRectangle(p1TurnPulse, tabMarginX, startY, leftTabW, boardPixelSize);
        Gdiplus::Pen p1Pen(ToGdiColor(Theme::P1TurnBorder), 3.0f);
        g.DrawRectangle(&p1Pen, tabMarginX, startY, leftTabW, boardPixelSize);
    }

    int avaSize = UIScaler::S(180);
    int avaX_L = tabMarginX + (leftTabW - avaSize) / 2;
    std::wstring player1NameW = state->player1.name;

    static Gdiplus::FontFamily s_fontFamily(L"Arial");
    static Gdiplus::Font s_waterFont(&s_fontFamily, 64, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    static Gdiplus::StringFormat s_alignCenter;
    static bool s_formatInit = false;
    if (!s_formatInit)
    {
        s_alignCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
        s_alignCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);
        s_formatInit = true;
    }

    Gdiplus::SolidBrush *waterBrushL = GetCachedBrush(ToGdiColor(Theme::P1Watermark));
    g.TranslateTransform((Gdiplus::REAL)(avaX_L + avaSize / 2), (Gdiplus::REAL)(startY + UIScaler::SY(80)));
    g.RotateTransform(-30.0f);
    g.DrawString(player1NameW.c_str(), -1, &s_waterFont, Gdiplus::PointF(0, 0), &s_alignCenter, waterBrushL);
    g.ResetTransform();

    DrawPixelAvatar(g, avaX_L, startY + UIScaler::SY(20), avaSize, decodeAvatar(state->player1.avatarPath));

    SetTextColor(hdc, ToCOLORREF(Palette::OrangeNormal));
    SelectObject(hdc, GlobalFont::Bold);
    RECT textRectL1 = {tabMarginX, startY + avaSize + UIScaler::SY(30), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(70)};
    DrawTextW(hdc, player1NameW.c_str(), -1, &textRectL1, DT_CENTER | DT_SINGLELINE);

    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, ToCOLORREF(Palette::White));
    std::wstring p1Piece = GetText("play_piece") + L"X";
    RECT textRectL2 = {tabMarginX, startY + avaSize + UIScaler::SY(60), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(100)};
    DrawTextW(hdc, p1Piece.c_str(), -1, &textRectL2, DT_CENTER | DT_SINGLELINE);

    std::wstring p1Wins = GetText("play_goals") + std::to_wstring(state->player1.totalWins);
    RECT textRectL3 = {tabMarginX, startY + avaSize + UIScaler::SY(90), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(130)};
    DrawTextW(hdc, p1Wins.c_str(), -1, &textRectL3, DT_CENTER | DT_SINGLELINE);

    std::wstring p1Moves = GetText("play_dribble") + std::to_wstring(state->player1.movesCount);
    RECT textRectL4 = {tabMarginX, startY + avaSize + UIScaler::SY(120), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(160)};
    DrawTextW(hdc, p1Moves.c_str(), -1, &textRectL4, DT_CENTER | DT_SINGLELINE);

    std::wstring p1MatchWins = GetText("play_bowins") + std::to_wstring(state->player1.matchWins);
    RECT textRectL5 = {tabMarginX, startY + avaSize + UIScaler::SY(150), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(190)};
    DrawTextW(hdc, p1MatchWins.c_str(), -1, &textRectL5, DT_CENTER | DT_SINGLELINE);

    {
        static PlayerState p1State;
        p1State.avatarType = decodeAvatar(state->player1.avatarPath);
        p1State.flipH = false;

        std::string nextAction = "idle";
        if (state->status == MATCH_FINISHED)
        {
            nextAction = (state->winner == CELL_PLAYER1) ? "win" : "sad";
        }
        else if (state->status == MATCH_PLAYING && state->isPlayer1Turn)
        {
            nextAction = "run";
        }

        if (p1State.currentAction != nextAction)
        {
            p1State.currentAction = nextAction;
            p1State.currentFrame = 0;
        }

        int animCX = tabMarginX + leftTabW / 2;
        int animCY = startY + avaSize + UIScaler::SY(450);
        int animSize = UIScaler::S(280);
        DrawPixelAction(g, animCX, animCY, animSize, p1State);
    }

    int rightTabStartX = startX + boardPixelSize + UIScaler::SX(10);
    int rightTabW = screenWidth - rightTabStartX - tabMarginX;
    g.FillRectangle(&shadowBrush, rightTabStartX, startY, rightTabW, boardPixelSize);

    if (state->status == MATCH_PLAYING && !state->isPlayer1Turn)
    {
        int alpha = (int)(30 + sin(g_GlobalAnimTime * 8.0f) * 30.0f);
        Gdiplus::SolidBrush *p2TurnPulse = GetCachedBrush(ToGdiColor(WithAlpha(Theme::P2TurnPulse, (BYTE)alpha)));
        g.FillRectangle(p2TurnPulse, rightTabStartX, startY, rightTabW, boardPixelSize);
        Gdiplus::Pen p2Pen(ToGdiColor(Theme::P2TurnBorder), 3.0f);
        g.DrawRectangle(&p2Pen, rightTabStartX, startY, rightTabW, boardPixelSize);
    }

    int avaX_R = rightTabStartX + (rightTabW - avaSize) / 2;
    std::wstring player2NameW = state->player2.name;

    Gdiplus::SolidBrush *waterBrushR = GetCachedBrush(ToGdiColor(Theme::P2Watermark));
    g.TranslateTransform((Gdiplus::REAL)(avaX_R + avaSize / 2), (Gdiplus::REAL)(startY + UIScaler::SY(80)));
    g.RotateTransform(30.0f);
    g.DrawString(player2NameW.c_str(), -1, &s_waterFont, Gdiplus::PointF(0, 0), &s_alignCenter, waterBrushR);
    g.ResetTransform();

    DrawPixelAvatar(g, avaX_R, startY + UIScaler::SY(20), avaSize, decodeAvatar(state->player2.avatarPath));

    SetTextColor(hdc, ToCOLORREF(Palette::CyanNormal));
    SelectObject(hdc, GlobalFont::Bold);
    RECT textRectR1 = {rightTabStartX, startY + avaSize + UIScaler::SY(30), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(70)};
    DrawTextW(hdc, player2NameW.c_str(), -1, &textRectR1, DT_CENTER | DT_SINGLELINE);

    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, ToCOLORREF(Palette::White));
    std::wstring p2Piece = GetText("play_piece") + L"O";
    RECT textRectR2 = {rightTabStartX, startY + avaSize + UIScaler::SY(60), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(100)};
    DrawTextW(hdc, p2Piece.c_str(), -1, &textRectR2, DT_CENTER | DT_SINGLELINE);

    std::wstring p2Wins = GetText("play_goals") + std::to_wstring(state->player2.totalWins);
    RECT textRectR3 = {rightTabStartX, startY + avaSize + UIScaler::SY(90), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(130)};
    DrawTextW(hdc, p2Wins.c_str(), -1, &textRectR3, DT_CENTER | DT_SINGLELINE);

    std::wstring p2Moves = GetText("play_dribble") + std::to_wstring(state->player2.movesCount);
    RECT textRectR4 = {rightTabStartX, startY + avaSize + UIScaler::SY(120), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(160)};
    DrawTextW(hdc, p2Moves.c_str(), -1, &textRectR4, DT_CENTER | DT_SINGLELINE);

    std::wstring p2MatchWins = GetText("play_bowins") + std::to_wstring(state->player2.matchWins);
    RECT textRectR5 = {rightTabStartX, startY + avaSize + UIScaler::SY(150), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(190)};
    DrawTextW(hdc, p2MatchWins.c_str(), -1, &textRectR5, DT_CENTER | DT_SINGLELINE);

    {
        static PlayerState p2State;
        p2State.avatarType = decodeAvatar(state->player2.avatarPath);
        p2State.flipH = true;
        std::string nextAction = "idle";
        if (state->status == MATCH_FINISHED)
        {
            nextAction = (state->winner == CELL_PLAYER2) ? "win" : "sad";
        }
        else if (state->status == MATCH_PLAYING && !state->isPlayer1Turn)
        {
            nextAction = "run";
        }

        if (p2State.currentAction != nextAction)
        {
            p2State.currentAction = nextAction;
            p2State.currentFrame = 0;
        }

        int animCX = rightTabStartX + rightTabW / 2;
        int animCY = startY + avaSize + UIScaler::SY(450);
        int animSize = UIScaler::S(280);
        DrawPixelAction(g, animCX, animCY, animSize, p2State);
    }

    Gdiplus::SolidBrush *pitchBrush = GetCachedBrush(ToGdiColor(Theme::BoardPitch));
    g.FillRectangle(pitchBrush, startX, startY, boardPixelSize, boardPixelSize);
    Gdiplus::Pen pitchBorder(ToGdiColor(Theme::BoardBorder), 3);
    g.DrawRectangle(&pitchBorder, startX, startY, boardPixelSize, boardPixelSize);

    DrawGameBoard(g, hdc, state, dynamicCellSize, startX, startY);

    // --- Redesigned Match Format & Turn Indicator (Scoreboard Style) ---
    std::wstring s_fmt = GetText("play_format");
    std::wstring s_trn = GetText("play_turn");

    std::wstring formatText = s_fmt + L" BO" + std::to_wstring(state->targetScore);
    std::wstring turnText = s_trn + L": " + (state->isPlayer1Turn ? player1NameW : player2NameW);
    std::wstring fullScoreText = formatText + L"  |  " + turnText;

    int scoreW = UIScaler::SX(600);
    int scoreH = UIScaler::SY(45);
    int scoreX = (screenWidth - scoreW) / 2;
    int scoreY = UIScaler::SY(10);

    // Background Scoreboard Panel (Glassmorphism + Gradient)
    Gdiplus::LinearGradientBrush scoreBg(
        Gdiplus::Point(scoreX, scoreY),
        Gdiplus::Point(scoreX, scoreY + scoreH),
        Gdiplus::Color(210, 10, 15, 25),
        Gdiplus::Color(210, 30, 45, 65));
    g.FillRectangle(&scoreBg, scoreX, scoreY, scoreW, scoreH);

    // Glowing border pulses with turn color
    COLORREF turnColor = state->isPlayer1Turn ? ToCOLORREF(Palette::OrangeNormal) : ToCOLORREF(Palette::CyanNormal);
    float pulseScore = 0.6f + sin(g_GlobalAnimTime * 6.0f) * 0.4f;
    BYTE scoreAlpha = (BYTE)(130 + pulseScore * 125);
    Gdiplus::Pen scoreBorder(Gdiplus::Color(scoreAlpha, GetRValue(turnColor), GetGValue(turnColor), GetBValue(turnColor)), 2.5f);
    g.DrawRectangle(&scoreBorder, scoreX, scoreY, scoreW, scoreH);

    // Main Scoreboard Text
    SetTextColor(hdc, ToCOLORREF(Palette::White));
    HFONT oldF = (HFONT)SelectObject(hdc, GlobalFont::Bold);
    RECT rScore = {scoreX, scoreY, scoreX + scoreW, scoreY + scoreH};
    DrawTextW(hdc, fullScoreText.c_str(), -1, &rScore, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, oldF);

    int timerY = UIScaler::SY(55);
    int shakeX = 0, shakeY = 0;
    COLORREF timerColor = ToCOLORREF(Palette::CyanLight);
    bool isWarning = (state->matchType != MATCH_PVE && state->timeRemaining <= 3);

    if (isWarning)
    {
        float shakeFreq = 25.0f;
        float shakeAmp = (float)UIScaler::S(4);
        shakeX = (int)(sin(g_GlobalAnimTime * shakeFreq) * shakeAmp);
        shakeY = (int)(cos(g_GlobalAnimTime * shakeFreq * 0.7f) * shakeAmp);
        timerColor = ToCOLORREF(Palette::RedNormal);
    }

    int clockSize = UIScaler::S(34);
    std::wstring timeText;
    std::wstring s_time = GetText("play_time");
    std::wstring s_time_rem = GetText("play_time_rem");

    if (state->matchType == MATCH_PVE)
    {
        int minutes = (int)state->matchDuration / 60;
        int seconds = (int)state->matchDuration % 60;
        wchar_t buffer[64];
        swprintf(buffer, 64, L"%02d:%02d", minutes, seconds);
        timeText = s_time + buffer;
    }
    else
    {
        timeText = s_time_rem + std::to_wstring(state->timeRemaining) + L"s";
    }

    // --- Redesigned Timer (Digital LED Style) ---
    int clockW = UIScaler::SX(320); // Further increased width for better spacing
    int clockH = UIScaler::SY(45);
    int clockX = (screenWidth - clockW) / 2 + shakeX;
    int clockY = UIScaler::SY(62) + shakeY;

    // Timer Box Background (Deep Glass)
    Gdiplus::SolidBrush timerBg(Gdiplus::Color(200, 5, 10, 20));
    g.FillRectangle(&timerBg, clockX, clockY, clockW, clockH);

    // Warning Pulse for border & Glow
    float pulseTime = isWarning ? (0.5f + sin(g_GlobalAnimTime * 20.0f) * 0.5f) : (0.7f + sin(g_GlobalAnimTime * 5.0f) * 0.3f);
    BYTE timerAlpha = (BYTE)(130 + pulseTime * 125);
    Gdiplus::Pen timerBorder(Gdiplus::Color(timerAlpha, GetRValue(timerColor), GetGValue(timerColor), GetBValue(timerColor)), 2.5f);
    g.DrawRectangle(&timerBorder, clockX, clockY, clockW, clockH);

    // Internal Digital Plate
    Gdiplus::SolidBrush plateBrush(Gdiplus::Color(40, GetRValue(timerColor), GetGValue(timerColor), GetBValue(timerColor)));
    g.FillRectangle(&plateBrush, clockX + 4, clockY + 4, clockW - 8, clockH - 8);

    // Draw time text centered in the digital box
    HFONT oldT = (HFONT)SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, timerColor);
    RECT rTimer = {clockX, clockY, clockX + clockW, clockY + clockH};
    DrawTextW(hdc, timeText.c_str(), -1, &rTimer, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, oldT);

    // Ghi chú tính năng Đi Lại
    if (state->matchType == MATCH_PVE)
    {
        std::wstring s_undo = GetText("play_undo_hint");
        DrawTextCentered(hdc, s_undo, screenHeight - UIScaler::SY(40), screenWidth, ToCOLORREF(Palette::GrayNormal), GlobalFont::Default);
    }

    SelectObject(hdc, hOldFont);

    // 2. Lớp phủ Pause Menu (Phong cách Light Glassmorphism)
    if (state->status == MATCH_PAUSED)
    {
        // Sử dụng tiếp đối tượng 'g' đã có ở đầu hàm để tối ưu hiệu năng
        Gdiplus::SolidBrush shadowBrush2(Gdiplus::Color(100, 255, 255, 255));
        g.FillRectangle(&shadowBrush2, 0, 0, screenWidth, screenHeight);

        int menuW = UIScaler::SX(580), menuH = UIScaler::SY(500);
        int guideW = UIScaler::SX(350);
        int gap = UIScaler::SX(25);

        int totalW = (g_CurrentSubMenu == SUB_MAIN) ? (menuW + gap + guideW) : menuW;
        int totalStartX = (screenWidth - totalW) / 2;

        int menuX = totalStartX;
        int menuY = (screenHeight - menuH) / 2;

        Gdiplus::SolidBrush bgBrush(ToGdiColor(Theme::GlassWhite));
        g.FillRectangle(&bgBrush, menuX, menuY, menuW, menuH);

        Gdiplus::Pen yellowBorder(ToGdiColor(Theme::PanelYellowBorder), 4.0f);
        g.DrawRectangle(&yellowBorder, menuX, menuY, menuW, menuH);

        static std::map<int, Gdiplus::Color> waterPalette;
        if (waterPalette.empty())
        {
            waterPalette[1] = Gdiplus::Color(30, 0, 0, 0);     // Viền cực mờ
            waterPalette[2] = Gdiplus::Color(40, 255, 200, 0); // Màu vàng nhạt mờ
            waterPalette[3] = Gdiplus::Color(40, 255, 255, 255);
        }
        static PixelModel whistleMod = LoadPixelModel("Asset/models/bg/whistle.txt");
        DrawPixelModel(g, whistleMod, screenWidth / 2, screenHeight / 2, UIScaler::S(250), waterPalette);

        if (g_CurrentSubMenu == SUB_MAIN)
        {
            std::wstring s_pause_title = GetText("pause_title");
            // Header: Thanh Banner Tạm dừng
            DrawPixelBanner(g, hdc, s_pause_title.c_str(), menuX + menuW / 2, menuY + UIScaler::SY(40),
                            menuW - UIScaler::SX(20), ToCOLORREF(Palette::White), RGB(255, 180, 0), "Asset/models/bg/whistle.txt");

            std::wstring s_labels[TOTAL_PAUSE_ITEMS];
            std::wstring s_on, s_off, s_locked;
            s_labels[0] = GetText("pause_resume");
            s_labels[1] = GetText("pause_sfx") + L":";
            s_labels[2] = GetText("pause_sfx_vol");
            s_labels[3] = GetText("pause_save");
            s_labels[4] = GetText("pause_exit");
            s_on = GetText("btn_on");
            s_off = GetText("btn_off");
            s_locked = GetText("btn_locked");

            for (int i = 0; i < TOTAL_PAUSE_ITEMS; i++)
            {
                std::wstring itemText = s_labels[i];
                COLORREF color = ToCOLORREF(Palette::GrayDarkest);
                HFONT font = GlobalFont::Default;
                bool isDisabled = (i == 2 && !config->isSfxEnabled);
                if (i == 1)
                {
                    itemText += (config->isSfxEnabled ? L" [ " + s_on + L" ]" : L" [ " + s_off + L" ]");
                }

                if (i == g_PauseSelected && !isDisabled)
                {
                    int wave = (int)(180 + sin(g_GlobalAnimTime * 10.0f) * 75);
                    color = RGB(255, wave, 0);
                    font = GlobalFont::Bold;
                    if (i != 2)
                        itemText = L">> " + itemText + L" <<";
                }

                if (isDisabled)
                {
                    color = RGB(150, 150, 150);
                    font = GlobalFont::Default;
                    itemText = s_labels[i] + L" [ " + s_locked + L" ]";
                }

                int itemY = menuY + UIScaler::SY(140) + i * UIScaler::SY(55);
                DrawTextCentered(hdc, (i == 2) ? s_labels[i] : itemText, itemY, menuX + menuW, color, font, menuX);

                // Vẽ Slider cho mục Âm Lượng
                if (i == 2)
                {
                    int barW = UIScaler::SX(220);
                    int barH = UIScaler::SY(8);
                    int barX = menuX + (menuW - barW) / 2;
                    int barY = itemY + UIScaler::SY(38);

                    Gdiplus::SolidBrush bgSlider(Gdiplus::Color(60, 0, 0, 0));
                    g.FillRectangle(&bgSlider, barX, barY, barW, barH);

                    float percent = config->sfxVolume / 100.0f;
                    Gdiplus::Color activeColor = isDisabled ? Gdiplus::Color(100, 150, 150, 150) : Gdiplus::Color(255, 255, 200, 0);
                    Gdiplus::SolidBrush valBrush(activeColor);
                    g.FillRectangle(&valBrush, barX, barY, (int)(barW * percent), barH);

                    int handleX = barX + (int)(barW * percent);
                    Gdiplus::Color hColor = isDisabled ? Gdiplus::Color(255, 100, 100, 100) : Gdiplus::Color(255, 255, 255, 255);
                    Gdiplus::SolidBrush hBrush(hColor);
                    int hr = UIScaler::S(8);
                    g.FillEllipse(&hBrush, handleX - hr, barY - UIScaler::SY(4), hr * 2, hr * 2);

                    if (i == g_PauseSelected && !isDisabled)
                    {
                        Gdiplus::Pen aura(Gdiplus::Color(150, 255, 200, 0), 2);
                        g.DrawEllipse(&aura, handleX - 10, barY - 6, 20, 20);
                    }
                }
            }

            // --- 4. Cột Hướng Dẫn Điều Khiển (Bên Phải) ---
            int guideX = menuX + menuW + gap;
            g.FillRectangle(&bgBrush, guideX, menuY, guideW, menuH);
            g.DrawRectangle(&yellowBorder, guideX, menuY, guideW, menuH);

            DrawPixelBanner(g, hdc, GetText("guide_title").c_str(), guideX + guideW / 2, menuY + UIScaler::SY(40),
                            guideW - UIScaler::SX(20), ToCOLORREF(Palette::White), RGB(0, 160, 255), "Asset/models/bg/football.txt");

            struct GuideItem
            {
                std::wstring key;
                std::wstring desc;
            };
            std::vector<GuideItem> instructions = {
                {GetText("guide_move"), GetText("guide_move_key")},
                {GetText("guide_place"), GetText("guide_place_key")},
                {GetText("guide_undo"), GetText("guide_undo_key")},
                {GetText("guide_redo"), GetText("guide_redo_key")},
                {GetText("guide_pause"), GetText("guide_pause_key")}};

            int startGuideY = menuY + UIScaler::SY(110);
            for (const auto &item : instructions)
            {
                SetTextColor(hdc, ToCOLORREF(Palette::BlueDarkest));
                SelectObject(hdc, GlobalFont::Default);
                RECT rKey = {guideX + UIScaler::SX(15), startGuideY, guideX + guideW - UIScaler::SX(15), startGuideY + UIScaler::SY(40)};
                DrawTextW(hdc, item.key.c_str(), -1, &rKey, DT_LEFT | DT_TOP | DT_SINGLELINE);

                SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
                SelectObject(hdc, GlobalFont::Note);
                RECT rDesc = {guideX + UIScaler::SX(25), startGuideY + UIScaler::SY(32), guideX + guideW - UIScaler::SX(15), startGuideY + UIScaler::SY(70)};
                DrawTextW(hdc, item.desc.c_str(), -1, &rDesc, DT_LEFT | DT_TOP | DT_SINGLELINE);

                startGuideY += UIScaler::SY(75);
            }
        }
        else if (g_CurrentSubMenu == SUB_SAVE_SELECT)
        {
            DrawTextCentered(hdc, GetText("save_title"), menuY + UIScaler::SY(40), screenWidth, ToCOLORREF(Palette::OrangeNormal), GlobalFont::Title);

            for (int i = 0; i < MAX_SAVE_SLOTS; i++)
            {
                std::wstring displayName = GetSaveDisplayName(i + 1);
                bool exists = CheckSaveExists(i + 1);
                if (exists)
                {
                    if (displayName.length() > 20)
                        displayName = displayName.substr(0, 17) + L"...";
                }
                else
                {
                    displayName = GetText("save_empty");
                }

                std::wstring slotLabel = L"Slot " + std::to_wstring(i + 1) + L": " + displayName;
                COLORREF color = (i == g_SaveSlotSelected) ? ToCOLORREF(Palette::BlueDarkest) : ToCOLORREF(Palette::GrayDarkest);
                HFONT font = (i == g_SaveSlotSelected) ? GlobalFont::Bold : GlobalFont::Default;

                if (i == g_SaveSlotSelected)
                {
                    slotLabel = L"-> " + slotLabel + L" <-";
                }
                DrawTextCentered(hdc, slotLabel, menuY + UIScaler::SY(130) + i * UIScaler::SY(55), screenWidth, color, font);
            }
            DrawTextCentered(hdc, GetText("save_confirm"), menuY + UIScaler::SY(430), screenWidth, ToCOLORREF(Palette::GreenNormal), GlobalFont::Default);
            DrawTextCentered(hdc, GetText("save_back"), menuY + UIScaler::SY(460), screenWidth, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);
        }
        else if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY)
        {
            DrawTextCentered(hdc, GetText("save_label_title"), menuY + UIScaler::SY(60), screenWidth, ToCOLORREF(Palette::CyanNormal), GlobalFont::Title);

            int boxW = UIScaler::SX(350), boxH = UIScaler::SY(50);
            int boxX = (screenWidth - boxW) / 2;
            int boxY = menuY + UIScaler::SY(200);

            HBRUSH hBoxBrush = CreateSolidBrush(RGB(240, 240, 250));
            RECT rectBox = {boxX, boxY, boxX + boxW, boxY + boxH};
            FillRect(hdc, &rectBox, hBoxBrush);
            DeleteObject(hBoxBrush);

            Gdiplus::Pen tbPen(ToGdiColor(WithAlpha(Theme::TitleBorder, 180)), 2.0f);
            g.DrawRectangle(&tbPen, boxX, boxY, boxW, boxH);

            DrawTextCentered(hdc, GetText("play_save_label"), boxY - UIScaler::SY(35), screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold);

            extern float g_GlobalAnimTime;
            bool showCursor = ((int)(g_GlobalAnimTime * 2.5f) % 2 == 0);
            std::wstring displayText = g_SaveNameInput + (showCursor ? L"_" : L" ");

            RECT rInput = {boxX, boxY, boxX + boxW, boxY + boxH + UIScaler::SY(8)};
            SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
            SelectObject(hdc, GlobalFont::Bold);
            DrawTextW(hdc, displayText.c_str(), -1, &rInput, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            if (!g_SaveStatusMsg.empty())
            {
                COLORREF msgColor = (g_SaveStatusMsg == GetText("msg_save_success")) ? ToCOLORREF(Palette::GreenNormal) : ToCOLORREF(Palette::RedNormal);
                DrawTextCentered(hdc, g_SaveStatusMsg, boxY + boxH + UIScaler::SY(20), screenWidth, msgColor, GlobalFont::Bold);
            }

            DrawTextCentered(hdc, GetText("play_save_hint1"), menuY + UIScaler::SY(380), screenWidth, ToCOLORREF(Palette::GreenNormal), GlobalFont::Default);
            DrawTextCentered(hdc, GetText("play_save_hint2"), menuY + UIScaler::SY(430), screenWidth, ToCOLORREF(Palette::RedNormal), GlobalFont::Note);
        }
    }
    else if (state->status == MATCH_FINISHED)
    {
        std::wstring winMsg;
        COLORREF winColor;
        COLORREF winGlow;

        int winRequired = state->targetScore / 2 + 1;
        bool matchOver = (state->player1.totalWins >= winRequired || state->player2.totalWins >= winRequired);

        if (state->winner == CELL_PLAYER1)
        {
            winMsg = matchOver ? (player1NameW + GetText("play_win_cup")) : (player1NameW + GetText("play_win_goal"));
            winColor = ToCOLORREF(Palette::OrangeNormal);
            winGlow = RGB(255, 120, 0);
        }
        else if (state->winner == CELL_PLAYER2)
        {
            winMsg = matchOver ? player2NameW + GetText("play_win_cup") : (player2NameW + GetText("play_win_goal"));
            winColor = ToCOLORREF(Palette::CyanNormal);
            winGlow = RGB(0, 150, 255);
        }
        else
        {
            winMsg = GetText("play_draw");
            winColor = ToCOLORREF(Palette::GrayDark);
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
        g.DrawLine(&glowLine, 0, bannerH - UIScaler::SY(2), screenWidth, bannerH - UIScaler::SY(2));

        int ballX = -UIScaler::SX(60) + (int)(fmod(g_WinAnimTime * 300.0f, (float)(screenWidth + UIScaler::SX(120))));
        int ballY = bannerH / 2 + UIScaler::SY((int)(sin(g_WinAnimTime * 6.0f) * 12.0f));
        DrawPixelFootball(g, ballX, ballY, UIScaler::S(40));

        int msgY = bannerH / 2 - UIScaler::SY(40);
        DrawTextCentered(hdc, winMsg, msgY, screenWidth, winColor, GlobalFont::Title);

        if (g_WinAnimTime > 0.5f)
        {
            extern float g_GlobalAnimTime;
            int blinkAlpha = (int)(155 + sin(g_GlobalAnimTime * 6.0f) * 100);
            COLORREF dynColor = RGB(blinkAlpha, blinkAlpha, blinkAlpha);
            DrawTextCentered(hdc, GetText("play_win_hint"), msgY + UIScaler::SY(52), screenWidth, dynColor, GlobalFont::Note);
        }
    }
}

void UpdatePlayScreen(PlayState *state, ScreenState &currentState, WPARAM wParam, GameConfig *config)
{
    if (wParam == 0)
    {
        return;
    }
    ProcessPlayInput(wParam, state, currentState, config);
}

void ResetPlayScreenStatics()
{
    g_CurrentSubMenu = SUB_MAIN;
    g_PauseSelected = 0;
    g_SaveSlotSelected = 0;
    g_SaveNameInput = L"";
}
