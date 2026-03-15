#ifndef UI_COMPONENTS_H
#define UI_COMPONENTS_H
#include "Renderer.h"

/**
 * @brief Vẽ lưới bàn cờ dựa trên kích thước và cài đặt đã cho.
 *
 * @para hdc: Device Context (thường là hdcMem của DoubleBuffer).
 * @para size: Số lượng ô vuông theo chiều ngang/dọc của bàn cờ (ví dụ: 3 cho Tic-Tac-Toe, 12 cho Caro).
 * @para cellSize: Kích thước của một ô vuông (pixel).
 * @para offsetX, offsetY: Tọa độ lề của bàn cờ để căn chỉnh
 * 
 * @logic
 * - Tạo một bút vẽ (HPEN) với màu sắc và độ dày phù hợp để vẽ lưới.
 * - Sử dụng vòng lặp để vẽ các đường ngang và dọc của lưới dựa trên kích thước và cellSize.
 * - Sau khi vẽ xong, phục hồi bút vẽ cũ và giải phóng tài nguyên của bút mới để tránh rò rỉ bộ nhớ.
 * 
 * @return void.
 */
void DrawGrid(HDC hdc, int size, int cellSize, int offsetX, int offsetY);

/**
 * @brief Vẽ hiệu ứng highlight (viền đỏ) xung quanh ô hiện tại được chọn.
 *
 * @para hdc: Device Context để vẽ lên.
 * @para row, col: Tọa độ hàng và cột của ô cần được highlight.
 * @para cellSize: Kích thước của một ô vuông (pixel).
 * @para offX, offY: Tọa độ lề của bàn cờ để căn chỉnh vị trí vẽ.
 *
 * @logic
 * - Tính toán vị trí và kích thước của hình chữ nhật dựa trên row, col, cellSize, offX và offY.
 * - Tạo một brush màu đỏ để vẽ viền xung quanh ô cờ được chọn bằng hàm FrameRect.
 * - Sau khi vẽ xong, giải phóng tài nguyên của brush để tránh rò rỉ bộ nhớ.
 *
 * @return void.
 */
void DrawHighlight(HDC hdc, int row, int col, int cellSize, int offX, int offY);

/**
 * @brief Cài đặt màu sắc và độ dày cho bút vẽ (HPEN) trong Device Context.
 *
 * @para hdc: Device Context cần thay đổi bút vẽ.
 * @para colour: Mã màu RGB để cài đặt cho bút vẽ.
 * @para thickness: Độ dày của bút vẽ (pixel).
 *
 * @logic
 * - Tạo một bút mới với màu sắc và độ dày được chỉ định bằng hàm CreatePen.
 * - Sử dụng SelectObject để chọn bút mới vào Device Context, đồng thời trả về handle của bút cũ để có thể phục hồi sau này.
 *
 * @return HPEN Trả về handle của bút cũ đã được lưu lại trước khi thay đổi, để có thể khôi phục sau này.
 */
HPEN SetDrawColour(HDC hdc, COLORREF colour, int thickness);

/**
 * @brief Phục hồi bút vẽ cũ vào Device Context và giải phóng tài nguyên của bút mới đã tạo.
 *
 * @para hdc: Device Context cần phục hồi bút vẽ.
 * @para hOldPen: Handle của bút cũ đã được lưu lại trước khi thay đổi.
 *
 * @logic
 * - Sử dụng SelectObject để chọn lại bút cũ vào Device Context, đồng thời lấy lại handle của bút mới đã tạo.
 * - Gọi DeleteObject để giải phóng tài nguyên của bút mới, đảm bảo không có rò rỉ bộ nhớ liên quan đến GDI.
 *
 * @return void.
 */
void ResetDrawColour(HDC hdc, HPEN hOldPen);

/**
 * @brief Cài đặt màu sắc cho văn bản được vẽ trên Device Context.
 *
 * @para hdc: Device Context cần thay đổi màu sắc văn bản.
 * @para colour: Mã màu RGB để cài đặt cho văn bản.
 *
 * @logic
 * - Sử dụng hàm SetTextColor để cài đặt màu sắc cho văn bản trong Device Context.
 * - Thiết lập chế độ nền của văn bản thành TRANSPARENT để đảm bảo rằng chữ không có nền màu bao quanh, giúp hiển thị mượt mà hơn trên các ô cờ có hình ảnh.
 *
 * @return void.
 */
void SetTextColour(HDC hdc, COLORREF colour);

/**
 * @brief Vẽ ảnh PNG lên một Device Context của GDI.
 *
 * @para hdc: Device Context để vẽ lên (thường là hdcMem của DoubleBuffer).
 * @para sprite: Cấu trúc Sprite chứa con trỏ Gdiplus::Image và kích thước của ảnh.
 * @para x, y: Tọa độ pixel góc trên bên trái của ô cờ.
 * @para targetSize: Kích thước hiển thị mong muốn (thường bằng cellSize).
 *
 * @logic
 * - Kiểm tra nếu sprite.image không phải là nullptr, nếu có thì tiếp tục vẽ.
 * - Tạo một đối tượng Graphics từ Device Context để sử dụng các phương thức vẽ nâng cao của GDI+.
 * - Thiết lập chế độ nội suy (InterpolationModeHighQuality) để đảm bảo chất lượng hình ảnh khi co giãn.
 * - Gọi phương thức DrawImage của đối tượng Graphics để vẽ ảnh PNG lên tọa độ (x, y) với kích thước targetSize.
 *
 * @return void.
 */
