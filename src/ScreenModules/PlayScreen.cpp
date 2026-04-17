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

extern bool UpdateCountdown(PlayState* state, double dt);

static std::future<std::pair<int, int>> g_AIFuture;
static bool g_AIsCalculating = false;
static float g_WinAnimTime = 0.0f;
static bool isEditingSaveName = false;
static MatchStatus g_PrePauseStatus = MATCH_PLAYING;
static int g_SummarySelected = 0;
static float g_SaveFeedbackTimer = 0.0f;
static std::wstring g_SaveStatusMsg = L"";

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
        state->matchDuration += (float)dt;
        if (state->isP1Turn) state->p1.totalTimePossessed += (float)dt;
        else state->p2.totalTimePossessed += (float)dt;

        if (state->matchType == MATCH_PVE) {
            needsRedraw = true;
        }
        else {
            if (UpdateCountdown(state, dt)) {
                needsRedraw = true;
                if (state->timeRemaining <= 0) {
                    PlaySFX("sfx_timeout");
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

    if (g_SaveFeedbackTimer > 0.0f) {
        g_SaveFeedbackTimer -= (float)dt;
        if (g_SaveFeedbackTimer <= 0.0f) {
            g_SaveFeedbackTimer = 0.0f;
            g_SaveStatusMsg = L"";
            if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
                g_CurrentSubMenu = SUB_MAIN;
            }
        }
        needsRedraw = true;
    }

    return needsRedraw;
}

bool ProcessPlayInput(WPARAM wParam, PlayState* state, ScreenState& currentState, GameConfig* config) {
    bool hasChanged = false;

    if (state->status == MATCH_PAUSED) {
        if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
            bool isChar = (wParam & 0x10000);
            wchar_t ch = (wchar_t)(wParam & 0xFFFF);
            if (isChar) {
                if (g_SaveNameInput.length() < 15 && ch >= 32) g_SaveNameInput += ch;
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
                if (g_SaveNameInput.empty()) g_SaveNameInput = L"NewGame";
                std::wstring path = GetSavePath(g_SaveSlotSelected + 1);
                state->saveName = g_SaveNameInput;

                if (SaveMatchData(state, path)) {
                    PlaySFX("sfx_success");
                    g_SaveStatusMsg = GetText("msg_save_success");
                    g_SaveFeedbackTimer = 1.5f;
                }
                else {
                    g_SaveStatusMsg = GetText("msg_save_error");
                    g_SaveFeedbackTimer = 1.5f;
                }
                return true;
            }
            return true;
        }

        if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            if (wParam == VK_ESCAPE) { g_CurrentSubMenu = SUB_MAIN; return true; }
            if (wParam == 'W' || wParam == VK_UP) { g_SaveSlotSelected = (g_SaveSlotSelected - 1 < 0) ? MAX_SAVE_SLOTS - 1 : g_SaveSlotSelected - 1; return true; }
            if (wParam == 'S' || wParam == VK_DOWN) { g_SaveSlotSelected = (g_SaveSlotSelected + 1 >= MAX_SAVE_SLOTS) ? 0 : g_SaveSlotSelected + 1; return true; }
            if (wParam == VK_RETURN || wParam == VK_SPACE) {
                PlaySFX("sfx_select");
                g_CurrentSubMenu = SUB_SAVE_NAME_ENTRY;
                g_SaveNameInput = L"";
                return true;
            }
            return false;
        }

        if (wParam == VK_ESCAPE) { state->status = g_PrePauseStatus; return true; }

        if (wParam == 'W' || wParam == VK_UP) {
            g_PauseSelected = (g_PauseSelected - 1 < 0) ? TOTAL_PAUSE_ITEMS - 1 : g_PauseSelected - 1;
            if (g_PauseSelected == 2 && !config->isBgmEnabled) g_PauseSelected = (g_PauseSelected - 1 < 0) ? TOTAL_PAUSE_ITEMS - 1 : g_PauseSelected - 1;
            PlaySFX("sfx_move");
            hasChanged = true;
        }
        else if (wParam == 'S' || wParam == VK_DOWN) {
            g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            if (g_PauseSelected == 2 && !config->isBgmEnabled) g_PauseSelected = (g_PauseSelected + 1 >= TOTAL_PAUSE_ITEMS) ? 0 : g_PauseSelected + 1;
            PlaySFX("sfx_move");
            hasChanged = true;
        }
        else if (wParam == VK_RETURN || wParam == VK_SPACE || wParam == VK_RIGHT || wParam == VK_LEFT) {
            int dir = (wParam == VK_LEFT) ? -1 : 1;
            switch (g_PauseSelected) {
            case 0: state->status = g_PrePauseStatus; break;
            case 1:
                config->isBgmEnabled = !config->isBgmEnabled;
                if (!config->isBgmEnabled) StopBGM();
                else PlayBGM("Asset/audio/c1.mp3");
                break;
            case 2:
                config->sfxVolume += dir * 10;
                if (config->sfxVolume > 100) config->sfxVolume = 100;
                if (config->sfxVolume < 0) config->sfxVolume = 0;
                break;
            case 3:
                PlaySFX("sfx_select");
                g_CurrentSubMenu = SUB_SAVE_SELECT;
                break;
            case 4:
                SaveConfig(config, "Asset/config.ini");
                currentState = SCREEN_MENU;
                if (config->isBgmEnabled) PlayBGM("Asset/audio/c1.mp3");
                ResetPlayScreenStatics();
                break;
            }
            if (g_PauseSelected != 3) PlaySFX("sfx_move");
            hasChanged = true;
        }
        return hasChanged;
    }

    if (state->status == MATCH_PLAYING) {
        if (g_AIsCalculating) return false;

        if (wParam == VK_ESCAPE) {
            PlaySFX("sfx_select");
            g_PrePauseStatus = state->status;
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }

        if (wParam == 'Q' || wParam == 'q') { undoMove(state); return true; }
        if (wParam == 'E' || wParam == 'e') { redoMove(state); return true; }

        if ((wParam == 'W' || wParam == VK_UP) && state->cursorRow > 0) { state->cursorRow--; PlaySFX("sfx_move"); hasChanged = true; }
        if ((wParam == 'S' || wParam == VK_DOWN) && state->cursorRow < state->boardSize - 1) { state->cursorRow++; PlaySFX("sfx_move"); hasChanged = true; }
        if ((wParam == 'A' || wParam == VK_LEFT) && state->cursorCol > 0) { state->cursorCol--; PlaySFX("sfx_move"); hasChanged = true; }
        if ((wParam == 'D' || wParam == VK_RIGHT) && state->cursorCol < state->boardSize - 1) { state->cursorCol++; PlaySFX("sfx_move"); hasChanged = true; }
        if (wParam == VK_RETURN || wParam == VK_SPACE) {
            if (processMove(state, state->cursorRow, state->cursorCol)) {
                PlaySFX("sfx_place");
                return true;
            }
        }
    }
    else if (state->status == MATCH_FINISHED) {
        if (g_WinAnimTime < 0.5f) return false;

        if (wParam == VK_ESCAPE) {
            PlaySFX("sfx_select");
            g_PrePauseStatus = state->status;
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }
        else {
            state->status = MATCH_SUMMARY;
            g_SummarySelected = 0;
            return true;
        }
    }
    else if (state->status == MATCH_SUMMARY) {
        if (wParam == VK_ESCAPE) {
            PlaySFX("sfx_select");
            g_PrePauseStatus = state->status;
            state->status = MATCH_PAUSED;
            g_PauseSelected = 0;
            return true;
        }

        if (wParam == 'W' || wParam == 'w' || wParam == VK_UP) {
            g_SummarySelected = (g_SummarySelected - 1 < 0) ? 2 : g_SummarySelected - 1;
            PlaySFX("sfx_move");
            hasChanged = true;
        }
        else if (wParam == 'S' || wParam == 's' || wParam == VK_DOWN) {
            g_SummarySelected = (g_SummarySelected + 1 > 2) ? 0 : g_SummarySelected + 1;
            PlaySFX("sfx_move");
            hasChanged = true;
        }
        else if (wParam == VK_RETURN || wParam == VK_SPACE) {
            StopSFX("sfx_crowd");
            StopSFX("sfx_whistle");
            StopSFX("sfx_siu");
            if (g_SummarySelected == 0) {
                int winRequired = state->targetScore / 2 + 1;
                bool matchOver = (state->p1.totalWins >= winRequired || state->p2.totalWins >= winRequired);
                if (matchOver) {
                    state->p1.totalWins = 0;
                    state->p2.totalWins = 0;
                }
                startNextRound(state);
                PlaySFX("sfx_whistle");
            }
            else if (g_SummarySelected == 1) {
                PlaySFX("sfx_select");
                g_PrePauseStatus = state->status;
                state->status = MATCH_PAUSED;
                g_CurrentSubMenu = SUB_SAVE_SELECT;
            }
            else if (g_SummarySelected == 2) {
                PlaySFX("sfx_select");
                currentState = SCREEN_MENU;
                if (config->isBgmEnabled) PlayBGM("Asset/audio/c1.mp3");
            }
            hasChanged = true;
        }
    }

    return hasChanged;
}

