# Hướng Dẫn Đóng Góp

Chào mừng bạn đến với dự án. Dưới đây là các quy chuẩn bắt buộc mà mọi thành viên cần tuân thủ để đảm bảo tính đồng nhất và chất lượng mã nguồn.

## 1. Quy Trình Cốt Lõi

1. **Phân Nhánh:** Không làm việc trực tiếp trên nhánh chính. Mọi tính năng hoặc bản sửa lỗi đều phải được thực hiện trên nhánh riêng biệt.
2. **Kiểm Thử Cục Bộ:** Đảm bảo mã nguồn biên dịch thành công và mọi chức năng hoạt động ổn định trên môi trường máy tính cá nhân trước khi đẩy mã lên kho lưu trữ.
3. **Đề Xuất Thay Đổi:** Tất cả các thay đổi phải thông qua Yêu cầu Kéo (Pull Request) và được đánh giá chéo bởi ít nhất một kỹ sư cấp cao.

## 2. Tiêu Chuẩn Kỹ Thuật Bắt Buộc

Bạn cần tuân thủ nghiêm ngặt các quy định về mã nguồn do hệ thống quy định:

### 2.1. Quy Tắc Đặt Tên

- **Tên Biến:** Tuyệt đối không sử dụng tên viết tắt tối nghĩa hoặc biến vòng lặp đơn ký tự (như `i`, `j`, `k`, `n`). Tên biến phải mang tính tự giải thích. Áp dụng định dạng `camelCase`.
- **Tên Hàm Và Phương Thức:** Bắt buộc bắt đầu bằng một động từ chỉ hành động theo cú pháp `<Action><Target>` (ví dụ: `calculateTotalScore()`, `updateUserProfile()`).

### 2.2. Bình Luận Mã Nguồn

- Chỉ viết bình luận kỹ thuật để giải thích "TẠI SAO" (ngữ cảnh và logic phức tạp của thuật toán).
- Tuyệt đối không giải thích "LÀM GÌ" đối với những đoạn mã đã thể hiện rõ ràng luồng thực thi.
- Sử dụng văn phong học thuật, nghiêm túc và chính xác.

### 2.3. Cấu Trúc Mã Nguồn

- Các hàm và phương thức không nên quá dài. Ưu tiên phân tách các logic phức tạp thành các hàm hỗ trợ độc lập.
- Tuân thủ kiến trúc phân tầng của dự án (Tầng Giao Diện, Tầng Dữ Liệu Supabase, Tầng Thực Thi Trí Tuệ Nhân Tạo).

## 3. Quy Chuẩn Thông Điệp Cam Kết

Chúng tôi áp dụng định dạng tiêu chuẩn. Cấu trúc yêu cầu:
`<loại>(<phạm vi>): <mô tả ngắn>`

Các loại hợp lệ bao gồm:

- `feat`: Bổ sung tính năng mới.
- `fix`: Khắc phục lỗi hệ thống.
- `docs`: Cập nhật tài liệu kỹ thuật.
- `refactor`: Tái cấu trúc mã nguồn (không làm thay đổi chức năng cốt lõi).
- `perf`: Cải thiện hiệu suất.

## 4. Báo Cáo Lỗi Và Đề Xuất Tính Năng

Xin vui lòng sử dụng các mẫu đã được thiết lập sẵn tại thư mục `ISSUE_TEMPLATE` để tạo mới Báo cáo lỗi hoặc Đề xuất tính năng trên hệ thống quản lý.
