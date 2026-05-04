# ĐỒ ÁN CARO - MÔN CƠ SỞ LẬP TRÌNH

**TRƯỜNG ĐẠI HỌC KHOA HỌC TỰ NHIÊN TP.HCM** **KHOA CÔNG NGHỆ THÔNG TIN** **TP.HCM, ngày 09 tháng 09 năm 2024**

---

## MỤC LỤC

| STT | Nội dung                           | Trang |
| :-- | :--------------------------------- | :---- |
| 1   | Giới thiệu                         | 3     |
| 2   | Kịch bản trò chơi                  | 3     |
| 3   | Các bước xây dựng trò chơi         | 4     |
| 4   | **YÊU CẦU ĐỒ ÁN**                  | 10    |
| 4.1 | Xử lý lưu/tải trò chơi (save/load) | 11    |
| 4.2 | Nhận biết thắng/thua/hòa           | 11    |
| 4.3 | Xử lý hiệu ứng thắng/thua/hòa      | 11    |
| 4.4 | Xử lý giao diện màn hình khi chơi  | 11    |
| 4.5 | Xử lý màn hình chính               | 11    |

---

## 1. Giới thiệu

Trong phần đồ án này ta sẽ phối hợp các kĩ thuật và cấu trúc dữ liệu cơ bản để xây dựng một trò chơi đơn giản, cờ caro. Để thực hiện được đồ án này ta cần các kiến thức cơ bản như: xử lý tập tin, handle, cấu trúc dữ liệu mảng một/hai chiều...

Phần hướng dẫn giúp sinh viên xây dựng trò chơi ở mức độ cơ bản, các em tự nghiên cứu để hoàn thiện một cách tốt nhất có thể.

---

## 2. Kịch bản trò chơi

Lúc đầu khi vào game sẽ xuất hiện bảng cờ caro, người chơi sẽ dùng các phím 'W', 'A', 'S', 'D' để điều chỉnh hướng di chuyển. Khi người chơi nhấn phím **Enter** thì sẽ xuất hiện dấu 'X' hoặc 'O' tùy vào lượt.

Khi một trong hai người chiến thắng theo luật caro thì màn hình xuất hiện dòng chữ chúc mừng người chiến thắng. Sau đó sẽ hỏi người dùng muốn tiếp tục chơi hay không:

- Nếu chọn phím **“y”**: Chương trình khởi động lại dữ liệu từ đầu.
- Nếu nhấn **phím bất kì**: Thoát chương trình.

Trường hợp khi vị trí bàn cờ đã kín chỗ thì màn hình xuất hiện dòng chữ "Hai ben hoa nhau" và hỏi người dùng có muốn thoát hay chơi tiếp tương tự như trên.

### Sơ đồ kịch bản trò chơi (Hình 1)

- **Bắt đầu** -> **Chơi** -> **Sự kiện**
- Nếu **Sự kiện** = Không -> Quay lại **Chơi**.
- Nếu **Sự kiện** = Có -> **Tiếp tục?**
- Nếu **Tiếp tục** = Có -> Quay lại **Chơi**.
- Nếu **Tiếp tục** = Không -> **Kết thúc**.

---

## 3. Các bước xây dựng trò chơi

### Bước 1: Cố định màn hình Console

Việc này giúp tránh trường hợp người dùng tự co giãn màn hình gây khó khăn trong quá trình xử lý giao diện.

```cpp
// Hàm nhóm View
void FixConsoleWindow() {
    HWND consoleWindow = GetConsoleWindow();
    LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
    style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
    SetWindowLong(consoleWindow, GWL_STYLE, style);
}
```

- `HWND`: Con trỏ trỏ tới chính cửa sổ Console.
- `GWL_STYLE`: Lấy các đặc tính hiện tại của cửa sổ.
- Dòng 4: Làm mờ nút maximize và chặn thay đổi kích thước.

### Bước 2: Hàm di chuyển con trỏ (GotoXY)

