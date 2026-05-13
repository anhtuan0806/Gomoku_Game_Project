#include "MatchConfigScreen.h"
#include "../RenderAPI/UIComponents.h"
#include "../RenderAPI/UIScaler.h"
#include "../RenderAPI/Colours.h"
#include "../RenderAPI/PixelLayout.h"
#include "../GameLogic/GameEngine.h"
#include "../SystemModules/Localization.h"
#include "../SystemModules/AudioSystem.h"
#include "../GameLogic/PlayerEngineer.h"
#include <cmath>
#include <vector>

/** @file MatchConfigScreen.cpp
 *  @brief Màn cấu hình trận đấu: xử lý input, kiểm tra tên, và vẽ giao diện cấu hình.
 *
 *  Hàm chính:
 *  - `ValidateNames`: kiểm tra và chuẩn hoá tên người chơi, đặt `validationMsg` nếu có lỗi.
 *  - `UpdateMatchConfigScreen`: xử lý phím người dùng (di chuyển menu, edit tên, xác nhận).
 *  - `RenderMatchConfigScreen`: vẽ hai trang cấu hình (tuỳ chọn trận và thẻ người chơi).
 */

static int currentPage = 0; // 0 = Lên Khuôn Trận Đấu, 1 = Lên Sơ Đồ Cầu Thủ

const int PAGE_0_ITEMS = 6;
const int PAGE_1_ITEMS = 6;

static std::wstring editName1 = L"Player 1";
static std::wstring editName2 = L"Player 2";
static int sPlayer1AvatarIndex = 0;
static int sPlayer2AvatarIndex = 1;

// Avatar names are now localized via GetText("config_avatar_1/2/3")
// Mapping: slot 0-2 -> avatarType (0,1,2)
static const int AVATAR_SLOT_TO_TYPE[3] = {0, 1, 2};
const int TOTAL_HUMAN_AVATARS = 3;

static bool isEditingName1 = false;
static bool isEditingName2 = false;
static std::wstring validationMsg = L"";

/** @brief Kiểm tra và chuẩn hoá tên người chơi.
 *  @param playState Trạng thái cấu hình trận đấu (dùng để xác định PvE/PvP).
 *  @return `true` nếu tên hợp lệ; nếu trả về `false` thì `validationMsg` được đặt thông báo.
 *  @note Hàm có thể tự động điền tên mặc định nếu trường tên để trống và giới hạn độ dài tối đa.
 */
bool ValidateNames(PlayState *playState)
{
    bool isPvE = (playState->matchType == MATCH_PVE);

    // Auto-fill logic instead of erroring
    if (editName1.empty())
    {
        editName1 = L"Player 1";
    }

    // Unify character limit to 1-15 (consistent with Save name)
    if (editName1.length() > 15)
    {
        validationMsg = GetText("config_err_p1_len");
        return false;
    }

    if (!isPvE)
    {
        if (editName2.empty())
        {
            editName2 = L"Player 2";
        }
        if (editName2.length() > 15)
        {
            validationMsg = GetText("config_err_p2_len");
            return false;
        }
        if (editName1 == editName2 && editName1 != L"Player 1")
        {
            validationMsg = GetText("config_err_duplicate");
            return false;
        }
    }
    validationMsg = L"";
    return true;
}

