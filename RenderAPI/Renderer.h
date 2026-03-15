#ifndef RENDERER_H
#define RENDERER_H
#include <windows.h>
#include <gdiplus.h>
#include <string>
#pragma comment(lib, "gdiplus.lib")

 /**
  * @brief  Sprite là cấu trúc đại diện cho một hình ảnh trong trò chơi
  *
  * @param  image: Con trỏ đến đối tượng Gdiplus::Image chứa hình ảnh của sprite
  * @param  width: Chiều rộng của sprite
  * @param  height: Chiều cao của sprite
  */
struct Sprite {
    Gdiplus::Image* image;
    int width;
    int height;
};

/**
 * @brief  DoubleBuffer là cấu trúc đại diện cho bộ đệm kép để giảm thiểu hiện tượng nhấp nháy khi vẽ đồ họa
 *
 * @param  hdcMem: Handle đến device context của bộ đệm kép
 * @param  hBitmap: Handle đến bitmap được sử dụng làm bộ đệm
 * @param  hOldBitmap: Handle đến bitmap cũ được lưu trữ để khôi phục sau khi vẽ xong
 * @param  width: Chiều rộng của bộ đệm
 * @param  height: Chiều cao của bộ đệm
 */
struct DoubleBuffer {
    HDC hdcMem;
    HBITMAP hBitmap;
    HBITMAP hOldBitmap;
    int width, height;
};

/**
 * @brief Quản lý hệ thống Font toàn cục 
 * Được nạp 1 lần vào RAM khi khởi động và tái sử dụng cho toàn bộ game.
 */
namespace GlobalFont {
    extern HFONT Default; // Font tiêu chuẩn (Cỡ 30, Thường)
    extern HFONT Bold;    // Font in đậm (Cỡ 35, Menu Highlight)
    extern HFONT Title;   // Font tiêu đề (Cỡ 50, Đậm)

    void Initialize();
    void Cleanup();
}

/**
 * @brief Khởi tạo GDI+ để sử dụng các chức năng đồ họa nâng cao trong ứng dụng.
 *
 * @param token Tham chiếu đến biến ULONG_PTR sẽ được sử dụng để lưu trữ định danh quản lý GDI+ sau khi khởi tạo thành công.
 * Hàm này sẽ gọi GdiplusStartup để thiết lập môi trường làm việc của GDI+ và lưu trữ token trả về để sử dụng cho việc giải phóng tài nguyên sau này.
 *
 * @logic
 * - Tạo một đối tượng GdiplusStartupInput để cấu hình các tham số khởi tạo.
 * - Gọi hàm GdiplusStartup với địa chỉ của token và đối tượng khởi tạo. Nếu trả về Status::Ok, quá trình khởi tạo thành công.
 *
 * @return bool Trả về true nếu khởi tạo thành công, ngược lại trả về false nếu có lỗi xảy ra trong quá trình khởi tạo.
 */
bool InitGraphics(ULONG_PTR& token);

/**
 * @brief Kết thúc phiên làm việc của GDI+ và giải phóng tài nguyên liên quan.
 *
 * @param token Biến ULONG_PTR chứa định danh quản lý GDI+ được trả về từ hàm InitGraphics. Hàm này sẽ gọi GdiplusShutdown với token để giải phóng tài nguyên đã được cấp phát cho GDI+.
 *
 * @logic
 * - Gọi hàm GdiplusShutdown với token để kết thúc phiên làm việc của GDI+ và giải phóng tất cả tài nguyên liên quan đến GDI+ đã được cấp phát trong quá trình khởi tạo.
 *
 * @return void.
 */
void ShutdownGraphics(ULONG_PTR token);

/**
 * @brief Tải một hình ảnh PNG từ đường dẫn tệp và trả về một đối tượng Sprite chứa hình ảnh đã nạp.
 *
 * @param path Đường dẫn đến tệp PNG cần tải. Đường dẫn này phải là chuỗi Unicode (wchar_t*) để hỗ trợ các ký tự đặc biệt trong tên tệp hoặc đường dẫn.
 *
 * @logic
 * - Sử dụng phương thức Image::FromFile của GDI+ để tạo một đối tượng Image từ đường dẫn được cung cấp.
 * - Kiểm tra xem việc nạp ảnh có thành công hay không bằng cách kiểm tra trạng thái trả về của đối tượng Image. Nếu thành công, gán con trỏ Image vào trường image của Sprite và lưu trữ chiều rộng và chiều cao của ảnh vào các trường width và height của Sprite.
 * - Trả về đối tượng Sprite đã được khởi tạo với hình ảnh đã nạp. Nếu có lỗi xảy ra trong quá trình nạp ảnh, trường image sẽ được đặt thành nullptr và width, height sẽ là 0.
 *
 * @return Sprite Đối tượng Sprite chứa hình ảnh đã nạp và kích thước của nó. Nếu nạp thất bại, image sẽ là nullptr và width, height sẽ là 0.
 */
Sprite LoadPNG(const wchar_t* path);

/**
 * @brief Thiết lập bộ đệm ảo (Back Buffer) để thực hiện kỹ thuật Double Buffering.
 *
 * @para hwnd: Handle của cửa sổ ứng dụng.
 * @para hdc: Device Context của màn hình chính (thường lấy từ BeginPaint).
 * @para buffer: Cấu trúc DoubleBuffer để lưu trữ các thành phần của bộ đệm.
 *
 * @logic
 * 1. Lấy kích thước hiện tại của vùng vẽ (client area) bằng GetClientRect.
 * 2. Tạo một DC ảo tương thích với màn hình chính bằng CreateCompatibleDC.
 * 3. Tạo một Bitmap trắng tương thích với màn hình chính bằng CreateCompatibleBitmap.
 * 4. Gắn (SelectObject) Bitmap vào DC ảo để tạo thành một "bản vẽ ngầm" trong RAM.
 *
 * @return void.
 */
void CreateBuffer(HWND hwnd, HDC hdc, DoubleBuffer& buffer);

/**
 * @brief Giải phóng tài nguyên của DoubleBuffer sau khi vẽ xong.
 *
 * @para buffer: Cấu trúc DoubleBuffer chứa các thành phần cần giải phóng.
 *
 * @logic
 * 1. Gọi DeleteObject để xóa bitmap được sử dụng làm bộ đệm.
 * 2. Gọi DeleteDC để xóa device context ảo (hdcMem).
 * 3. Đặt lại các handle và kích thước trong buffer về giá trị mặc định (nullptr, 0).
 *
 * @return void.
 */
void DeleteBuffer(DoubleBuffer& buffer);
#endif // RENDERER_H