Cần thiết để in dữ liệu tại bất kỳ vị trí nào trên màn hình.

```cpp
// Hàm nhóm View
void GotoXY(int x, int y) {
    COORD coord;
    coord.X = $x$;
    coord.Y = $y$;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
```

- `COORD`: Cấu trúc xử lý tọa độ màn hình.
- `GetStdHandle(STD_OUTPUT_HANDLE)`: Lấy con trỏ (HANDLE) tới màn hình console.

### Bước 3: Khai báo dữ liệu toàn cục

Sử dụng các biến và hằng số cơ bản để quản lý trạng thái trò chơi.

```cpp
// Hằng số
#define BOARD_SIZE 12 // Kích thước ma trận bàn cờ
#define LEFT 3        // Tọa độ trái màn hình
#define TOP 1         // Tọa độ trên màn hình

// Khai báo kiểu dữ liệu
struct _POINT { int x, y, c; }; // x, y: tọa độ màn hình, c: trạng thái đánh dấu

_POINT _A[BOARD_SIZE][BOARD_SIZE]; // Ma trận bàn cờ
bool _TURN;      // true: lượt người thứ nhất, false: lượt người thứ hai
int _COMMAND;    // Biến nhận giá trị phím nhập
int _X, _Y;      // Tọa độ hiện hành trên màn hình bàn cờ
```

### Bước 4: Khởi tạo dữ liệu (ResetData)

Thiết lập lại trạng thái ban đầu cho mỗi ván chơi mới.

```cpp
// Hàm nhóm Model
void ResetData() {
    for (int $i = 0$; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _A[i][j].x = 4 * j + LEFT + 2; // Tính toán hoành độ
            _A[i][j].y = 2 * i + TOP + 1;  // Tính toán tung độ
            _A[i][j].c = 0;                // 0: chưa đánh, -1: lượt true, 1: lượt false
        }
    }
    _TURN = true;
    _COMMAND = -1;
    _X = _A[0][0].x;
    _Y = _A[0][0].y;
}
```

### Bước 5: Vẽ bàn cờ (DrawBoard)

Vẽ một hình vuông bao quanh phạm vi chơi dựa trên `BOARD_SIZE`.

```cpp
// Hàm nhóm View
void DrawBoard(int pSize) {
    for (int $i = 0$; i < pSize; i++) {
        for (int $j = 0$; j <= pSize; j++) {
            GotoXY(LEFT + 4 * i, TOP + 2 * j);
            printf(".");
        }
    }
}
```

### Bước 6: Hàm bắt đầu trò chơi (StartGame)

```cpp
// Hàm nhóm Control
void StartGame() {
    system("cls");
    ResetData();
    DrawBoard(BOARD_SIZE);
}
```

_Sơ đồ gọi hàm:_ `StartGame()` -> gọi `ResetData()` và `DrawBoard()`.

### Bước 7: Thoát và dọn dẹp (ExitGame, GabageCollect)

```cpp
// Hàm nhóm Model
void GabageCollect() {
    // Dọn dẹp tài nguyên nếu có dùng con trỏ
}

// Hàm nhóm Control
void ExitGame() {
    system("cls");
    GabageCollect();
}
```

### Bước 8: Xử lý kết thúc và hỏi tiếp tục

```cpp
// Hàm nhóm View
int ProcessFinish(int pWhoWin) {
    GotoXY(0, _A[BOARD_SIZE - 1][BOARD_SIZE - 1].y + 2);
    switch (pWhoWin) {
        case -1: printf("Nguoi choi %d da thang va nguoi choi %d da thua\n", true, false); break;
        case 1:  printf("Nguoi choi %d da thang va nguoi choi %d da thua\n", false, true); break;
        case 0:  printf("Nguoi choi %d da hoa nguoi choi %d\n", false, true); break;
        case 2:  _TURN = !_TURN; // Đổi lượt nếu chưa kết thúc
    }
    GotoXY(_X, _Y);
    return pWhoWin;
}

int AskContinue() {
    GotoXY(0, _A[BOARD_SIZE - 1][BOARD_SIZE - 1].y + 4);
    printf("Nhan 'y/n' de tiep tuc/dung: ");
    return toupper(getch());
}
```

