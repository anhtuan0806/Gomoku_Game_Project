## Các Chức Năng Chính Của Trò Chơi

- **Chế độ chơi đa dạng**:
  - `PvP` (Người đấu với Người).
  - `PvE` (Người đấu với Máy - Bot AI có thuật toán đánh giá thế cờ phản công/phòng thủ).
- **Hệ thống Quản lý Ván Đấu (Save/Load)**: Cho phép tạm dừng, lưu trữ tiến độ trận đấu hiện tại ra file (Save Game) và khôi phục (Load Game) để tiếp tục chơi ở lần mở máy sau.
- **Trải nghiệm cá nhân hóa (Player)**: 
  - Đặt tên hiển thị và chọn biểu tượng Avatar theo sở thích.
  - Hệ thống đếm ngược thời gian (Countdown Time) áp lực cho mỗi lượt đi riêng biệt.
  - Cập nhật số liệu thống kê: Tổng số ván đã chiến thắng, Tổng số bước đi.
- **Giao diện Console Đa Sắc Tái Sinh (GUI)**: 
  - Chống nháy màn hình, thiết kế bảng UI, nút bấm trực tiếp bằng Console API và mã màu RGB.
  - Trải nghiệm mượt mà, phân cấp các màn hình (Intro, Menu, Play, Setting).
- **Hệ Thống Thiết Lập & Âm thanh**: 
  - Hỗ trợ đổi chủ đề bàn cờ (Theme).
  - Tự động phát nhạc nền (BGM) và các hiệu ứng đếm ngược, tiếng thả cờ (SFX).
- **Tính Năng Đa ngôn ngữ**: Hệ thống chuyển đổi text mượt mà giữa Tiếng Việt và Tiếng Anh.
- **Menu Tiện Ích Phụ Cơ Bản**: Màn hình xem trợ giúp chỉ dẫn luật chơi và Danh sách Credit phát triển.

## Cấu Trúc Thư Mục & Mã Nguồn

Dự án được chia thành các hệ thống tách biệt để đảm bảo tính độc lập của dữ liệu và logic. Tất cả source code C++ đều nằm trong thư mục `src`.

| Thư mục / Tập tin | Mô tả chi tiết chức năng |
| :--- | :--- |
| **1. ApplicationTypes/** | **Kiểu Dữ Liệu Cốt Lõi (Chỉ chứa struct và enum, không chứa logic)** |
| `GameConfig.h` | Struct lưu cài đặt hiện tại (Âm thanh nền, SFX, Ngôn ngữ: EN/VI). |
| `GameConstants.h` | Chứa các hằng số cấu hình dùng chung toàn dự án (Kích thước bàn cờ, Đường dẫn). |
| `GameState.h` | Struct trạng thái điều hướng của game (Đang ở Menu, Đang chơi, Tạm dừng...). |
| `PlayState.h` | Struct thông tin ván đấu (Chế độ PvP/PvE, Thời gian đếm ngược, Điểm số, Ma trận bàn cờ). |
| **2. GameLogic/** | **Logic Trò Chơi (Hàm xử lý quy luật, tính toán đầu vào -> Output)** |
| `GameRules.h/cpp` | Kiểm tra quy luật thắng/thua/hòa, cập nhật điểm số và kiểm tra quân vi phạm. |
| `BotAI.h/cpp` | Thuật toán đánh giá thế cờ giúp máy tính tính toán nước đi thông minh (Chế độ PvE). |
| `GameEngine.h/cpp` | Bộ khung xử lý chuyển lượt, ra quyết định kết thúc ván đấu dựa trên State. |
| `PlayerEngineer.h/cpp`| Chịu trách nhiệm quản lý điểm số, số bước đi và trạng thái nội bộ của Player. |
| `SaveManager.h/cpp` | Xử lý việc chuyển đổi PlayState ghi/đọc ra file txt để tải lại tiến trình đang đánh. |
| **3. RenderAPI/** | **Giao Diện & Đồ Họa Console** |
| `Colours.h` | Hỗ trợ bộ mã màu RGB để vẽ nét vẽ màu sắc mượt mà trên nền Console. |
| `Renderer.h/cpp` | Thao tác lệnh vẽ trực tiếp với OS: thay đổi kích thước, di chuyển con trỏ, xóa màn hình. |
| `UIComponents.h/cpp` | Khởi tạo các Component GUI tĩnh tái sử dụng được (Button, Checkbox, TextBox...). |
| **4. ScreenModules/** | **Các Trang Màn Hình Điều Hướng Phân Rã Thành File Riêng** |
| `PlayScreen.h/cpp` | Xử lý Logic hiển thị và bắt luồng xử lý riêng cho màn hình Trận đấu chính. |
| `MenuScreen.h/cpp` | Màn hình đầu tiên (Main Menu) để người chơi lựa chọn danh mục. |
| `SettingScreen.h/cpp` | Màn hình cài đặt các tính năng (Âm thanh, chủ đề, ngôn ngữ...). |
| `LoadGameScreen.h/cpp`| Màn hình hiển thị danh sách file Save và Load game. |
| `GuildScreen.h/cpp` | (Guide) Màn hình Hướng dẫn luật chơi cho người mới. |
| `AboutScreen.h/cpp` | Màn hình Credit và Giới thiệu thông tin Nhóm phát triển. |
| **5. SystemModules/** | **Các Hệ Thống Quản Lý Tiện Ích Trợ Năng Ngoại Vi** |
| `AudioSystem.h/cpp` | Chịu trách nhiệm trực tiếp mở/dừng nhạc Background và hiệu ứng SFX. |
| `TimeSystem.h/cpp` | Hệ thống quản lý bộ trích xuất thông số millisecond & Đếm ngược thời gian. |
| `Localization.h/cpp` | Load Json/Text file dựa trên key ngôn ngữ tương ứng để biên dịch in ra giao diện. |
| `ConfigLoader.h/cpp` | Logic load dữ liệu thiết lập người dùng đã lưu lại từ Setting. |
| `SaveLoadSystem.h/cpp`| Xử lý danh sách file Save, liệt kê vào hệ thống. |
| **6. Asset/** | **Thư Mục Lưu Trữ Tài Nguyên (Assets) & Cấu hình** |
| `config.ini` | File lưu trước các cài đặt của người dùng (.ini format) để giữ thiết lập qua nhiều lần mở máy. |
| `images/` | Nơi lưu trữ hình ảnh vật lý (như danh sách Avatar) dùng cho Player. |
| `lang/` | Chứa các file Text/Json biên dịch riêng lẻ của Tiếng Việt (VI) và Tiếng Anh (EN). |
| **7. main.cpp** | **File Gốc (Entry point)**: Điểm chạy đầu tiên, chứa hàm `main()` khởi động vòng lặp trò chơi. |
