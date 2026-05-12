#ifndef _UI_COMPONENTS_H_
#define _UI_COMPONENTS_H_
#include "Renderer.h"
#include "../ApplicationTypes/PlayState.h"
#include <vector>
#include <string>
#include <map>

/**
 * @file UIComponents.h
 * @brief API vẽ UI (pixel art, banner, sân, bảng) và cơ chế cache liên quan.
 *
 * Tài liệu bằng tiếng Việt. Mọi hàm public mô tả ownership (ai cấp phát/giải phóng)
 * và các side-effect quan trọng (ví dụ cache nội bộ). Không thay đổi logic.
 */

/**
 * @brief Thời gian (giây) tổng cho các animation toàn cục.
 *
 * Giá trị được cập nhật bởi vòng lặp game; các hàm vẽ sử dụng giá trị này
 * để tạo các hiệu ứng nhấp nháy/pulse.
 */
extern float g_GlobalAnimTime;

/**
 * @brief Trạng thái animation cho một người chơi.
 *
 * Trả về/giữ trạng thái hiện tại của avatar để cập nhật frame, flip, và tốc độ.
 * Không thread-safe; được quản lý bởi luồng UI chính của game.
 */
struct PlayerState
{
    int avatarType = 0;                 ///< Loại avatar (id)
    std::string currentAction = "idle"; ///< Tên thư mục hành động (ví dụ: "run")
    int currentFrame = 0;               ///< Frame hiện tại của hành động
    ULONGLONG lastFrameTime = 0;        ///< Thời điểm frame trước (GetTickCount64)
    int animationSpeed = 100;           ///< Khoảng thời gian (ms) giữa 2 frame
    bool flipH = false;                 ///< Lật ngang ảnh
};

/**
 * @brief Ma trận pixel biểu diễn một sprite nhỏ.
 *
 * @note Trường `data` là matrix [height][width] với các mã màu (int).
 *       `isLoaded` true khi đã đọc thành công từ file. Chi tiết về định dạng
 *       file: dòng đầu chứa "width height", các dòng sau là các giá trị số.
 */
struct PixelModel
{
    int width = 0;
    int height = 0;
    std::vector<std::vector<int>> data;
    bool isLoaded = false;
};

/**
 * @brief Load dữ liệu PixelModel từ file text và cache vào bộ nhớ module.
 *
 * @param filePath Đường dẫn tới file model (ví dụ: "Asset/models/avt_00/avt.txt").
 * @return PixelModel Trả về struct theo giá trị; `isLoaded` = true khi thành công.
 * @note Hàm lưu một bản copy vào cache nội bộ `g_RawModelCache` để tái sử dụng.
 * @warning Không thread-safe.
 */
PixelModel LoadPixelModel(const std::string &filePath);

/**
 * @brief Vẽ một PixelModel lên GDI+ tại tâm (centerX, centerY) với palette cung cấp.
 *
 * @param g Đối tượng Gdiplus::Graphics để vẽ (caller quản lý).
 * @param model Dữ liệu pixel (không sở hữu bởi hàm).
 * @param centerX Tọa độ X tâm vẽ (px).
 * @param centerY Tọa độ Y tâm vẽ (px).
 * @param totalSize Kích thước tổng (px) mà sprite nên chiếm.
 * @param palette Bản đồ mã màu -> Gdiplus::Color dùng để tô.
 * @param manualPaletteHash Nếu khác 0 thì dùng để tạo key cache cố định.
 * @note Hàm sử dụng cache nội bộ `g_ModelCache` chứa `Gdiplus::Bitmap*` được cấp phát
 *       bởi module. Caller không được giải phóng các bitmap này.
 */
void DrawPixelModel(Gdiplus::Graphics &g, const PixelModel &model, int centerX, int centerY, int totalSize, const std::map<int, Gdiplus::Color> &palette, size_t manualPaletteHash = 0);