void DrawSprite(HDC hdc, const Sprite& sprite, int x, int y, int targetSize);

/**
 * @brief Vẽ ảnh PNG lên một đối tượng Graphics của GDI+.
 *
 * @para g: Đối tượng Graphics đã được tạo từ Device Context.
 * @para sprite: Cấu trúc Sprite chứa con trỏ Gdiplus::Image và kích thước của ảnh.
 * @para x, y: Tọa độ pixel góc trên bên trái của ô cờ.
 * @para targetSize: Kích thước hiển thị mong muốn (thường bằng cellSize).
 *
 * @logic
 * - Kiểm tra nếu sprite.image không phải là nullptr, nếu có thì tiếp tục vẽ.
 * - Thiết lập chế độ nội suy (InterpolationModeHighQuality) để đảm bảo chất lượng hình ảnh khi co giãn.
 * - Gọi phương thức DrawImage của đối tượng Graphics để vẽ ảnh PNG lên tọa độ (x, y) với kích thước targetSize.
 *
 * @return void.
 */
void DrawSprite(Gdiplus::Graphics& g, const Sprite& sprite, int x, int y, int targetSize);

/**
 * @brief Giải phóng tài nguyên của một Sprite, bao gồm cả đối tượng Gdiplus::Image và đặt lại các giá trị trong cấu trúc Sprite.
 *
 * @para sprite: Cấu trúc Sprite cần được giải phóng tài nguyên.
 *
 * @logic
 * - Kiểm tra nếu sprite.image không phải là nullptr, nếu có thì gọi delete để giải phóng bộ nhớ của đối tượng Gdiplus::Image.
 * - Đặt sprite.image về nullptr để tránh việc truy cập sau khi đã giải phóng.
 * - Đặt lại sprite.width và sprite.height về 0 để phản ánh rằng Sprite hiện không chứa ảnh nào.
 *
 * @return void.
 */
void FreeSprite(Sprite& sprite);

/**
 * @brief Thay đổi kích thước của một Sprite bằng cách tạo một Bitmap mới với kích thước đích và vẽ ảnh gốc lên đó.
 *
 * @para sprite: Cấu trúc Sprite cần được thay đổi kích thước.
 * @para targetWidth: Chiều rộng mong muốn của Sprite sau khi thay đổi kích thước.
 * @para targetHeight: Chiều cao mong muốn của Sprite sau khi thay đổi kích thước.
 *
 * @logic
 * - Kiểm tra nếu sprite.image không phải là nullptr, nếu có thì tiếp tục thực hiện thay đổi kích thước.
 * - Tạo một đối tượng Bitmap mới với kích thước targetWidth và targetHeight, sử dụng định dạng PixelFormat32bppARGB để hỗ trợ kênh alpha.
 * - Tạo một đối tượng Graphics từ Bitmap mới để vẽ ảnh gốc lên đó.
 * - Thiết lập chế độ nội suy (InterpolationModeHighQualityBicubic) để đảm bảo chất lượng hình ảnh khi co giãn.
 * - Gọi phương thức DrawImage của đối tượng Graphics để vẽ ảnh gốc lên Bitmap mới với kích thước targetWidth và targetHeight.
 * - Giải phóng bộ nhớ của ảnh gốc bằng cách gọi delete trên sprite.image, sau đó cập nhật sprite.image để trỏ đến Bitmap mới và cập nhật sprite.width và sprite.height với kích thước đích.
 *
 * @return void.
 */
void ScaleSprite(Sprite& sprite, int targetWidth, int targetHeight);

/**
 * @brief Vẽ một chuỗi văn bản được căn giữa theo chiều ngang trên một Device Context.
 *
 * @para hdc: Device Context để vẽ lên.
 * @para text: Chuỗi văn bản cần vẽ (dạng std::wstring để hỗ trợ Unicode).
 * @para y: Tọa độ pixel theo chiều dọc nơi văn bản sẽ được vẽ.
 * @para screenWidth: Chiều rộng của vùng vẽ để tính toán vị trí căn giữa.
 * @para color: Mã màu RGB để cài đặt cho văn bản.
 * @para hFont: Handle của Font để sử dụng khi vẽ văn bản. Nếu là nullptr, sẽ sử dụng Font mặc định toàn cục.
 *
 * @logic
 * - Kiểm tra nếu hFont là nullptr, nếu có thì sử dụng GlobalFont::Default làm font mặc định.
 * - Sử dụng SelectObject để chọn font vào Device Context và lưu lại handle của font cũ để phục hồi sau này.
 * - Cài đặt màu sắc cho văn bản bằng cách gọi SetTextColor và thiết lập chế độ nền thành TRANSPARENT.
 * - Xác định vùng vẽ chữ bằng cách tạo một RECT với chiều rộng bằng screenWidth và chiều cao đủ lớn để chứa văn bản (ví dụ: y + 100).
 * - Gọi hàm DrawTextW với các tham số phù hợp để vẽ văn bản được căn giữa trong vùng đã xác định.
 * - Sau khi vẽ xong, phục hồi font cũ vào Device Context và giải phóng tài nguyên của font mới nếu cần thiết.
 *
 * @return void.
 */
void DrawTextCentered(HDC hdc, const std::wstring& text, int y, int screenWidth, COLORREF color, HFONT hFont = nullptr);

#endif // UI_COMPONENTS_H