bool UpdateMatchConfigScreen(ScreenState &currentState, PlayState *playState, int &selectedOption, WPARAM wParam)
{
    if (wParam == 0)
    {
        return false;
    }

    bool changed = true;

    bool isChar = (wParam & 0x10000);
    wchar_t ch = (wchar_t)(wParam & 0xFFFF);

    if (!isChar)
        validationMsg = L""; // Xóa lỗi khi người dùng thao tác phím điều hướng

    // Trường hợp: đang ở chế độ chỉnh sửa tên (edit mode)
    if (isEditingName1 || isEditingName2)
    {
        std::wstring &target = isEditingName1 ? editName1 : editName2;

        if (!isChar)
        {
            if (wParam == VK_RETURN || wParam == VK_ESCAPE)
            {
                isEditingName1 = false;
                isEditingName2 = false;
                ValidateNames(playState); // Kiểm tra ngay sau khi đặt tên
                return true;
            }
            if (wParam == VK_BACK)
            {
                if (!target.empty())
                    target.pop_back();
                return true;
            }
            // Ngăn chặn các phím di chuyển (W,A,S,D, Up, Down...) bị lọt xuống dưới khi đang edit
            return false;
        }
        else
        {
            // Nhận ký tự Unicode trực tiếp từ WM_CHAR (UniKey đã xử lý dấu)
            if (ch >= 32 && target.length() < 15)
            {
                target += ch;
            }
            return true;
        }
    }

    // Trường hợp: thao tác điều hướng menu và chọn tuỳ chọn
    if (isChar)
        return false; // Không xử lý WM_CHAR khi không edit

    bool isRepeat = (wParam & 0x20000) != 0;
    WPARAM rawKey = wParam & 0xFFFF;

    // Throttling: Giới hạn 80ms cho phím nhấn tay, 150ms cho phím giữ (Repeat)
    static ULONGLONG lastMoveTime = 0;
    ULONGLONG now = GetTickCount64();
    bool canMove = (now - lastMoveTime > (ULONGLONG)(isRepeat ? 150 : 80));

    int totalItems = (currentPage == 0) ? PAGE_0_ITEMS : PAGE_1_ITEMS;

    if (rawKey == 'W' || rawKey == 'w' || rawKey == VK_UP)
    {
        if (!canMove)
            return false;

        if (currentPage == 0)
        {
            // Trang 0: điều hướng tuần tự, bỏ qua item bị ẩn
            do
            {
                selectedOption = (selectedOption - 1 + totalItems) % totalItems;
            } while (
                (selectedOption == 2 && playState->matchType == MATCH_PVP) ||
                (selectedOption == 3 && playState->matchType == MATCH_PVE));
        }
        else
        {
            // Trang 1: điều hướng 2 cột
            // Cột trái: 0 → 1 → 4,  Cột phải: 2 → 3 → 5
            bool isPvE = (playState->matchType == MATCH_PVE);
            switch (selectedOption)
            {
            case 0: selectedOption = 4; break;  // P1 Avatar → Back
            case 1: selectedOption = 0; break;  // P1 Name → P1 Avatar
            case 2: selectedOption = 5; break;  // P2 Avatar → Start
            case 3: selectedOption = 2; break;  // P2 Name → P2 Avatar
            case 4: selectedOption = 1; break;  // Back → P1 Name
            case 5: selectedOption = isPvE ? 5 : 3; break;  // Start → P2 Name (hoặc giữ nguyên nếu PvE)
            }
        }

        if (!isRepeat)
            playSfx("sfx_move");
        lastMoveTime = now;
    }
    else if (rawKey == 'S' || rawKey == 's' || rawKey == VK_DOWN)
    {
        if (!canMove)
            return false;

        if (currentPage == 0)
        {
            do
            {
                selectedOption = (selectedOption + 1) % totalItems;
            } while (
                (selectedOption == 2 && playState->matchType == MATCH_PVP) ||
                (selectedOption == 3 && playState->matchType == MATCH_PVE));
        }
        else
        {
            // Trang 1: điều hướng 2 cột
            bool isPvE = (playState->matchType == MATCH_PVE);
            switch (selectedOption)
            {
            case 0: selectedOption = 1; break;  // P1 Avatar → P1 Name
            case 1: selectedOption = 4; break;  // P1 Name → Back
            case 2: selectedOption = 3; break;  // P2 Avatar → P2 Name
            case 3: selectedOption = 5; break;  // P2 Name → Start
            case 4: selectedOption = 0; break;  // Back → P1 Avatar
            case 5: selectedOption = isPvE ? 5 : 2; break;  // Start → P2 Avatar (hoặc giữ nguyên nếu PvE)
            }
        }

        if (!isRepeat)
            playSfx("sfx_move");
        lastMoveTime = now;
    }

    // Trang 0: VK_LEFT/VK_RIGHT hoạt động như A/D (thay đổi giá trị)
    // Trang 1: VK_LEFT/VK_RIGHT + A/D đều dùng để di chuyển giữa 2 cột
    int adjustDir = 0;
    int navDir = 0;

    if (currentPage == 0)
    {
        // Trang 0: cả A/D và Left/Right đều thay đổi giá trị
        adjustDir = (rawKey == 'D' || rawKey == 'd' || rawKey == VK_RIGHT) ? 1
                  : ((rawKey == 'A' || rawKey == 'a' || rawKey == VK_LEFT) ? -1 : 0);
    }
    else
    {
        // Trang 1: A/D thay đổi giá trị (avatar), Left/Right điều hướng cột
        adjustDir = (rawKey == 'D' || rawKey == 'd') ? 1 : ((rawKey == 'A' || rawKey == 'a') ? -1 : 0);
        navDir = (rawKey == VK_RIGHT) ? 1 : ((rawKey == VK_LEFT) ? -1 : 0);
    }

    if ((adjustDir != 0 || navDir != 0) && !canMove)
        return false; // Throttling cho cả việc đổi giá trị và di chuyển trái/phải
    if (adjustDir != 0 || navDir != 0)
        lastMoveTime = now;

    if (currentPage == 1 && navDir != 0)
    {
        bool isPvE = (playState->matchType == MATCH_PVE);
        int nextOption = selectedOption;

        // Chuyển cột: hàng tương ứng
        // Left col (0,1,4) ↔ Right col (2,3,5)
        if (navDir > 0) // Sang phải
        {
            switch (selectedOption)
            {
            case 0: nextOption = isPvE ? 0 : 2; break;
            case 1: nextOption = isPvE ? 1 : 3; break;
            case 4: nextOption = 5; break;
            }
        }
        else // Sang trái
        {
            switch (selectedOption)
            {
            case 2: nextOption = 0; break;
            case 3: nextOption = 1; break;
            case 5: nextOption = 4; break;
            }
        }

        if (nextOption != selectedOption)
        {
            selectedOption = nextOption;
            if (!isRepeat)
                playSfx("sfx_move");
        }
        return true;
    }

    if (currentPage == 0)
    {
        switch (selectedOption)
        {
        case 0: // Chế độ
            if (adjustDir != 0)
            {
                playState->gameMode = (playState->gameMode == MODE_CARO) ? MODE_TIC_TAC_TOE : MODE_CARO;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 1: // PvP / PvE
            if (adjustDir != 0)
            {
                playState->matchType = (playState->matchType == MATCH_PVP) ? MATCH_PVE : MATCH_PVP;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 2: // Độ khó
            if (playState->matchType == MATCH_PVE && adjustDir != 0)
            {
                playState->difficulty += adjustDir;
                if (playState->difficulty < BOT_DIFFICULTY_MIN)
                    playState->difficulty = BOT_DIFFICULTY_MAX;
                if (playState->difficulty > BOT_DIFFICULTY_MAX)
                    playState->difficulty = BOT_DIFFICULTY_MIN;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 3: // Thời gian
            if (adjustDir != 0)
            {
                playState->countdownTime += adjustDir * COUNTDOWN_STEP_SECONDS;
                if (playState->countdownTime < COUNTDOWN_MIN_SECONDS)
                    playState->countdownTime = COUNTDOWN_MIN_SECONDS;
                if (playState->countdownTime > COUNTDOWN_MAX_SECONDS)
                    playState->countdownTime = COUNTDOWN_MAX_SECONDS;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 4: // Bo
            if (adjustDir != 0)
            {
                playState->targetScore += adjustDir * TARGET_SCORE_STEP;
                if (playState->targetScore < TARGET_SCORE_MIN)
                    playState->targetScore = TARGET_SCORE_MAX;
                if (playState->targetScore > TARGET_SCORE_MAX)
                    playState->targetScore = TARGET_SCORE_MIN;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 5: // Tiếp Theo
            if (wParam == VK_RETURN || wParam == VK_SPACE)
            {
                currentPage = 1;
                selectedOption = 0;
                playSfx("sfx_select");
            }
            break;
        }
    }
    else
    {
        bool isPvE = (playState->matchType == MATCH_PVE);
        switch (selectedOption)
        {
        case 0: // Đổi Avatar P1 (A/D)
            if (adjustDir != 0)
            {
                sPlayer1AvatarIndex = (sPlayer1AvatarIndex + adjustDir + TOTAL_HUMAN_AVATARS) % TOTAL_HUMAN_AVATARS;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 1: // Sửa Tên P1 (Enter)
            if (wParam == VK_RETURN)
            {
                isEditingName1 = true;
                editName1 = L""; // Xóa để nhập mới cho lẹ
                playSfx("sfx_select");
            }
            break;
        case 2: // Đổi Avatar P2 (A/D)
            if (!isPvE && adjustDir != 0)
            {
                sPlayer2AvatarIndex = (sPlayer2AvatarIndex + adjustDir + TOTAL_HUMAN_AVATARS) % TOTAL_HUMAN_AVATARS;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 3: // Sửa Tên P2 (Enter)
            if (!isPvE && wParam == VK_RETURN)
            {
                isEditingName2 = true;
                editName2 = L"";
                playSfx("sfx_select");
            }
            break;
        case 4: // Quay lại
            if (wParam == VK_RETURN || wParam == VK_SPACE)
            {
                currentPage = 0;
                selectedOption = 5;
                if (!isRepeat)
                    playSfx("sfx_move");
            }
            break;
        case 5: // Bắt Đầu
            if (wParam == VK_RETURN || wParam == VK_SPACE)
            {
                if (!ValidateNames(playState))
                {
                    playSfx("sfx_error");
                    return true;
                }

                // Nếu hợp lệ thì tiến hành vào trận thông qua PlayerEngineer
                static const char *SLOT_PATHS[3] = {"avatar_0", "avatar_1", "avatar_2"};
                initializePlayer(playState->player1, editName1, SLOT_PATHS[sPlayer1AvatarIndex], 'X', (float)playState->countdownTime);

                if (isPvE)
                {
                    std::wstring botName = (playState->difficulty == 1) ? GetText("config_bot_easy") : (playState->difficulty == 2 ? GetText("config_bot_med") : GetText("config_bot_hard"));
                    initializePlayer(playState->player2, botName, "avatar_0", 'O', (float)playState->countdownTime);
                }
                else
                {
                    initializePlayer(playState->player2, editName2, SLOT_PATHS[sPlayer2AvatarIndex], 'O', (float)playState->countdownTime);
                }

                int bSize = (playState->gameMode == MODE_CARO) ? 15 : 3;
                initializeNewMatch(playState, playState->gameMode, playState->matchType, bSize, playState->countdownTime, playState->difficulty, playState->targetScore, 15);

                stopBgm(); // Tắt nhạc menu
                playSfx("sfx_whistle");

                currentPage = 0;
                selectedOption = 0;
                currentState = SCREEN_PLAY;
            }
            break;
        }
    }
    return changed;
}

/** @brief Vẽ một ô văn bản (căn theo cột) với font và alignment xác định.
 *  @param hdc Device context để vẽ.
 *  @param text Nội dung cần vẽ.
 *  @param x Tọa độ trái của ô.
 *  @param y Tọa độ trên của ô.
 *  @param width Chiều rộng ô.
 *  @param color Màu chữ (COLORREF).
 *  @param font Font sử dụng (HFONT).
 *  @param format Cờ định dạng DrawText (ví dụ DT_LEFT/DT_RIGHT/DT_CENTER).
 */
void DrawColText(HDC hdc, const std::wstring &text, int x, int y, int width, COLORREF color, HFONT font, UINT format, bool useOutline = true, COLORREF outlineColor = RGB(255, 255, 255))
{
    // Tăng vùng đệm chiều cao từ 42 lên 80 để các Font to (GlobalFont::Title) không bị xén đỉnh/đáy
    RECT rect = {x, y, x + width, y + UIScaler::SY(80)};

    if (useOutline)
    {
        DrawTextOutlined(hdc, text, rect, color, outlineColor, font, format | DT_VCENTER | DT_SINGLELINE);
    }
    else
    {
        SetTextColor(hdc, color);
        HFONT oldFont = (HFONT)SelectObject(hdc, font);
        SetBkMode(hdc, TRANSPARENT);
        DrawTextW(hdc, text.c_str(), -1, &rect, format | DT_VCENTER | DT_SINGLELINE);
        SelectObject(hdc, oldFont);
    }
}

/** @brief Vẽ toàn bộ màn hình cấu hình trận đấu.
 *  @param hdc Device context để vẽ.
 *  @param selectedOption Mục đang được chọn (dùng để highlight và xử lý input).
 *  @param config Trạng thái cấu hình (PlayState) để hiển thị giá trị hiện tại.
 *  @param screenWidth Chiều rộng vùng vẽ.
 *  @param screenHeight Chiều cao vùng vẽ.
 *  @note Hàm vẽ hai trang: trang 0 là các tuỳ chọn trận đấu; trang 1 là thẻ người chơi (avatar/tên).
 */
void RenderMatchConfigScreen(HDC hdc, int selectedOption, const PlayState *config, int screenWidth, int screenHeight)
{
    Gdiplus::Graphics g(hdc);
    PixelLayout::ApplyPixelArtBlit(g);

    DrawProceduralStadium(g, screenWidth, screenHeight);

    // Khung Trắng Kính (White Glassmorphism)
    Gdiplus::SolidBrush whitePanel(ToGdiColor(Theme::GlassWhite));
    int panelW = UIScaler::SX(750);
    int panelH = UIScaler::SY(500);
    int panelX = (screenWidth - panelW) / 2;
    int panelY = (screenHeight - panelH) / 2 - UIScaler::SY(10);
    PixelLayout::AlignRectToPixelGrid(panelX, panelY, panelW, panelH);

    g.FillRectangle(&whitePanel, panelX, panelY, panelW, panelH);

    Gdiplus::Pen panelPen(ToGdiColor(Theme::PanelGreenBorder), 3.0f);
    g.DrawRectangle(&panelPen, panelX, panelY, panelW, panelH);

    if (currentPage == 0)
    {
        DrawPixelBanner(g, hdc, GetText("config_page1").c_str(), screenWidth / 2, panelY + UIScaler::SY(50), panelW - UIScaler::SX(40), ToCOLORREF(Palette::White), ToCOLORREF(Theme::BannerGlowBlue), "Asset/models/bg/gears.txt");

        int col1X = panelX + UIScaler::SX(30);
        int col1W = UIScaler::SX(320);
        int col2X = panelX + UIScaler::SX(370);
        int col2W = UIScaler::SX(350);
        int startY = panelY + UIScaler::SY(120);
        int spacing = UIScaler::SY(50);

        std::wstring labels[] = {
            GetText("config_size"), GetText("config_type"),
            GetText("config_diff"), GetText("config_time"), GetText("config_bo")};

        std::wstring values[] = {
            std::wstring(config->gameMode == MODE_CARO ? L"< " + GetText("config_val_caro") + L" >" : L"< " + GetText("config_val_ttt") + L" >"),
            std::wstring(config->matchType == MATCH_PVP ? L"< " + GetText("val_pvp") + L" >" : L"< " + GetText("val_pve") + L" >"),
            std::wstring(config->difficulty == 1 ? L"< " + GetText("val_bronze") + L" >" : (config->difficulty == 2 ? L"< " + GetText("val_gold") + L" >" : L"< " + GetText("val_challenger") + L" >")),
            L"< " + std::to_wstring(config->countdownTime) + L" " + GetText("val_sec") + L" >",
            L"< " + GetText("val_first_to") + L" " + std::to_wstring(config->targetScore) + L" " + GetText("val_goals") + L" >"};

        bool isPvE_p0 = (config->matchType == MATCH_PVE);
        int drawRow = 0; // Hàng thực tế được vẽ
        for (int i = 0; i < 5; i++)
        {
            if (i == 3 && isPvE_p0)
                continue;
            if (i == 2 && config->matchType == MATCH_PVP)
                continue;

            int yPos = startY + drawRow * spacing;
            COLORREF valColor = ToCOLORREF(Palette::GrayDarkest);
            bool isSelected = (i == selectedOption);

            if (isSelected)
            {
                int rCol = (int)(180 + PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 12.f) * 75);
                valColor = RGB(255, max(0, min(255, 255 - rCol)), 0);
            }

            HFONT fontVal = isSelected ? GlobalFont::Bold : GlobalFont::Default;
            DrawColText(hdc, labels[i], col1X, yPos, col1W, ToCOLORREF(Palette::GrayDarkest), GlobalFont::Bold, DT_RIGHT);
            DrawColText(hdc, values[i], col2X, yPos, col2W, valColor, fontVal, DT_LEFT);
            drawRow++;
        }

        COLORREF nextColor = ToCOLORREF(Palette::BlueDarkest);
        if (selectedOption == 5)
        {
            int gCol = (int)(150 + PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 15.f) * 105);
            nextColor = RGB(max(0, min(255, 255 - gCol)), 100, 255);
        }
        DrawTextCentered(hdc, GetText("config_next").c_str(), startY + 5 * spacing + UIScaler::SY(40), screenWidth, nextColor, (selectedOption == 5) ? GlobalFont::Title : GlobalFont::Bold);
    }
    else
    {
        DrawPixelBanner(g, hdc, GetText("config_page2").c_str(), screenWidth / 2, panelY + UIScaler::SY(50), panelW - UIScaler::SX(40), ToCOLORREF(Palette::White), ToCOLORREF(Theme::BannerGlowOrange), "Asset/models/bg/cup.txt");

        int cardW = panelW / 2 - UIScaler::SX(40);
        int cardH = UIScaler::SY(320);
        int cardY = panelY + UIScaler::SY(90);
        int avaSize = UIScaler::S(160);
        bool isPvE = (config->matchType == MATCH_PVE);

        auto drawPlayerCard = [&](int x, int avatarIdx, const std::wstring &name, bool isP1, bool isSelected_Ava, bool isSelected_Name, bool isEditing)
        {
            bool isPvE_Bot = (!isP1 && isPvE);
            int curType = isPvE_Bot ? 0 : AVATAR_SLOT_TO_TYPE[avatarIdx];

            Gdiplus::SolidBrush cardBg(isP1 ? Gdiplus::Color(220, 255, 240, 230) : Gdiplus::Color(220, 230, 245, 255));
            g.FillRectangle(&cardBg, x, cardY, cardW, cardH);

            if (isSelected_Ava || isSelected_Name)
            {
                int alpha = (int)(60 + PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 8.f) * 40);
                Gdiplus::Color glowCol = isP1 ? ToGdiColor(WithAlpha(Palette::OrangeNormal, (BYTE)alpha)) : ToGdiColor(WithAlpha(Palette::CyanNormal, (BYTE)alpha));
                Gdiplus::SolidBrush glowB(glowCol);
                float glowSize = avaSize * 1.1f;
                g.FillEllipse(&glowB, (Gdiplus::REAL)(x + cardW / 2.0f - glowSize / 2.0f - UIScaler::SX(4)), (Gdiplus::REAL)(cardY + UIScaler::SY(110) - glowSize / 2.0f), (Gdiplus::REAL)glowSize, (Gdiplus::REAL)glowSize);
            }

            DrawPixelAvatar(g, x + (cardW - avaSize) / 2, cardY + UIScaler::SY(30), avaSize, curType);

            COLORREF avaCol = isSelected_Ava ? (isP1 ? ToCOLORREF(Theme::P1AccentSelected) : ToCOLORREF(Theme::P2AccentSelected)) : ToCOLORREF(Palette::GrayDarkest);
            int textY = cardY + UIScaler::SY(195);

            std::string avatarKey = "config_avatar_" + std::to_string(avatarIdx + 1);
            std::wstring avaName = isPvE_Bot ? L"[ " + GetText("val_locked") + L" ]" : GetText(avatarKey);
            DrawColText(hdc, avaName, x, textY, cardW, avaCol, isSelected_Ava ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER, false);
            if (!isPvE_Bot)
            {
                std::wstring slotStr = L"< " + std::to_wstring(avatarIdx + 1) + L"/" + std::to_wstring(TOTAL_HUMAN_AVATARS) + L" >";
                DrawColText(hdc, slotStr, x, textY + UIScaler::SY(28), cardW, avaCol, GlobalFont::Note, DT_CENTER, false);
            }

            COLORREF nameCol = isSelected_Name ? (isP1 ? ToCOLORREF(Theme::P1NameSelected) : ToCOLORREF(Theme::P2NameSelected)) : ToCOLORREF(Palette::GrayDarkest);
            bool showCursor = isEditing && ((int)(g_GlobalAnimTime * 3.0f) % 2 == 0);
            std::wstring dispName = name + (isEditing ? (showCursor ? L"_" : L" ") : L"");
            DrawColText(hdc, GetText("config_pname") + L" " + dispName, x, textY + UIScaler::SY(50), cardW, nameCol, isSelected_Name ? GlobalFont::Bold : GlobalFont::Default, DT_CENTER);

            Gdiplus::Pen cardBorder(isSelected_Ava || isSelected_Name ? (isP1 ? ToGdiColor(Palette::OrangeNormal) : ToGdiColor(Palette::CyanNormal)) : Gdiplus::Color(100, 150, 150, 150), 3.0f);
            g.DrawRectangle(&cardBorder, x, cardY, cardW, cardH);
        };

        std::wstring botName = L"";
        if (isPvE)
        {
            if (config->difficulty == 1)
                botName = GetText("config_bot_easy");
            else if (config->difficulty == 2)
                botName = GetText("config_bot_med");
            else
                botName = GetText("config_bot_hard");
        }

        drawPlayerCard(panelX + UIScaler::SX(25), sPlayer1AvatarIndex, editName1, true, selectedOption == 0, selectedOption == 1, isEditingName1);
        drawPlayerCard(panelX + panelW / 2 + UIScaler::SX(15), sPlayer2AvatarIndex, isPvE ? botName : editName2, false, selectedOption == 2, selectedOption == 3, isEditingName2);

        int botY = panelY + panelH - UIScaler::SY(80);
        int halfW = panelW / 2;

        COLORREF backCol = (selectedOption == 4) ? ToCOLORREF(Theme::BackButtonSelected) : ToCOLORREF(Palette::GrayDark);
        DrawColText(hdc, GetText("config_back"), panelX + UIScaler::SX(30), botY, halfW - UIScaler::SX(30), backCol, selectedOption == 4 ? GlobalFont::Bold : GlobalFont::Default, DT_LEFT);

        COLORREF startCol = ToCOLORREF(Palette::GreenDark);
        if (selectedOption == 5)
        {
            int gCol = (int)(150 + PixelLayout::SinSmoothedSigned(g_GlobalAnimTime, 20.f) * 105);
            startCol = RGB(0, max(0, min(255, gCol)), 0);
        }
        DrawColText(hdc, GetText("config_start"), panelX, botY, panelW - UIScaler::SX(30), startCol, selectedOption == 5 ? GlobalFont::Title : GlobalFont::Bold, DT_RIGHT);
    }

    if (!validationMsg.empty())
    {
        DrawTextCentered(hdc, validationMsg, panelY + panelH - UIScaler::SY(45), screenWidth, ToCOLORREF(Theme::ValidationError), GlobalFont::Bold);
    }

    std::wstring hintKey = (currentPage == 0) ? GetText("config_page1_hint") : GetText("config_page2_hint");
    DrawTextCentered(hdc, (isEditingName1 || isEditingName2) ? GetText("config_edit_hint") : hintKey, screenHeight - UIScaler::SY(60), screenWidth, ToCOLORREF(Palette::White), GlobalFont::Note);
}