### Bước 9: Kiểm tra trạng thái bàn cờ (TestBoard)

```cpp
// Hàm nhóm Model
int TestBoard() {
    if (<Ma trận đầy>) return 0; // Hòa
    else {
        if (<Điều kiện thắng luật caro>) return (_TURN == true ? -1 : 1);
        return 2; // Chưa ai thắng
    }
}
```

### Bước 10: Đánh dấu vào ma trận (CheckBoard)

```cpp
// Hàm nhóm Model
int CheckBoard(int pX, int pY) {
    for (int $i = 0$; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].x == pX && _A[i][j].y == pY && _A[i][j].c == 0) {
                if (_TURN == true) _A[i][j].c = -1;
                else _A[i][j].c = 1;
                return _A[i][j].c;
            }
        }
    }
    return 0;
}
```

### Bước 11: Các hàm di chuyển

Sử dụng các phím điều hướng để cập nhật tọa độ trên màn hình bàn cờ.

- `MoveRight()`: `$X += 4$` (nếu chưa vượt biên phải).
- `MoveLeft()`: `$X -= 4$` (nếu chưa vượt biên trái).
- `MoveDown()`: `$Y += 2$` (nếu chưa vượt biên dưới).
- `MoveUp()`: `$Y -= 2$` (nếu chưa vượt biên trên).

### Bước 12: Hàm main

Điều khiển luồng chính của trò chơi.

```cpp
void main() {
    FixConsoleWindow();
    StartGame();
    bool validEnter = true;
    while (1) {
        _COMMAND = toupper(getch());
        if (_COMMAND == 27) { // Phím Esc
            ExitGame(); return;
        } else {
            if (_COMMAND == 'A') MoveLeft();
            else if (_COMMAND == 'W') MoveUp();
            else if (_COMMAND == 'S') MoveDown();
            else if (_COMMAND == 'D') MoveRight();
            else if (_COMMAND == 13) { // Phím Enter
                switch (CheckBoard(_X, _Y)) {
                    case -1: printf("X"); break;
                    case 1:  printf("O"); break;
                    case 0:  validEnter = false; break;
                }
                if (validEnter == true) {
                    switch (ProcessFinish(TestBoard())) {
                        case -1: case 1: case 0:
                            if (AskContinue() != 'Y') {
                                ExitGame(); return;
                            } else StartGame();
                    }
                }
                validEnter = true;
            }
        }
    }
}
```

---

## 4. YÊU CẦU ĐỒ ÁN

Để hoàn thiện trò chơi, sinh viên cần thực hiện thêm các chức năng sau:

### 4.1 Xử lý lưu/tải trò chơi (save/load)

- Phím **'L'**: Nhập tên tập tin để lưu trạng thái hiện hành.
- Phím **'T'**: Nhập tên tập tin để tải lại màn chơi cũ.

### 4.2 Nhận biết thắng/thua/hòa

Cài đặt thuật toán kiểm tra quy luật caro (ví dụ: 5 ô liên tiếp hàng ngang, dọc, chéo) sau mỗi bước đi.

### 4.3 Xử lý hiệu ứng thắng/thua/hòa

Thay vì chỉ in dòng chữ đơn giản, hãy cài đặt các hiệu ứng màu sắc hoặc âm thanh sinh động.

### 4.4 Xử lý giao diện màn hình khi chơi

Hiển thị các thông số như: số bước đã đi của người 1/người 2, số ván thắng/thua. Tổ chức giao diện rõ ràng.

### 4.5 Xử lý màn hình chính

Xây dựng Menu trước khi vào game:

- New Game
- Load Game
- Settings
- Exit

---

**Trang 11 - Hết**
