#ifndef _AUDIOSYSTEM_H_
#define _AUDIOSYSTEM_H_
#include <string>

/** @file AudioSystem.h
 *  @brief API hệ thống âm thanh: BGM (nhạc nền), SFX và worker luồng nền.
 */

/** @brief Worker luồng nền xử lý hàng đợi SFX (preload/play/stop). */
void sfxWorker();

/** @brief Đăng ký/tiền tải một file âm thanh với `alias` để dùng sau.
 *  @param filepath Đường dẫn tới file âm thanh.
 *  @param alias Khóa nhận diện âm thanh trong hệ thống.
 */
void preLoad(const std::string &filepath, const std::string &alias);

/** @brief Khởi tạo hệ thống âm thanh (khởi worker, preload âm cần thiết).
 *  Gọi một lần khi ứng dụng bắt đầu.
 */
void initAudioSystem();

/** @brief Phát nhạc nền từ `filepath`. */
void playBgm(const std::string &filepath);

/** @brief Dừng nhạc nền hiện hành. */
void stopBgm();

/** @brief Cập nhật âm lượng BGM. Nếu `force` = true, áp dụng ngay cả khi không thay đổi. */
void updateBgmVolume(bool force = false);

/** @brief Phát hiệu ứng SFX đã được preload bằng `alias`. */
void playSfx(const std::string &alias);

/** @brief Dừng hiệu ứng SFX đang phát theo `alias`. */
void stopSfx(const std::string &alias);

/** @brief Thoát và giải phóng toàn bộ tài nguyên âm thanh (gọi khi ứng dụng đóng).
 */
void shutdownAudioSystem();

/* Backwards-compatible wrappers removed: all callsites use canonical API now. */

#endif // _AUDIOSYSTEM_H_