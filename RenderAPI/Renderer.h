#pragma once
#include <raylib.h> // Gọi thư viện đồ họa Raylib
#include <cstdint>

// --- QUẢN LÝ CỬA SỔ ĐỒ HỌA ---

// Khởi tạo cửa sổ Game 
void InitGameWindow(uint16_t width, uint16_t height, const char* title);

// Đóng cửa sổ và giải phóng bộ nhớ đồ họa
void CloseGameWindow();

// Kiểm tra xem người dùng có bấm nút [X] góc trên cùng để tắt game không
bool IsWindowRunning();

// --- CÁC HÀM XỬ LÝ QUÁ TRÌNH VẼ ---
// BẮT BUỘC: Mọi thao tác vẽ (Draw) phải nằm giữa BeginRender và EndRender

void BeginRender();
void EndRender();

// Xóa nền cũ bằng một màu trơn (Thay thế triệt để cho ClearScreen / system("cls"))
void ClearScreenBackground(Color color);