void RenderPlayScreen(HDC hdc, const PlayState* state, int screenWidth, int screenHeight, const GameConfig* config) {
    Gdiplus::Graphics g(hdc);
    DrawProceduralStadium(g, screenWidth, screenHeight);

    auto decodeAvatar = [](const std::string& path) -> int {
        if (path.find("avatar_") != std::string::npos) {
            try { return std::stoi(path.substr(7)); }
            catch (...) { return 0; }
        }
        return 0;
        };

    if (state->status == MATCH_SUMMARY) {
        Gdiplus::SolidBrush shadowBrush2(Gdiplus::Color(230, 245, 250, 255));
        g.FillRectangle(&shadowBrush2, 0, 0, screenWidth, screenHeight);

        DrawTextCentered(hdc, GetText("summary_title"), UIScaler::SY(50), screenWidth, ToCOLORREF(Palette::BlueDarkest), GlobalFont::Title);

        int panelW = UIScaler::SX(350);
        int panelH = UIScaler::SY(450);
        int p1X = screenWidth / 2 - panelW - UIScaler::SX(40);
        int p2X = screenWidth / 2 + UIScaler::SX(40);
        int panelY = UIScaler::SY(120);

        auto drawSummaryPanel = [&](int x, const PlayerInfo2& p, bool isWinner, bool flipModel) {
            Gdiplus::SolidBrush bgBrush(isWinner ? ToGdiColor(WithAlpha(Palette::YellowLight, 220)) : ToGdiColor(WithAlpha(Palette::GrayLightest, 220)));
            Gdiplus::Pen border(isWinner ? ToGdiColor(Palette::OrangeNormal) : ToGdiColor(Palette::GrayNormal), 4.0f);
            g.FillRectangle(&bgBrush, x, panelY, panelW, panelH);
            g.DrawRectangle(&border, x, panelY, panelW, panelH);

            std::wstring title = p.name;
            if (isWinner) title += GetText("summary_win");
            SetTextColor(hdc, ToCOLORREF(Palette::RedDarkest));
            SelectObject(hdc, GlobalFont::Bold);
            RECT rTitle = { x, panelY + UIScaler::SY(12), x + panelW, panelY + UIScaler::SY(48) };
            DrawTextW(hdc, title.c_str(), -1, &rTitle, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            PlayerState pState;
            pState.avatarType = decodeAvatar(p.avatarPath);
            pState.flipH = flipModel;
            pState.currentAction = isWinner ? "win" : "sad";
            if (state->winner == 0) pState.currentAction = "idle";
            extern float g_GlobalAnimTime;
            pState.currentFrame = (int)(g_GlobalAnimTime * 8.0f) % 4;

            DrawPixelAction(g, x + panelW / 2, panelY + UIScaler::SY(148), UIScaler::S(155), pState);

            SelectObject(hdc, GlobalFont::Default);
            int sY = panelY + UIScaler::SY(258);
            int sH = UIScaler::SY(32);
            int sXL = x + UIScaler::SX(8);
            int sXR = x + panelW - UIScaler::SX(8);

            SetTextColor(hdc, ToCOLORREF(Palette::BlueDark));
            std::wstring statsP = GetText("summary_dribble") + std::to_wstring(p.movesCount);
            RECT r1 = { sXL, sY, sXR, sY + sH };
            DrawTextW(hdc, statsP.c_str(), -1, &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            sY += sH;

            int possMins = (int)p.totalTimePossessed / 60;
            int possSecs = (int)p.totalTimePossessed % 60;
            wchar_t possBuffer[64];
            swprintf(possBuffer, 64, L"%02d:%02d", possMins, possSecs);
            std::wstring possStr = GetText("summary_poss") + possBuffer;
            SetTextColor(hdc, ToCOLORREF(Palette::GreenDark));
            RECT r2 = { sXL, sY, sXR, sY + sH };
            DrawTextW(hdc, possStr.c_str(), -1, &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            sY += sH;

            const PlayerInfo2& opponent = (&p == &state->p1) ? state->p2 : state->p1;
            SetTextColor(hdc, ToCOLORREF(Palette::OrangeDarkest));
            std::wstring tkWin = GetText("summary_goal") + std::to_wstring(p.totalWins);
            RECT r3 = { sXL, sY, sXR, sY + sH };
            DrawTextW(hdc, tkWin.c_str(), -1, &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            sY += sH;

            SetTextColor(hdc, ToCOLORREF(Palette::RedNormal));
            std::wstring tkLose = GetText("summary_concede") + std::to_wstring(opponent.totalWins);
            RECT r4 = { sXL, sY, sXR, sY + sH };
            DrawTextW(hdc, tkLose.c_str(), -1, &r4, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            sY += sH;

            SetTextColor(hdc, ToCOLORREF(Palette::BlueDarkest));
            std::wstring tkMatch = GetText("summary_bowin") + std::to_wstring(p.matchWins);
            RECT r5 = { sXL, sY, sXR, sY + sH };
            DrawTextW(hdc, tkMatch.c_str(), -1, &r5, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            };

        bool p1Win = (state->winner == CELL_PLAYER1);
        bool p2Win = (state->winner == CELL_PLAYER2);

        drawSummaryPanel(p1X, state->p1, p1Win, false);
        drawSummaryPanel(p2X, state->p2, p2Win, true);

        int minutes = (int)state->matchDuration / 60;
        int seconds = (int)state->matchDuration % 60;
        wchar_t timeBuffer[64];
        swprintf(timeBuffer, 64, L"%02d:%02d", minutes, seconds);
        std::wstring totalTimeStr = GetText("play_time") + timeBuffer;
        DrawTextCentered(hdc, totalTimeStr, panelY + panelH + UIScaler::SY(30), screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold);

        std::wstring options[] = { GetText("summary_continue"), GetText("summary_save"), GetText("summary_exit") };
        int optY = panelY + panelH + UIScaler::SY(80);
        for (int i = 0; i < 3; i++) {
            COLORREF color = (i == g_SummarySelected) ? ToCOLORREF(Palette::RedNormal) : ToCOLORREF(Palette::GrayDark);
            HFONT font = (i == g_SummarySelected) ? GlobalFont::Bold : GlobalFont::Default;
            std::wstring text = options[i];
            if (i == g_SummarySelected) text = L">> " + text + L" <<";
            DrawTextCentered(hdc, text, optY + i * UIScaler::SY(40), screenWidth, color, font);
        }
        return;
    }

    int availableHeight = screenHeight - UIScaler::SY(120);
    int availableWidth = screenWidth - UIScaler::SX(300);
    int maxBoardSize = availableWidth < availableHeight ? availableWidth : availableHeight;
    int minBoardSize = UIScaler::S(200);
    if (maxBoardSize < minBoardSize) maxBoardSize = minBoardSize;

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

    if (state->status == MATCH_PLAYING && state->isP1Turn) {
        int alpha = (int)(30 + sin(g_GlobalAnimTime * 8.0f) * 30.0f);
        Gdiplus::SolidBrush p1TurnPulse(ToGdiColor(WithAlpha(Theme::P1TurnPulse, (BYTE)alpha)));
        g.FillRectangle(&p1TurnPulse, tabMarginX, startY, leftTabW, boardPixelSize);
        Gdiplus::Pen p1Pen(ToGdiColor(Theme::P1TurnBorder), 3.0f);
        g.DrawRectangle(&p1Pen, tabMarginX, startY, leftTabW, boardPixelSize);
    }

    int avaSize = UIScaler::S(180);
    int avaX_L = tabMarginX + (leftTabW - avaSize) / 2;
    std::wstring p1NameW = state->p1.name;

    Gdiplus::FontFamily fontFamily(L"Arial");
    Gdiplus::Font waterFont(&fontFamily, 64, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush waterBrushL(ToGdiColor(Theme::P1Watermark));
    g.TranslateTransform((Gdiplus::REAL)(avaX_L + avaSize / 2), (Gdiplus::REAL)(startY + UIScaler::SY(80)));
    g.RotateTransform(-30.0f);
    Gdiplus::StringFormat alignCenter;
    alignCenter.SetAlignment(Gdiplus::StringAlignmentCenter);
    alignCenter.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(p1NameW.c_str(), -1, &waterFont, Gdiplus::PointF(0, 0), &alignCenter, &waterBrushL);
    g.ResetTransform();

    DrawPixelAvatar(g, avaX_L, startY + UIScaler::SY(20), avaSize, decodeAvatar(state->p1.avatarPath));

    SetTextColor(hdc, ToCOLORREF(Palette::OrangeNormal));
    SelectObject(hdc, GlobalFont::Bold);
    RECT textRectL1 = { tabMarginX, startY + avaSize + UIScaler::SY(30), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(70) };
    DrawTextW(hdc, p1NameW.c_str(), -1, &textRectL1, DT_CENTER | DT_SINGLELINE);

    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, ToCOLORREF(Palette::White));
    std::wstring p1Piece = GetText("play_piece") + L"X";
    RECT textRectL2 = { tabMarginX, startY + avaSize + UIScaler::SY(60), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(100) };
    DrawTextW(hdc, p1Piece.c_str(), -1, &textRectL2, DT_CENTER | DT_SINGLELINE);

    std::wstring p1Wins = GetText("play_goals") + std::to_wstring(state->p1.totalWins);
    RECT textRectL3 = { tabMarginX, startY + avaSize + UIScaler::SY(90), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(130) };
    DrawTextW(hdc, p1Wins.c_str(), -1, &textRectL3, DT_CENTER | DT_SINGLELINE);

    std::wstring p1Moves = GetText("play_dribble") + std::to_wstring(state->p1.movesCount);
    RECT textRectL4 = { tabMarginX, startY + avaSize + UIScaler::SY(120), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(160) };
    DrawTextW(hdc, p1Moves.c_str(), -1, &textRectL4, DT_CENTER | DT_SINGLELINE);

    std::wstring p1MatchWins = GetText("play_bowins") + std::to_wstring(state->p1.matchWins);
    RECT textRectL5 = { tabMarginX, startY + avaSize + UIScaler::SY(150), tabMarginX + leftTabW, startY + avaSize + UIScaler::SY(190) };
    DrawTextW(hdc, p1MatchWins.c_str(), -1, &textRectL5, DT_CENTER | DT_SINGLELINE);

    {
        static PlayerState p1State;
        p1State.avatarType = decodeAvatar(state->p1.avatarPath);
        p1State.flipH = false;
        std::string nextAction = "idle";
        if (state->status == MATCH_FINISHED) nextAction = (state->winner == CELL_PLAYER1) ? "win" : "sad";
        else if (state->status == MATCH_PLAYING && state->isP1Turn) nextAction = "run";
        if (p1State.currentAction != nextAction) { p1State.currentAction = nextAction; p1State.currentFrame = 0; }
        DrawPixelAction(g, tabMarginX + leftTabW / 2, startY + avaSize + UIScaler::SY(450), UIScaler::S(280), p1State);
    }

    int rightTabStartX = startX + boardPixelSize + UIScaler::SX(10);
    int rightTabW = screenWidth - rightTabStartX - tabMarginX;
    g.FillRectangle(&shadowBrush, rightTabStartX, startY, rightTabW, boardPixelSize);

    if (state->status == MATCH_PLAYING && !state->isP1Turn) {
        int alpha = (int)(30 + sin(g_GlobalAnimTime * 8.0f) * 30.0f);
        Gdiplus::SolidBrush p2TurnPulse(ToGdiColor(WithAlpha(Theme::P2TurnPulse, (BYTE)alpha)));
        g.FillRectangle(&p2TurnPulse, rightTabStartX, startY, rightTabW, boardPixelSize);
        Gdiplus::Pen p2Pen(ToGdiColor(Theme::P2TurnBorder), 3.0f);
        g.DrawRectangle(&p2Pen, rightTabStartX, startY, rightTabW, boardPixelSize);
    }

    int avaX_R = rightTabStartX + (rightTabW - avaSize) / 2;
    std::wstring p2NameW = state->p2.name;

    Gdiplus::SolidBrush waterBrushR(ToGdiColor(Theme::P2Watermark));
    g.TranslateTransform((Gdiplus::REAL)(avaX_R + avaSize / 2), (Gdiplus::REAL)(startY + UIScaler::SY(80)));
    g.RotateTransform(30.0f);
    g.DrawString(p2NameW.c_str(), -1, &waterFont, Gdiplus::PointF(0, 0), &alignCenter, &waterBrushR);
    g.ResetTransform();

    DrawPixelAvatar(g, avaX_R, startY + UIScaler::SY(20), avaSize, decodeAvatar(state->p2.avatarPath));

    SetTextColor(hdc, ToCOLORREF(Palette::CyanNormal));
    SelectObject(hdc, GlobalFont::Bold);
    RECT textRectR1 = { rightTabStartX, startY + avaSize + UIScaler::SY(30), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(70) };
    DrawTextW(hdc, p2NameW.c_str(), -1, &textRectR1, DT_CENTER | DT_SINGLELINE);

    SelectObject(hdc, GlobalFont::Default);
    SetTextColor(hdc, ToCOLORREF(Palette::White));
    std::wstring p2Piece = GetText("play_piece") + L"O";
    RECT textRectR2 = { rightTabStartX, startY + avaSize + UIScaler::SY(60), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(100) };
    DrawTextW(hdc, p2Piece.c_str(), -1, &textRectR2, DT_CENTER | DT_SINGLELINE);

    std::wstring p2Wins = GetText("play_goals") + std::to_wstring(state->p2.totalWins);
    RECT textRectR3 = { rightTabStartX, startY + avaSize + UIScaler::SY(90), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(130) };
    DrawTextW(hdc, p2Wins.c_str(), -1, &textRectR3, DT_CENTER | DT_SINGLELINE);

    std::wstring p2Moves = GetText("play_dribble") + std::to_wstring(state->p2.movesCount);
    RECT textRectR4 = { rightTabStartX, startY + avaSize + UIScaler::SY(120), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(160) };
    DrawTextW(hdc, p2Moves.c_str(), -1, &textRectR4, DT_CENTER | DT_SINGLELINE);

    std::wstring p2MatchWins = GetText("play_bowins") + std::to_wstring(state->p2.matchWins);
    RECT textRectR5 = { rightTabStartX, startY + avaSize + UIScaler::SY(150), rightTabStartX + rightTabW, startY + avaSize + UIScaler::SY(190) };
    DrawTextW(hdc, p2MatchWins.c_str(), -1, &textRectR5, DT_CENTER | DT_SINGLELINE);

    {
        static PlayerState p2State;
        p2State.avatarType = decodeAvatar(state->p2.avatarPath);
        p2State.flipH = true;
        std::string nextAction = "idle";
        if (state->status == MATCH_FINISHED) nextAction = (state->winner == CELL_PLAYER2) ? "win" : "sad";
        else if (state->status == MATCH_PLAYING && !state->isP1Turn) nextAction = "run";
        if (p2State.currentAction != nextAction) { p2State.currentAction = nextAction; p2State.currentFrame = 0; }
        DrawPixelAction(g, rightTabStartX + rightTabW / 2, startY + avaSize + UIScaler::SY(450), UIScaler::S(280), p2State);
    }

    Gdiplus::SolidBrush pitchBrush(ToGdiColor(Theme::BoardPitch));
    g.FillRectangle(&pitchBrush, startX, startY, boardPixelSize, boardPixelSize);
    Gdiplus::Pen pitchBorder(ToGdiColor(Theme::BoardBorder), 3);
    g.DrawRectangle(&pitchBorder, startX, startY, boardPixelSize, boardPixelSize);
    DrawGameBoard(hdc, state, dynamicCellSize, startX, startY);

    std::wstring boText = GetText("play_format") + L"BO" + std::to_wstring(state->targetScore) + L" " + GetText("play_turn") + (state->isP1Turn ? p1NameW : p2NameW);
    COLORREF turnColor = state->isP1Turn ? ToCOLORREF(Palette::RedNormal) : ToCOLORREF(Palette::BlueNormal);
    DrawTextCentered(hdc, boText, UIScaler::SY(10), screenWidth, turnColor, GlobalFont::Bold);

    int timerY = UIScaler::SY(55);
    int shakeX = 0, shakeY = 0;
    COLORREF timerColor = ToCOLORREF(Palette::CyanLight);
    bool isWarning = (state->matchType != MATCH_PVE && state->timeRemaining <= 3);

    if (isWarning) {
        float shakeFreq = 25.0f;
        float shakeAmp = (float)UIScaler::S(4);
        shakeX = (int)(sin(g_GlobalAnimTime * shakeFreq) * shakeAmp);
        shakeY = (int)(cos(g_GlobalAnimTime * shakeFreq * 0.7f) * shakeAmp);
        timerColor = ToCOLORREF(Palette::RedNormal);
    }

    int clockSize = UIScaler::S(34);
    std::wstring timeText;
    if (state->matchType == MATCH_PVE) {
        int minutes = (int)state->matchDuration / 60;
        int seconds = (int)state->matchDuration % 60;
        wchar_t buffer[64];
        swprintf(buffer, 64, L"%02d:%02d", minutes, seconds);
        timeText = GetText("play_time") + buffer;
    }
    else {
        timeText = L"THỜI GIAN: " + std::to_wstring(state->timeRemaining) + L"s";
    }

    int textEstimatedW = UIScaler::SX((int)timeText.length() * 12 + 20);
    int groupTotalW = clockSize + UIScaler::SX(15) + textEstimatedW;
    int groupStartX = (screenWidth - groupTotalW) / 2;

    float pulse = 0.7f + sin(g_GlobalAnimTime * 8.0f) * 0.3f;
    BYTE neonA = (BYTE)(150 + pulse * 105);
    Gdiplus::Color neonColor = isWarning ? Gdiplus::Color(neonA, 255, 0, 0) : Gdiplus::Color(neonA, 0, 200, 255);
    DrawPixelClock(g, groupStartX + shakeX + clockSize / 2, timerY + shakeY + UIScaler::SY(28), clockSize, neonColor);

    HFONT oldT = (HFONT)SelectObject(hdc, GlobalFont::Bold);
    SetTextColor(hdc, timerColor);
    RECT rTimer = { groupStartX + shakeX + clockSize + UIScaler::SX(10), timerY + shakeY + UIScaler::SY(8), screenWidth, timerY + shakeY + UIScaler::SY(50) };
    DrawTextW(hdc, timeText.c_str(), -1, &rTimer, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);
    SelectObject(hdc, oldT);

    if (state->matchType == MATCH_PVE) {
        DrawTextCentered(hdc, GetText("play_undo_hint"), screenHeight - UIScaler::SY(40), screenWidth, ToCOLORREF(Palette::GrayNormal), GlobalFont::Default);
    }
    SelectObject(hdc, hOldFont);

    if (state->status == MATCH_PAUSED) {
        Gdiplus::SolidBrush shadowBrush2(Gdiplus::Color(100, 255, 255, 255));
        g.FillRectangle(&shadowBrush2, 0, 0, screenWidth, screenHeight);

        int menuW = UIScaler::SX(580), menuH = UIScaler::SY(500);
        int menuX = (screenWidth - menuW) / 2;
        int menuY = (screenHeight - menuH) / 2;

        Gdiplus::SolidBrush bgBrush(ToGdiColor(Theme::GlassWhite));
        g.FillRectangle(&bgBrush, menuX, menuY, menuW, menuH);

        Gdiplus::Pen yellowBorder(ToGdiColor(Theme::PanelYellowBorder), 4.0f);
        g.DrawRectangle(&yellowBorder, menuX, menuY, menuW, menuH);

        static std::map<int, Gdiplus::Color> waterPalette;
        if (waterPalette.empty()) {
            waterPalette[1] = Gdiplus::Color(30, 0, 0, 0);
            waterPalette[2] = Gdiplus::Color(40, 255, 200, 0);
            waterPalette[3] = Gdiplus::Color(40, 255, 255, 255);
        }
        static PixelModel whistleMod = LoadPixelModel("Asset/models/bg/whistle.txt");
        DrawPixelModel(g, whistleMod, screenWidth / 2, screenHeight / 2, UIScaler::S(250), waterPalette);

        if (g_CurrentSubMenu == SUB_MAIN) {
            DrawPixelBanner(g, hdc, GetText("pause_title").c_str(), screenWidth / 2, menuY + UIScaler::SY(40),
                menuW - UIScaler::SX(20), ToCOLORREF(Palette::White), RGB(255, 180, 0), "Asset/models/bg/whistle.txt");

            std::wstring labels[] = { GetText("pause_resume"), GetText("pause_bgm"), GetText("pause_vol"), GetText("pause_save"), GetText("pause_exit") };
            for (int i = 0; i < TOTAL_PAUSE_ITEMS; i++) {
                std::wstring itemText = labels[i];
                COLORREF color = ToCOLORREF(Palette::GrayDarkest);
                HFONT font = GlobalFont::Default;
                bool isDisabled = (i == 2 && !config->isBgmEnabled);

                if (i == 1) itemText += (config->isBgmEnabled ? L" [ " + GetText("btn_on") + L" ]" : L" [ " + GetText("btn_off") + L" ]");

                if (i == g_PauseSelected && !isDisabled) {
                    int wave = (int)(180 + sin(g_GlobalAnimTime * 10.0f) * 75);
                    color = RGB(255, wave, 0);
                    font = GlobalFont::Bold;
                    if (i != 2) itemText = L">> " + itemText + L" <<";
                }

                if (isDisabled) {
                    color = RGB(150, 150, 150);
                    font = GlobalFont::Default;
                    itemText = labels[i] + L" [ ĐÃ KHÓA ]";
                }

                int itemY = menuY + UIScaler::SY(140) + i * UIScaler::SY(55);
                DrawTextCentered(hdc, (i == 2) ? labels[i] : itemText, itemY, screenWidth, color, font);

                if (i == 2) {
                    int barW = UIScaler::SX(220);
                    int barH = UIScaler::SY(8);
                    int barX = (screenWidth - barW) / 2;
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

                    if (i == g_PauseSelected && !isDisabled) {
                        Gdiplus::Pen aura(Gdiplus::Color(150, 255, 200, 0), 2);
                        g.DrawEllipse(&aura, handleX - 10, barY - 6, 20, 20);
                    }
                }
            }
        }
        else if (g_CurrentSubMenu == SUB_SAVE_SELECT) {
            DrawTextCentered(hdc, GetText("save_title"), menuY + UIScaler::SY(40), screenWidth, ToCOLORREF(Palette::OrangeNormal), GlobalFont::Title);

            for (int i = 0; i < MAX_SAVE_SLOTS; i++) {
                std::wstring displayName = GetSaveDisplayName(i + 1);
                bool exists = CheckSaveExists(i + 1);
                if (exists) {
                    if (displayName.length() > 20) displayName = displayName.substr(0, 17) + L"...";
                }
                else {
                    displayName = GetText("save_empty");
                }

                std::wstring slotLabel = L"Slot " + std::to_wstring(i + 1) + L": " + displayName;
                COLORREF color = (i == g_SaveSlotSelected) ? ToCOLORREF(Palette::BlueDarkest) : ToCOLORREF(Palette::GrayDarkest);
                HFONT font = (i == g_SaveSlotSelected) ? GlobalFont::Bold : GlobalFont::Default;

                if (i == g_SaveSlotSelected) slotLabel = L"-> " + slotLabel + L" <-";
                DrawTextCentered(hdc, slotLabel, menuY + UIScaler::SY(130) + i * UIScaler::SY(55), screenWidth, color, font);
            }
            DrawTextCentered(hdc, GetText("save_confirm"), menuY + UIScaler::SY(430), screenWidth, ToCOLORREF(Palette::GreenNormal), GlobalFont::Default);
            DrawTextCentered(hdc, GetText("save_back"), menuY + UIScaler::SY(460), screenWidth, ToCOLORREF(Palette::GrayDark), GlobalFont::Note);
        }
        else if (g_CurrentSubMenu == SUB_SAVE_NAME_ENTRY) {
            DrawTextCentered(hdc, GetText("save_title"), menuY + UIScaler::SY(60), screenWidth, ToCOLORREF(Palette::CyanNormal), GlobalFont::Title);

            int boxW = UIScaler::SX(350), boxH = UIScaler::SY(50);
            int boxX = (screenWidth - boxW) / 2;
            int boxY = menuY + UIScaler::SY(200);

            HBRUSH hBoxBrush = CreateSolidBrush(RGB(240, 240, 250));
            RECT rectBox = { boxX, boxY, boxX + boxW, boxY + boxH };
            FillRect(hdc, &rectBox, hBoxBrush);
            DeleteObject(hBoxBrush);

            Gdiplus::Pen tbPen(ToGdiColor(WithAlpha(Theme::TitleBorder, 180)), 2.0f);
            g.DrawRectangle(&tbPen, boxX, boxY, boxW, boxH);

            DrawTextCentered(hdc, GetText("play_save_label"), boxY - UIScaler::SY(35), screenWidth, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold);

            extern float g_GlobalAnimTime;
            bool showCursor = ((int)(g_GlobalAnimTime * 2.5f) % 2 == 0);
            std::wstring displayText = g_SaveNameInput + (showCursor ? L"_" : L" ");

            RECT rInput = { boxX, boxY, boxX + boxW, boxY + boxH + UIScaler::SY(8) };
            SetTextColor(hdc, ToCOLORREF(Palette::GrayDarkest));
            SelectObject(hdc, GlobalFont::Bold);
            DrawTextW(hdc, displayText.c_str(), -1, &rInput, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

            if (!g_SaveStatusMsg.empty()) {
                COLORREF msgColor = (g_SaveStatusMsg == GetText("msg_save_success")) ? ToCOLORREF(Palette::GreenNormal) : ToCOLORREF(Palette::RedNormal);
                DrawTextCentered(hdc, g_SaveStatusMsg, boxY + boxH + UIScaler::SY(20), screenWidth, msgColor, GlobalFont::Bold);
            }

            DrawTextCentered(hdc, GetText("play_save_hint1"), menuY + UIScaler::SY(380), screenWidth, ToCOLORREF(Palette::GreenNormal), GlobalFont::Default);
            DrawTextCentered(hdc, GetText("play_save_hint2"), menuY + UIScaler::SY(430), screenWidth, ToCOLORREF(Palette::RedNormal), GlobalFont::Note);
        }
    }
    else if (state->status == MATCH_FINISHED) {
        std::wstring winMsg;
        COLORREF winColor;
        COLORREF winGlow;

        int winRequired = state->targetScore / 2 + 1;
        bool matchOver = (state->p1.totalWins >= winRequired || state->p2.totalWins >= winRequired);

        if (state->winner == CELL_PLAYER1) {
            winMsg = matchOver ? (L"🏆 " + p1NameW + GetText("play_win_cup") + L" 🏆") : (L"⚽ " + p1NameW + GetText("play_win_goal") + L" ⚽");
            winColor = ToCOLORREF(Palette::OrangeNormal);
            winGlow = RGB(255, 120, 0);
        }
        else if (state->winner == CELL_PLAYER2) {
            winMsg = matchOver ? (L"🏆 " + p2NameW + GetText("play_win_cup") + L" 🏆") : (L"⚽ " + p2NameW + GetText("play_win_goal") + L" ⚽");
            winColor = ToCOLORREF(Palette::CyanNormal);
            winGlow = RGB(0, 150, 255);
        }
        else {
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

        if (g_WinAnimTime > 0.5f) {
            extern float g_GlobalAnimTime;
            int blinkAlpha = (int)(155 + sin(g_GlobalAnimTime * 6.0f) * 100);
            COLORREF dynColor = RGB(blinkAlpha, blinkAlpha, blinkAlpha);
            DrawTextCentered(hdc, GetText("play_win_hint"), msgY + UIScaler::SY(52), screenWidth, dynColor, GlobalFont::Note);
        }
    }
}

void UpdatePlayScreen(PlayState* state, ScreenState& currentState, WPARAM wParam, GameConfig* config) {
    if (wParam == 0) return;
    ProcessPlayInput(wParam, state, currentState, config);
}

void ResetPlayScreenStatics() {
    g_CurrentSubMenu = SUB_MAIN;
    g_PauseSelected = 0;
    g_SaveSlotSelected = 0;
    g_SaveNameInput = L"";
}