/**
 * @brief Vẽ văn bản căn giữa nằm trong vùng [leftX, rightX].
 *
 * @param hdc HDC dùng cho GDI text rendering.
 * @param text Chuỗi Unicode cần vẽ.
 * @param posY Tọa độ Y của dòng văn bản (px).
 * @param rightX Biên phải vùng vẽ (px).
 * @param color Màu chữ (COLORREF).
 * @param hFont Font tùy chọn; nếu null dùng `GlobalFont::Default`.
 * @param leftX Biên trái vùng vẽ (px).
 */
void DrawTextCentered(HDC hdc, const std::wstring &text, int posY, int rightX, COLORREF color, HFONT hFont = nullptr, int leftX = 0);

/**
 * @brief Vẽ text có viền mỏng bên ngoài để dễ đọc trên nền phức tạp.
 *
 * Kỹ thuật: Vẽ text 4 lần với offset ±1px bằng màu viền, sau đó vẽ text chính lên trên.
 *
 * @param hdc Device context.
 * @param text Chuỗi Unicode cần vẽ.
 * @param rect Vùng vẽ (RECT).
 * @param textColor Màu chữ chính.
 * @param outlineColor Màu viền.
 * @param hFont Font sử dụng; nếu null dùng GlobalFont::Default.
 * @param format Cờ DrawTextW (DT_CENTER | DT_SINGLELINE, v.v.).
 */
void DrawTextOutlined(HDC hdc, const std::wstring &text, RECT rect, COLORREF textColor, COLORREF outlineColor, HFONT hFont = nullptr, UINT format = DT_CENTER | DT_SINGLELINE | DT_VCENTER);

/**
 * @brief Vẽ text căn giữa có viền mỏng.
 * @param outlineColor Màu viền (mặc định đen).
 */
void DrawTextCenteredOutlined(HDC hdc, const std::wstring &text, int posY, int rightX, COLORREF textColor, COLORREF outlineColor, HFONT hFont = nullptr, int leftX = 0);

/**
 * @brief Vẽ nền sân vận động (pitch) bằng thuật toán procedural và cache tĩnh.
 *
 * @param g Đối tượng GDI+ Graphics.
 * @param screenWidth Chiều rộng màn hình (px).
 * @param screenHeight Chiều cao màn hình (px).
 * @param shouldShowFlashes Nếu true bật hiệu ứng flash camera (dùng ở menu).
 * @param shouldAnimate Nếu false chỉ vẽ nền tĩnh (tiết kiệm CPU).
 * @note Hàm duy trì cache tĩnh `pitchCache` để tránh vẽ lại toàn bộ khi không cần thiết.
 */
void DrawProceduralStadium(Gdiplus::Graphics &g, int screenWidth, int screenHeight, bool shouldShowFlashes = false, bool shouldAnimate = true);

/**
 * @brief Vẽ avatar pixel-art (8x8) tại vị trí center.
 *
 * @param g Đối tượng GDI+ Graphics.
 * @param centerX Tọa độ X tâm vẽ (px).
 * @param centerY Tọa độ Y tâm vẽ (px).
 * @param size Kích thước (px) mong muốn của avatar.
 * @param avatarType Loại avatar (id). Nếu không hợp lệ, dùng 0.
 * @note Hàm sử dụng cache `g_AvatarCache` chứa `Gdiplus::Bitmap*` do module quản lý.
 */
void DrawPixelAvatar(Gdiplus::Graphics &g, int centerX, int centerY, int size, int avatarType);

/**
 * @brief Lấy màu GDI+ tương ứng từ palette nội bộ theo type/code.
 *
 * @param type Loại (avatar type hoặc ngữ cảnh màu).
 * @param code Mã màu trong ma trận pixel.
 * @return Gdiplus::Color Màu GDI+ đã chuyển.
 */
Gdiplus::Color GetPaletteColor(int type, int code);

