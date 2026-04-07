# Caro Champions League

Caro Champions League là một trò chơi cờ caro (Gomoku) và Tic-Tac-Toe chạy trên Windows, được xây dựng hoàn toàn bằng C++ thuần với Win32 API và GDI+. Giao diện mang chủ đề bóng đá với đồ họa pixel art được vẽ theo quy trình (procedural), hệ thống animation mượt và âm thanh tích hợp.

## Mục lục

- [Tính năng](#tính-năng)
- [Yêu cầu hệ thống](#yêu-cầu-hệ-thống)
- [Hướng dẫn build](#hướng-dẫn-build)
- [Cấu trúc dự án](#cấu-trúc-dự-án)
- [Kiến trúc chương trình](#kiến-trúc-chương-trình)
- [Luồng hoạt động](#luồng-hoạt-động)
- [Điều khiển](#điều-khiển)
- [Tệp cấu hình và lưu game](#tệp-cấu-hình-và-lưu-game)
- [Hệ thống Asset](#hệ-thống-asset)

---

## Tính năng

### Chế độ chơi
- **Caro (Gomoku):** Bàn cờ 15x15, thắng khi tạo được chuỗi 5 quân liên tiếp theo hàng ngang, dọc hoặc chéo.
- **Tic-Tac-Toe:** Bàn cờ 3x3, thắng khi tạo được chuỗi 3 quân liên tiếp.
- **PvP (Player vs Player):** Hai người chơi luân phiên trên cùng một máy.
- **PvE (Player vs Bot):** Người chơi đối đầu với bot AI với 3 cấp độ khó.

### Thể thức thi đấu
- **Bo1 / Bo3 / Bo5:** Thể thức phân định thắng bằng số bàn thắng (chạm 1, 2 hoặc 3 bàn).
- Tự động chuyển sang ván tiếp theo khi chưa đủ số bàn thắng, giữ nguyên tổng điểm.

### Trí tuệ nhân tạo (Bot AI)
- **Cấp 1 - Phân Hạng Đồng (Easy):** Bot đặt quân ngẫu nhiên vào ô trống.
- **Cấp 2 - Phân Hạng Vàng (Medium):** Phân tích điểm số từng nước đi dựa trên số quân liên tiếp và số đầu mở, ưu tiên phòng thủ gấp đôi tấn công.
- **Cấp 3 - Thách Đấu (Hard):** Thuật toán Alpha-Beta Pruning với độ sâu 4, kết hợp Move Ordering để cắt tỉa nhánh kém hiệu quả và tăng tốc tìm kiếm.

Bot luôn đánh vai PLAYER 2, chạy trên luồng (thread) riêng (`std::async`) để không làm treo giao diện trong khi tính toán.

### Hệ thống đồng hồ
- **Chế độ PvP:** Đếm ngược thời gian từng lượt, khi hết giờ tự động chuyển lượt cho đối thủ.
- **Chế độ PvE:** Đo thời gian tổng đã chơi (không giới hạn lượt).
- Mỗi người chơi có `maxTurnTime` riêng, được khởi tạo từ `countdownTime` khi bắt đầu trận.

### Undo / Redo
- Chỉ khả dụng trong chế độ PvE.
- **Undo (Q):** Rút lại 2 nước đi liên tiếp (nước của người chơi và nước phản hồi của bot).
- **Redo (E):** Hoàn tác thao tác Undo, tái hiện lại bước đi, đồng thời kiểm tra lại điều kiện thắng.
- Redo stack bị xóa ngay khi người chơi thực hiện một nước đi mới.

### Lưu và tải game
- Có tối đa **5 slot** lưu game trong thư mục `Asset/save/`.
- Thêm **1 slot autosave** (`save_auto.bin`) được lưu tự động.
- Định dạng lưu là binary có **magic number** (`0xCA05A1E2`) và **version** (`2`) để kiểm tra tính hợp lệ.
- Toàn bộ trạng thái được serialize theo từng trường (field-by-field), bao gồm ma trận bàn cờ, thông tin người chơi, lịch sử bước đi, redo stack, và thời gian.
- Khi load, chương trình kiểm tra magic và version; nếu không khớp sẽ từ chối tải và báo lỗi thay vì crash.

### Giao diện và đồ họa
- **Nền sân vận động procedural:** Cỏ sọc hai màu xen kẽ, vạch kẻ sân (đường giữa sân, vòng tròn trung tâm, khu cấm địa), hiệu ứng camera flash trên khán đài.
- **Pixel Art thủ tục:** Các đối tượng như logo "CARO", cúp vô địch, trái bóng, và avatar cầu thủ đều được vẽ từ ma trận số nguyên được load từ file `.txt` trong thư mục `Asset/models/`.
- **Double Buffering:** Toàn bộ frame được vẽ vào bộ đệm ảo trong RAM trước, rồi mới sao chép ra màn hình bằng `BitBlt`, loại bỏ hoàn toàn hiện tượng nhấp nháy.
- **Font VT323:** Font pixel art được load riêng vào bộ nhớ ứng dụng (`FR_PRIVATE`) dùng làm font chính toàn game, kết hợp **Be Vietnam Pro** để hiển thị tiếng Việt.
- **Animation thời gian thực:** Biến `g_GlobalAnimTime` tích lũy delta time mỗi frame, dùng làm tham số cho `sin()` để tạo hiệu ứng nhấp nháy, dao động, và pulse trên toàn bộ giao diện.
- **Glassmorphism:** Panel Pause, bảng cấu hình trận đấu và bảng load game dùng brush màu trắng hoặc đen bán trong suốt để tạo hiệu ứng kính.

### Pause Menu (Menu Tạm Dừng)
Nhấn `ESC` trong lúc chơi để mở giao diện "Hộp Tác Chiến" với 5 mục:
1. Tiếp tục thi đấu
2. Bật / tắt nhạc nền
3. Điều chỉnh âm lượng SFX
4. Ghi hình trận (lưu game vào slot)
5. Rời phòng thay đồ (quay về Menu chính)

Trong luồng lưu game, người chơi chọn slot (1-5), đặt tên cho bản lưu, rồi nhấn Enter.

### Cài đặt
Màn hình cài đặt có 7 mục điều chỉnh:
- Nhạc nền (BGM): Bật / Tắt
- Âm lượng nhạc (0-100, thanh trượt)
- Hiệu ứng âm thanh (SFX): Bật / Tắt
- Âm lượng SFX (0-100, thanh trượt)
- Ngôn ngữ: Tiếng Việt / English
- Chủ đề nền: Sân Cỏ Anh / Sân Neon / Retro Matrix
- Nút lưu và thoát

Cài đặt được lưu nhị phân vào `Asset/config.ini`. Nếu file không tồn tại, chương trình tự tạo và áp dụng giá trị mặc định.

---

## Yêu cầu hệ thống

- **Hệ điều hành:** Windows (Win32 API)
- **Trình biên dịch:** Visual Studio 2019 trở lên (MSVC)
- **Thư viện:** GDI+ (`gdiplus.lib`), Windows Multimedia (`winmm.lib`) — đều có sẵn trong Windows SDK, không cần cài thêm.
- **C++ Standard:** C++17 (dùng `std::filesystem` trong `SaveLoadSystem.cpp`)

---

## Hướng dẫn build

1. Mở file `src/src.sln` bằng Visual Studio.
2. Chọn cấu hình `Debug` hoặc `Release`.
3. Nhấn `Ctrl+Shift+B` để build toàn bộ dự án.
4. File thực thi (`.exe`) sẽ nằm trong thư mục `src/Debug/` hoặc `src/Release/`.
5. Sao chép thư mục `src/Asset/` vào cùng thư mục chứa file `.exe` trước khi chạy.

> Chú ý: Chương trình sử dụng đường dẫn tương đối (`Asset/font/...`, `Asset/models/...`, v.v.), nên thư mục `Asset` phải nằm cùng cấp với file `.exe`.

---

## Cấu trúc dự án

```
Gomoku_Game_Project/
├── .gitignore
├── README.md
└── src/
    ├── src.sln                     -- Giải pháp Visual Studio
    ├── src.vcxproj                 -- File dự án MSVC
    ├── main.cpp                    -- Điểm vào WinMain và vòng lặp chính
    │
    ├── ApplicationTypes/           -- Kiểu dữ liệu toàn cục
    │   ├── GameConfig.h            -- Struct GameConfig (âm thanh, ngôn ngữ, chủ đề)
    │   ├── GameConstants.h         -- Hằng số (kích thước bàn cờ, điều kiện thắng, slot lưu)
    │   ├── GameState.h             -- Enum ScreenState, PlayMode, MatchType
    │   └── PlayState.h             -- Struct PlayState (toàn bộ trạng thái ván đấu)
    │
    ├── GameLogic/                  -- Logic game cốt lõi
    │   ├── GameEngine.h / .cpp     -- Khởi tạo trận, xử lý nước đi, chuyển lượt, undo/redo
    │   ├── GameRules.h / .cpp      -- Kiểm tra nước đi hợp lệ, phát hiện thắng/thua/hòa
    │   ├── BotAI.h / .cpp          -- Thuật toán AI (Random, heuristic, Alpha-Beta)
    │   └── PlayerEngineer.h / .cpp -- (Dự phòng / chưa dùng)
    │
    ├── RenderAPI/                  -- Hệ thống kết xuất đồ họa
    │   ├── Colours.h               -- Bảng màu toàn cục (namespace Colour và GdipColour)
    │   ├── Renderer.h / .cpp       -- GDI+ init/shutdown, quản lý Sprite, Double Buffer, Font
    │   └── UIComponents.h / .cpp   -- Hàm vẽ UI: bàn cờ, pixel art, banner, sân bóng, avatar
    │
    ├── ScreenModules/              -- Màn hình giao diện
    │   ├── MenuScreen.h / .cpp     -- Menu chính (6 mục)
    │   ├── MatchConfigScreen.h / .cpp -- Cấu hình trận đấu (2 trang)
    │   ├── PlayScreen.h / .cpp     -- Màn hình thi đấu chính
    │   ├── LoadGameScreen.h / .cpp -- Tải game từ slot
    │   ├── SettingScreen.h / .cpp  -- Cài đặt âm thanh, ngôn ngữ, chủ đề
    │   ├── AboutScreen.h / .cpp    -- Về chúng tôi (stub)
    │   └── GuildScreen.h / .cpp    -- Hướng dẫn (stub)
    │
    └── SystemModules/              -- Hệ thống hỗ trợ
        ├── AudioSystem.h / .cpp    -- Phát nhạc nền (MCI) và hiệu ứng âm thanh (PlaySound)
        ├── ConfigLoader.h / .cpp   -- Đọc/ghi cấu hình nhị phân vào config.ini
        ├── Localization.h / .cpp   -- Nội địa hóa (stub, chưa triển khai đầy đủ)
        ├── SaveLoadSystem.h / .cpp -- Lưu/tải trạng thái ván đấu (binary serialization)
        └── TimeSystem.h / .cpp     -- Đếm ngược lượt, reset timer, hiển thị MM:SS
```

---

## Kiến trúc chương trình

### Vòng lặp chính (Game Loop)

Chương trình chạy theo mô hình **Event-Driven + Frame Loop**:

1. `PeekMessage` xử lý tất cả sự kiện Windows (`WM_KEYDOWN`, `WM_PAINT`, v.v.) không chặn.
2. Nếu không có sự kiện, chương trình `Sleep(16)` (~60 fps) rồi tính delta time.
3. Gọi `UpdatePlayLogic` để cập nhật đồng hồ, kích hoạt bot AI, và cập nhật hoạt ảnh thắng.
4. Gọi `InvalidateRect` để ép vẽ lại toàn bộ cửa sổ mỗi frame.

### Trạng thái toàn cục

Ba biến toàn cục được quản lý trong `main.cpp`:

| Biến | Kiểu | Mô tả |
|------|------|-------|
| `g_CurrentScreen` | `ScreenState` | Màn hình đang hiển thị |
| `g_Config` | `GameConfig` | Cài đặt âm thanh, ngôn ngữ, chủ đề |
| `g_PlayState` | `PlayState` | Toàn bộ trạng thái ván đấu hiện tại |

### Điều phối màn hình

`WndProc` đóng vai trò bộ điều phối trung tâm:
- `WM_KEYDOWN`: Gọi `Update*Screen()` tương ứng với `g_CurrentScreen`.
- `WM_PAINT`: Gọi `Render*Screen()` tương ứng, vẽ vào `DoubleBuffer` rồi `BitBlt` ra màn hình.
- `WM_ERASEBKGND`: Trả về 1 để ngăn Windows tự xóa nền (tránh nhấp nháy).

### Hệ thống màu (Colours.h)

Tất cả màu sắc được tập trung trong hai namespace:
- **`Colour::`** — Các hằng số `COLORREF` dùng cho Win32 GDI (văn bản, đường kẻ).
- **`GdipColour::`** — Các đối tượng `Gdiplus::Color` dùng cho GDI+ (brush, pen, bán trong suốt). Hàm `WithAlpha()` trong namespace này cho phép thay alpha channel của một màu có sẵn để dùng trong animation.

### Hệ thống Pixel Model

Asset đồ họa pixel art được mã hóa dưới dạng ma trận số nguyên trong file `.txt`:
- Dòng đầu: `width height`
- Các dòng tiếp theo: giá trị nguyên với `0` = trong suốt, các giá trị khác ánh xạ tới màu trong palette.
- Hàm `LoadPixelModel()` đọc file và trả về `struct PixelModel`.
- Hàm `DrawPixelModel()` vẽ model lên `Gdiplus::Graphics` với kích thước pixel và palette tùy chỉnh.
- Avatar cầu thủ dùng 5 palette màu khác nhau (P1 đỏ-trắng, P2 lam-vàng, Bot Easy xám, Bot Medium lục, Bot Hard đen-đỏ), ánh xạ qua hàm `GetPaletteColor()`.

---

## Luồng hoạt động

```
Khởi động
    └── Khởi tạo GDI+, load font (VT323, Be Vietnam Pro)
    └── Đọc GameConfig từ Asset/config.ini
    └── Tạo cửa sổ Win32 (850x750)
    └── Vào SCREEN_MENU

SCREEN_MENU  (W/S/Enter)
    ├── Bắt Đầu         --> SCREEN_MATCH_CONFIG
    ├── Tải Băng         --> SCREEN_LOAD_GAME
    ├── Cài Đặt Sân      --> SCREEN_SETTING
    ├── Hướng Dẫn        --> SCREEN_GUIDE (ESC để thoát)
    ├── Giới Thiệu       --> SCREEN_ABOUT (ESC để thoát)
    └── Thoát            --> Kết thúc chương trình

SCREEN_MATCH_CONFIG  (2 trang)
    ├── Trang 1: Chế độ (Caro/Tic-Tac-Toe), PvP/PvE, Độ khó, Thời gian lượt, Bo
    └── Trang 2: Avatar và tên P1, Avatar và tên P2 --> SCREEN_PLAY

SCREEN_PLAY
    ├── Đang chơi: Di chuyển cursor (W/A/S/D/mũi tên), đặt quân (Enter/Space)
    │             Q = Undo, E = Redo (chỉ PvE)
    │             ESC = pause
    ├── Pause: Tiếp tục, BGM, SFX, Lưu game, Thoát về Menu
    └── Kết thúc: Y = chơi lại, N/ESC = về Menu

SCREEN_LOAD_GAME  (W/S + Enter)
    └── Chọn slot 1-5 --> Load binary --> SCREEN_PLAY

SCREEN_SETTING  (W/S + A/D + Enter)
    └── ESC hoặc chọn "Quay lại" --> lưu config --> SCREEN_MENU
```

---

## Điều khiển

### Menu chính và các màn hình điều hướng

| Phím | Tác dụng |
|------|----------|
| W / S / Len / Xuong | Di chuyển giữa các mục |
| Enter / Space | Chọn / Xác nhận |
| ESC | Thoát / Quay lại |

### Màn hình thi đấu (PlayScreen)

| Phím | Tác dụng |
|------|----------|
| W / A / S / D | Di chuyển con trỏ trên bàn cờ |
| Mũi tên | Di chuyển con trỏ trên bàn cờ |
| Enter / Space | Đặt quân tại vị trí con trỏ |
| Q | Undo (chỉ PvE) |
| E | Redo (chỉ PvE) |
| ESC | Mở menu tạm dừng |

### Khi kết thúc ván

| Phím | Tác dụng |
|------|----------|
| Y | Chơi lại (reset bàn cờ, giữ điểm tổng) |
| N / ESC | Quay về Menu chính |

### Màn hình cài đặt

| Phím | Tác dụng |
|------|----------|
| W / S | Chọn mục |
| A / D | Thay đổi giá trị |
| Enter | Bật/tắt (với mục toggle) |
| ESC | Lưu cài đặt và quay về Menu |

---

## Tệp cấu hình và lưu game

### Asset/config.ini

Lưu nhị phân toàn bộ `struct GameConfig` (24 byte). Cấu trúc gồm:
- `isBgmEnabled` (bool)
- `bgmVolume` (int, 0-100)
- `isSfxEnabled` (bool)
- `sfxVolume` (int, 0-100)
- `currentLang` (enum: `APP_LANG_VI` = 0, `APP_LANG_EN` = 1)
- `currentTheme` (enum: `THEME_CLASSIC` = 0, `THEME_NEON` = 1, `THEME_RETRO` = 2)

### Asset/save/slot_N.bin

Định dạng binary theo thứ tự:

| Thứ tự | Nội dung |
|--------|----------|
| 1 | Magic number `0xCA05A1E2` (4 byte) |
| 2 | Version `2` (4 byte) |
| 3 | Các trường POD của `PlayState` (gameMode, matchType, isP1Turn, countdownTime, timeRemaining, boardSize, board[20][20], cursorRow, cursorCol, lastMoveRow, lastMoveCol, status, winner, difficulty, targetScore, matchDuration, p1TotalTimeLeft, p2TotalTimeLeft, isMatchTimed) |
| 4 | Thông tin người chơi P1 (tên Unicode, đường dẫn avatar, piece, totalWins, movesCount, maxTurnTime) |
| 5 | Thông tin người chơi P2 (tương tự P1) |
| 6 | Vector winningCells (độ dài + dữ liệu) |
| 7 | Vector matchHistory (độ dài + dữ liệu) |
| 8 | Vector redoStack (độ dài + dữ liệu) |

Chương trình bảo vệ chống đọc file hỏng bằng cách giới hạn độ dài chuỗi (<=4096) và kích thước vector (<= 100000).

---

## Hệ thống Asset

### Asset/font/

| Thư mục | Nội dung |
|---------|----------|
| `VT323/` | VT323-Regular.ttf — Font pixel art dùng làm VT323Default/Bold/Title/Note |
| `Be_Vietnam_Pro/` | Regular, Bold, Black, Italic — Dùng để hỗ trợ ký tự tiếng Việt |

Font được load bằng `AddFontResourceExW(..., FR_PRIVATE, 0)` và gỡ khi thoát.

### Asset/models/

Các file `.txt` chứa ma trận pixel art theo định dạng `width height` ở dòng đầu, tiếp theo là các giá trị nguyên:

| File | Kích thước | Nội dung |
|------|-----------|----------|
| `title_caro.txt` | 31x8 | Logo "CARO" pixel art 4 chữ cái |
| `trophy.txt` | (biến) | Cúp vô địch |
| `football.txt` | (biến) | Trái bóng đá pixel |
| `avatar_0.txt` | 8x8 | Avatar mặc định / P1 |
| `avatar_1.txt` | 8x8 | Avatar P2 |
| `avatar_2.txt` | 8x8 | Avatar Bot Easy |
| `avatar_3.txt` | 8x8 | Avatar Bot Medium |
| `avatar_4.txt` | 8x8 | Avatar Bot Hard |

Giá trị 0 = trong suốt. Từ 1 trở lên ánh xạ tới màu tùy theo loại asset (logo dùng palette màu cam-vàng, avatar dùng palette theo từng nhân vật).

### Asset/images/

| File | Nội dung |
|------|----------|
| `football.png` | Ảnh PNG trái bóng (dự phòng, chưa dùng chính thức) |

### Asset/lang/

| File | Nội dung |
|------|----------|
| `vi.txt` | Chuỗi ngôn ngữ tiếng Việt (chưa triển khai đầy đủ) |
| `en.txt` | Chuỗi ngôn ngữ tiếng Anh (chưa triển khai đầy đủ) |

### Asset/save/

Chứa các file `slot_1.bin` đến `slot_5.bin` và `save_auto.bin`. Thư mục được tạo tự động nếu chưa tồn tại khi lưu game.

---

## Ghi chú kỹ thuật

- **Bot AI chạy bất đồng bộ:** `std::async(std::launch::async, ...)` tránh đóng băng UI khi bot tính toán. Bàn cờ bị khóa (`g_AIsCalculating`) trong lúc chờ.
- **Sprite caching:** Ảnh quân X/O được scale sẵn vào `g_CachedX` / `g_CachedO` mỗi khi `cellSize` thay đổi, tránh scale lại mỗi frame.
- **Failsafe timer:** `ResetTimer` kiểm tra `maxTurnTime <= 0` và tự phục hồi về `countdownTime` hoặc 30 giây để tránh crash khi load game cũ.
- **Âm thanh:** `PlayBGM` dùng MCI (`mciSendString`) để hỗ trợ phát lặp. `PlaySFX` dùng `PlaySoundW` với cờ `SND_ASYNC` để không chặn luồng game. Nhạc nền hiện bị comment out, chỉ SFX còn hoạt động.
- **Localization:** Hệ thống `Localization` (`lang/vi.txt`, `lang/en.txt`) được khai báo nhưng chưa được tích hợp vào giao diện; toàn bộ chuỗi hiện vẫn hardcode tiếng Việt trong code.
- **AboutScreen / GuildScreen:** Được khai báo nhưng nội dung chỉ là stub rỗng (file `.cpp` chứa 3 byte).
