# 🎮 Dự Án Game Caro & Tic-Tac-Toe (Console C++)

## 🌟 Chức Năng Nổi Bật

* **Chế độ chơi:** Hỗ trợ cả Caro và Tic-Tac-Toe với hai chế độ PvP (Người vs Người) và PvE (Người vs Máy).
* **Điều hướng:** Giao diện điều khiển hoàn toàn bằng bàn phím (W, A, S, D để di chuyển, Enter để chọn, Esc để quay lại/tạm dừng).
* **Hệ thống màn hình:** Bao gồm Menu chính, Giới thiệu (About), Hướng dẫn (Guide), Màn hình chơi (Play Game), Tải game (Load Game) và Cài đặt (Setting).
* **Trải nghiệm cá nhân hóa:** * Nhập tên người chơi và chọn Avatar.
    * Hệ thống tính giờ (Countdown) cho mỗi lượt đi.
    * Giao diện trong trận hiển thị trực quan nước đi, thời gian, điểm số và menu tạm dừng.
* **Chủ đề & Âm thanh:** Hỗ trợ thay đổi chủ đề bàn cờ kèm theo âm thanh tương ứng. Có thể tùy chỉnh BGM (nhạc nền) và SFX (hiệu ứng âm thanh).
* **Đa ngôn ngữ:** Hỗ trợ chuyển đổi mượt mà giữa Tiếng Việt và Tiếng Anh.

## 📂 Cấu Trúc Thư Mục & Mã Nguồn

Dự án được chia thành các hệ thống tách biệt để đảm bảo tính độc lập của dữ liệu và logic.

| Thư mục / Tập tin | Mô tả chi tiết chức năng |
| :--- | :--- |
| **1. ApplicationTypes/** | **Kiểu Dữ Liệu Cốt Lõi (Chỉ chứa struct và enum, không chứa logic)** |
| `GameConfig.h` | Struct lưu cài đặt hiện tại (Âm thanh nền, SFX, Ngôn ngữ: EN/VI). |
| `GameState.h` | Struct trạng thái điều hướng của game (Đang ở Menu, Đang chơi, Tạm dừng...). |
| `PlayState.h` | Struct thông tin ván đấu (Chế độ PvP/PvE, Thời gian đếm ngược, Điểm số, Ma trận bàn cờ). |
| **2. GameLogic/** | **Logic Trò Chơi (Hàm thuần túy: Input -> Output)** |
| `MatchRules.h/cpp` | Kiểm tra quy luật thắng/thua/hòa và cập nhật điểm số. |
| `AI_PvE.h/cpp` | Thuật toán giúp máy tính tính toán nước đi thông minh. |
| `StateUpdater.h/cpp` | Xử lý việc chuyển đổi giữa các trạng thái màn hình. |
| **3. RenderAPI/** | **Giao Diện & Đồ Họa Console** |
| `Renderer.h/cpp` | Các hàm thao tác Console cơ bản (Vẽ Text, Vẽ Hình, Xóa Màn hình, GotoXY). |
| `UIComponents.h/cpp` | Hàm vẽ các khối giao diện dùng chung (Nút bấm, Khung viền, Đồng hồ đếm ngược). |
| **4. ScreenModules/** | **Quản Lý Từng Màn Hình (Chứa hàm Update Input và Render)** |
| `MenuScreen.h/cpp` | Hiển thị và xử lý chọn lựa tại màn hình Menu chính. |
| `AboutScreen.h/cpp` | Hiển thị thông tin nhóm phát triển và phiên bản game. |
| `GuildScreen.h/cpp` | Hiển thị cẩm nang, luật chơi Caro/Tic-Tac-Toe. |
| `PlayScreen.h/cpp` | Xử lý vòng lặp chính của ván đấu (Hiển thị bàn cờ, theo dõi nước đi, tạm dừng). |
| `LoadGameScreen.h/cpp`| Đọc danh sách file save và khởi tạo lại ván đấu. |
| `SettingScreen.h/cpp` | Giao diện điều chỉnh âm lượng và ngôn ngữ. |
| **5. SystemModules/** | **Hệ Thống Thiết Yếu (Tương tác OS, File, Audio)** |
| `AudioSystem.h/cpp` | Quản lý phát/dừng nhạc nền và hiệu ứng âm thanh. |
| `ConfigLoader.h/cpp` | Đọc/Ghi trạng thái cài đặt vào tệp `config.ini`. |
| `Localization.h/cpp` | Hệ thống tải chuỗi văn bản theo ngôn ngữ (Tiếng Việt/Tiếng Anh). |
| `TimeSystem.h/cpp` | Cung cấp các hàm đếm thời gian thực cho tính năng Countdown. |
| **6. Asset/** | **Tài Nguyên Trò Chơi** |
| `lang/` | Chứa các file dữ liệu ngôn ngữ (VD: `vi.json`, `en.json` hoặc file `.txt`). |
| `audio/` | Chứa các tệp âm thanh định dạng `.wav` hoặc `.mp3`. |
| `config.ini` | Tệp lưu trữ thiết lập tự động được tạo ra khi người chơi cấu hình. |