/**
 * @brief Vẽ hoạt họa theo thư mục hành động của avatar.
 *
 * @param g Đối tượng GDI+ Graphics.
 * @param centerX Tọa độ X tâm vẽ.
 * @param centerY Tọa độ Y tâm vẽ.
 * @param size Kích thước vẽ (px).
 * @param state Tham chiếu đến PlayerState, hàm có thể cập nhật frame/state.
 * @note Hàm cập nhật `state.currentFrame` dựa trên thời gian và sử dụng cache `g_ActionCache`.
 */
void DrawPixelAction(Gdiplus::Graphics &g, int centerX, int centerY, int size, PlayerState &state);

/**
 * @brief Vẽ các biểu tượng pixel chuyên dụng (football, trophy, clock).
 */
void DrawPixelFootball(Gdiplus::Graphics &g, int centerX, int centerY, int size);
void DrawPixelTrophy(Gdiplus::Graphics &g, int centerX, int centerY, int size);
void DrawPixelClock(Gdiplus::Graphics &g, int centerX, int centerY, int size, Gdiplus::Color color);

/**
 * @brief Vẽ banner tiêu đề pixel-art với text GDI và icon tùy chọn.
 *
 * @param g Đối tượng GDI+ Graphics.
 * @param hdc HDC dùng cho DrawTextW.
 * @param text Chuỗi tiêu đề (Unicode).
 * @param centerX Tọa độ X tâm banner.
 * @param centerY Tọa độ Y tâm banner.
 * @param panelWidth Chiều rộng vùng banner.
 * @param textColor Màu chữ (COLORREF).
 * @param glowColor Màu glow/nhấn (COLORREF).
 */
void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text, int centerX, int centerY, int panelWidth, COLORREF textColor, COLORREF glowColor);
void DrawPixelBanner(Gdiplus::Graphics &g, HDC hdc, const std::wstring &text, int centerX, int centerY, int panelWidth, COLORREF textColor, COLORREF glowColor, const std::string &iconModelPath);

/**
 * @brief Vẽ lại toàn bộ bảng (grid) và quân cờ dựa trên trạng thái `PlayState`.
 *
 * @param g Đối tượng GDI+ Graphics (để vẽ highlight, glow).
 * @param hdc HDC dùng cho vẽ text/quân cờ bằng GDI.
 * @param state Con trỏ tới `PlayState` nguồn dữ liệu (không null).
 * @param cellSize Kích thước ô (px).
 * @param offsetX Tọa độ X bắt đầu bảng (px).
 * @param offsetY Tọa độ Y bắt đầu bảng (px).
 * @note Hàm sử dụng cả GDI và GDI+; caller phải đảm bảo `g` và `hdc` hợp lệ.
 */
void DrawGameBoard(Gdiplus::Graphics &g, HDC hdc, const PlayState *state, int cellSize, int offsetX, int offsetY, const RECT *clip = nullptr);

/**
 * @brief Thiết lập màu chữ cho HDC và chế độ nền trong suốt.
 *
 * @param hdc HDC để đặt màu.
 * @param colour Màu chữ (COLORREF).
 */
void SetTextColour(HDC hdc, COLORREF colour);

/**
 * @brief Lấy hoặc tạo `Gdiplus::SolidBrush*` đã được cache theo màu.
 *
 * @param color Màu GDI+ cần brush.
 * @return Gdiplus::SolidBrush* Con trỏ brush được quản lý bởi cache module (caller không free).
 * @note Cache nội bộ `g_BrushCache` chịu trách nhiệm giải phóng khi `ClearUICaches` được gọi.
 */
Gdiplus::SolidBrush *GetCachedBrush(const Gdiplus::Color &color);

/**
 * @brief Giải phóng toàn bộ bộ nhớ đệm nội bộ (bitmaps, brushes, raw models).
 *
 * @note Gọi hàm này khi shutdown để tránh rò rỉ bộ nhớ. Sau khi gọi,
 *       các cache sẽ rỗng và các con trỏ bitmap/brush đã bị delete.
 */
void ClearUICaches();

#endif // _UI_COMPONENTS_H_